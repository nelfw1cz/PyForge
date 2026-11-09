#include "TimeTravelModule.h"
#include "editor/CodeEditor.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QStyle>

TimeTravelModule::TimeTravelModule(CodeEditor* editor, QObject* parent)
    : QObject(parent), m_editor(editor) {
    // Панель-«видеоплеер» над редактором
    m_bar = new QWidget;
    m_bar->setObjectName("TimeBar");
    m_bar->setFixedHeight(52);

    auto* lay = new QHBoxLayout(m_bar);
    lay->setContentsMargins(16, 6, 16, 6);
    lay->setSpacing(12);

    auto* caption = new QLabel(QStringLiteral("ИСТОРИЯ ИЗМЕНЕНИЙ"), m_bar);
    caption->setObjectName("TimeCaption");

    m_slider = new QSlider(Qt::Horizontal, m_bar);
    m_slider->setObjectName("TimeSlider");
    m_slider->setRange(0, 0);
    m_slider->setToolTip(QStringLiteral("Тяните влево — в прошлое, вправо — в настоящее"));

    m_info = new QLabel(QStringLiteral("● LIVE"), m_bar);
    m_info->setObjectName("TimeInfo");
    m_info->setMinimumWidth(170);
    m_info->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto* back = new QPushButton(QStringLiteral("в настоящее →"), m_bar);
    back->setObjectName("SnippetChip");
    back->setCursor(Qt::PointingHandCursor);
    connect(back, &QPushButton::clicked, this, [this]{
        returnToPresent();
        m_slider->setValue(m_slider->maximum());
    });

    lay->addWidget(caption);
    lay->addWidget(m_slider, 1);
    lay->addWidget(m_info);
    lay->addWidget(back);

    // Дебаунс: снимок делается, когда пользователь «успокоился» на 600 мс
    m_debounce = new QTimer(this);
    m_debounce->setSingleShot(true);
    m_debounce->setInterval(600);
    connect(m_debounce, &QTimer::timeout, this, &TimeTravelModule::capture);

    connect(m_editor, &QPlainTextEdit::textChanged, this, &TimeTravelModule::onTextChanged);
    connect(m_editor, &CodeEditor::lineCommitted, this, [this]{
        if (!m_inPast) capture();          // завершённая строка — сразу в историю
    });
    connect(m_slider, &QSlider::sliderMoved, this, &TimeTravelModule::onSliderMoved);
}

void TimeTravelModule::activate() {
    m_bar->show();
    capture();                             // базовый снимок на момент входа в режим
    m_slider->setRange(0, qMax(0, m_snapshots.size() - 1));
    m_slider->setValue(m_slider->maximum());
    updateLabel();
}

void TimeTravelModule::deactivate() {
    m_debounce->stop();
    if (m_inPast) returnToPresent();       // нельзя покинуть режим, застряв в прошлом
    m_bar->hide();
}

void TimeTravelModule::onTextChanged() {
    // В прошлом текст меняется программно (перемотка) — такие изменения не снимаем
    if (m_inPast || !m_bar->isVisible()) return;
    m_debounce->start();
}

void TimeTravelModule::capture() {
    if (m_inPast) return;
    const QString txt = m_editor->toPlainText();
    if (!m_snapshots.isEmpty() && m_snapshots.last().text == txt) {
        updateLabel();
        return;                            // ничего не изменилось
    }
    m_snapshots.append({txt, QTime::currentTime()});
    while (m_snapshots.size() > kMaxSnapshots)
        m_snapshots.removeFirst();         // кольцевой буфер: выталкиваем старьё

    m_slider->setRange(0, m_snapshots.size() - 1);
    m_slider->setValue(m_slider->maximum());// «сейчас» всегда справа
    updateLabel();
}

void TimeTravelModule::onSliderMoved(int value) {
    const int last = m_snapshots.size() - 1;
    if (value >= last) {                   // доскрестили до настоящего
        returnToPresent();
        return;
    }
    // Первый уход из «сейчас»: фиксируем живой текст и блокируем ввод
    if (!m_inPast) {
        m_inPast = true;
        m_liveText = m_editor->toPlainText();
        m_editor->setReadOnly(true);
        m_editor->setProperty("inPast", true);   // включает «призрачный» стиль в QSS
        m_editor->style()->polish(m_editor);
    }
    // Внимание: setPlainText сбрасывает undo-историю — осознанное упрощение
    m_editor->setPlainText(m_snapshots[value].text);
    updateLabel();
}

void TimeTravelModule::returnToPresent() {
    if (!m_inPast) { updateLabel(); return; }
    m_inPast = false;
    m_editor->setReadOnly(false);
    m_editor->setProperty("inPast", false);
    m_editor->style()->polish(m_editor);
    m_editor->setPlainText(m_liveText);
    updateLabel();
}

void TimeTravelModule::updateLabel() {
    if (!m_inPast) {
        m_info->setText(QStringLiteral("● LIVE · кадров: %1").arg(m_snapshots.size()));
        m_info->setProperty("past", false);
    } else {
        const int idx = m_slider->value();
        m_info->setText(QStringLiteral("◂ кадр %1/%2 · %3")
                            .arg(idx + 1)
                            .arg(m_snapshots.size())
                            .arg(m_snapshots.value(idx).time.toString("HH:mm:ss")));
        m_info->setProperty("past", true);
    }
    m_info->style()->polish(m_info);
}