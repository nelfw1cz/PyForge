#include "TitleBar.h"
#include "theme/NeonTheme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <QPainter>

TitleBar::TitleBar(QWidget* parent) : QWidget(parent) {
    setObjectName("TitleBar");
    setFixedHeight(46);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(14, 0, 10, 0);
    lay->setSpacing(10);

    // Логотип: светящаяся точка, нарисованная в QPixmap
    auto* logo = new QLabel(this);
    {
        QPixmap px(16, 16);
        px.fill(Qt::transparent);
        QPainter g(&px);
        g.setRenderHint(QPainter::Antialiasing);
        QRadialGradient grad(8, 8, 8);
        grad.setColorAt(0, Neon::cyan().lighter(150));
        grad.setColorAt(1, Neon::cyan().darker(140));
        g.setBrush(grad);
        g.setPen(QPen(Neon::cyan(), 1));
        g.drawEllipse(2, 2, 12, 12);
        logo->setPixmap(px);
    }
    lay->addWidget(logo);

    auto* title = new QLabel("PYFORGE", this);
    title->setObjectName("AppTitle");
    lay->addWidget(title);

    auto* sub = new QLabel("PYTHON EDITOR · ПРОТОТИП", this);
    sub->setObjectName("AppSub");
    sub->setContentsMargins(0, 4, 0, 0);
    lay->addWidget(sub);

    lay->addStretch(1);

    m_modeLabel = new QLabel(this);
    m_modeLabel->setObjectName("ModeLabel");
    lay->addWidget(m_modeLabel);

    lay->addStretch(1);

    // Кнопка запуска скрипта
    m_run = new QPushButton(QStringLiteral("▶  Запустить"), this);
    m_run->setObjectName("RunBtn");
    m_run->setCursor(Qt::PointingHandCursor);
    m_run->setToolTip(QStringLiteral("F5 или Ctrl+Enter — выполнить скрипт"));
    connect(m_run, &QPushButton::clicked, this, &TitleBar::runClicked);
    lay->addWidget(m_run);

    lay->addSpacing(14);

    // Кнопки управления окном
    auto makeBtn = [this](const QString& text, const char* obj, const QString& tip) {
        auto* b = new QPushButton(text, this);
        b->setObjectName(obj);
        b->setFixedSize(34, 28);
        b->setCursor(Qt::PointingHandCursor);
        b->setToolTip(tip);
        return b;
    };
    auto* bMin = makeBtn(QStringLiteral("─"), "WinBtn",   QStringLiteral("Свернуть"));
    auto* bMax = makeBtn(QStringLiteral("▢"), "WinBtn",   QStringLiteral("Развернуть"));
    auto* bCls = makeBtn(QStringLiteral("✕"), "CloseBtn", QStringLiteral("Закрыть"));

    connect(bMin, &QPushButton::clicked, this, [this]{ window()->showMinimized(); });
    connect(bMax, &QPushButton::clicked, this, [this]{
        window()->isMaximized() ? window()->showNormal() : window()->showMaximized();
    });
    connect(bCls, &QPushButton::clicked, this, [this]{ window()->close(); });

    lay->addWidget(bMin);
    lay->addWidget(bMax);
    lay->addWidget(bCls);
}

void TitleBar::setModeName(const QString& name) {
    m_modeLabel->setText(QStringLiteral("РЕЖИМ · %1").arg(name.toUpper()));
}

void TitleBar::setBusy(bool busy) {
    m_run->setText(busy ? QStringLiteral("■  Стоп") : QStringLiteral("▶  Запустить"));
}

// ── Перетаскивание безрамочного окна ─────────────────────────
void TitleBar::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton)
        m_dragPos = e->globalPosition().toPoint() - window()->frameGeometry().topLeft();
    e->accept();
}

void TitleBar::mouseMoveEvent(QMouseEvent* e) {
    if (!m_dragPos.isNull() && (e->buttons() & Qt::LeftButton)) {
        if (window()->isMaximized()) window()->showNormal();
        window()->move(e->globalPosition().toPoint() - m_dragPos);
    }
    e->accept();
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* e) {
    window()->isMaximized() ? window()->showNormal() : window()->showMaximized();
    e->accept();
}