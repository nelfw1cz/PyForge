#include "RpgModule.h"
#include "editor/CodeEditor.h"
#include "theme/NeonTheme.h"

#include <QPainter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>

// ═══════════════════ NeonBar ═══════════════════

NeonBar::NeonBar(const QString& label, QColor color, QWidget* parent)
    : QWidget(parent), m_label(label), m_color(color) {
    // QVariantAnimation плавно ведёт m_shown к целевому значению
    connect(&m_anim, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& v){ m_shown = v.toReal(); update(); });
}

void NeonBar::setValue(qreal v) {
    m_anim.stop();
    m_anim.setDuration(350);
    m_anim.setEasingCurve(QEasingCurve::OutCubic);
    m_anim.setStartValue(m_shown);
    m_anim.setEndValue(qBound(0.0, v, 1.0));
    m_anim.start();
}

void NeonBar::setCaption(const QString& text) { m_caption = text; update(); }
void NeonBar::setColor(QColor c)              { m_color = c;     update(); }

void NeonBar::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF r = rect().adjusted(1, 1, -1, -1);
    const qreal h = r.height();

    // Подложка-«стекло»
    p.setPen(QPen(Neon::line(), 1));
    p.setBrush(QColor(15, 18, 24));
    p.drawRoundedRect(r, h / 2, h / 2);

    // Заполнение + свечение
    if (m_shown > 0.004) {
        const qreal w = qMax(h, r.width() * qBound(0.0, m_shown, 1.0));
        const QRectF fill(r.left(), r.top(), w, h);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(m_color.red(), m_color.green(), m_color.blue(), 60));
        p.drawRoundedRect(fill.adjusted(-2, -2, 2, 2), h / 2 + 2, h / 2 + 2);   // glow

        QLinearGradient g(0, 0, 0, h);
        g.setColorAt(0, m_color.lighter(125));
        g.setColorAt(1, m_color.darker(120));
        p.setBrush(g);
        p.drawRoundedRect(fill, h / 2, h / 2);
    }

    // Подписи: слева — имя шкалы, справа — числовое значение
    QFont f = font();
    f.setBold(true);
    f.setPointSize(qMax(7, f.pointSize() - 2));
    p.setFont(f);
    p.setPen(QColor(232, 238, 248));
    p.drawText(r.adjusted(12, 0, -12, 0), Qt::AlignVCenter | Qt::AlignLeft, m_label);
    p.setPen(Neon::dim());
    p.drawText(r.adjusted(12, 0, -12, 0), Qt::AlignVCenter | Qt::AlignRight, m_caption);
}

// ═══════════════════ RpgHud ═══════════════════

RpgHud::RpgHud(QWidget* parent) : QWidget(parent) {
    setObjectName("HudPanel");
    setFixedHeight(58);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(14, 8, 14, 8);
    lay->setSpacing(14);

    m_level = new QLabel("LV 1", this);
    m_level->setObjectName("LevelBadge");
    m_level->setAlignment(Qt::AlignCenter);
    m_level->setFixedSize(58, 40);

    m_xp = new NeonBar(QStringLiteral("XP"), Neon::violet(), this);
    m_hp = new NeonBar(QStringLiteral("HP"), Neon::green(),  this);

    m_combo = new QLabel(QStringLiteral("×0"), this);
    m_combo->setObjectName("ComboLabel");
    m_combo->setAlignment(Qt::AlignCenter);
    m_combo->setMinimumWidth(90);

    lay->addWidget(m_level);
    lay->addWidget(m_xp, 3);
    lay->addWidget(m_hp, 3);
    lay->addWidget(m_combo);
}

void RpgHud::setState(int level, qreal xp01, const QString& xpText,
                      qreal hp01, const QString& hpText, int combo) {
    m_level->setText(QStringLiteral("LV %1").arg(level));
    m_xp->setValue(xp01);
    m_xp->setCaption(xpText);
    m_hp->setValue(hp01);
    m_hp->setCaption(hpText);
    // Цвет HP зависит от остатка здоровья
    m_hp->setColor(hp01 > 0.5 ? Neon::green() : (hp01 > 0.25 ? Neon::amber() : Neon::red()));
    m_combo->setText(QStringLiteral("×%1").arg(combo));
    m_combo->setProperty("hot", combo >= 5);   // «горячее» комбо подсвечивается
    m_combo->style()->polish(m_combo);
}

// ═══════════════════ Всплывающее уведомление ═══════════════════

namespace {
// Одноразовый тост: появляется с фейдом, висит, исчезает и удаляет себя.
class Toast : public QWidget {
public:
    Toast(QWidget* host, const QString& title, const QString& sub, QColor accent)
        : QWidget(host) {
        setObjectName("Toast");
        setAttribute(Qt::WA_DeleteOnClose);
        setStyleSheet(QStringLiteral(
            "#Toast { background: rgba(16,19,26,240); border: 1px solid %1; border-radius: 12px; }")
            .arg(accent.name()));

        auto* lay = new QVBoxLayout(this);
        lay->setContentsMargins(36, 20, 36, 20);
        lay->setSpacing(4);

        auto* t = new QLabel(title, this);
        t->setAlignment(Qt::AlignCenter);
        t->setStyleSheet(QStringLiteral(
            "color:%1; font-size:22px; font-weight:800; letter-spacing:3px;").arg(accent.name()));

        auto* s = new QLabel(sub, this);
        s->setAlignment(Qt::AlignCenter);
        s->setStyleSheet(QStringLiteral("color:#aab3c5; font-size:12px;"));

        lay->addWidget(t);
        lay->addWidget(s);
        adjustSize();

        auto* fx = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(fx);

        move((host->width() - width()) / 2, int(host->height() * 0.30));
        raise();
        show();

        // Фейд-ин → пауза → фейд-аут → close()
        auto* in = new QPropertyAnimation(fx, "opacity", this);
        in->setDuration(220);
        in->setStartValue(0.0);
        in->setEndValue(1.0);
        connect(in, &QPropertyAnimation::finished, this, [this, fx]{
            QTimer::singleShot(1400, this, [this, fx]{
                auto* out = new QPropertyAnimation(fx, "opacity", this);
                out->setDuration(320);
                out->setStartValue(1.0);
                out->setEndValue(0.0);
                connect(out, &QPropertyAnimation::finished, this, [this]{ close(); });
                out->start();
            });
        });
        in->start();
    }
};
} // namespace

// ═══════════════════ RpgModule ═══════════════════

RpgModule::RpgModule(CodeEditor* editor, QWidget* toastHost, QObject* parent)
    : QObject(parent), m_editor(editor), m_host(toastHost) {
    m_hud = new RpgHud;   // родителя назначит EditorWindow при вставке в layout
    // Сигнал «строка сдана» слушаем всегда, но начисляем только в активном режиме
    connect(editor, &CodeEditor::lineCommitted, this, &RpgModule::onLineCommitted);
}

void RpgModule::activate() {
    m_active = true;
    m_hud->show();
    updateHud();
}

void RpgModule::deactivate() {
    m_active = false;
    m_hud->hide();
}

int RpgModule::xpNeeded() const {
    return 100 + (m_level - 1) * 50;   // каждый уровень дороже предыдущего
}

void RpgModule::onLineCommitted() {
    if (!m_active) return;             // изоляция: в других режимах XP не капает
    ++m_combo;
    gainXp(4);
}

void RpgModule::onRunFinished(bool success) {
    if (!m_active) return;
    if (success) {
        ++m_combo;
        m_hp = qMin(kMaxHp, m_hp + 5); // небольшая награда-лечение за чистый запуск
        gainXp(30);
    } else {
        m_combo = 0;                   // ошибка скрипта рушит серию
        damage(18);
    }
    updateHud();
}

void RpgModule::gainXp(int amount) {
    m_xp += amount;
    // Возможно несколько левелапов подряд (например, после большого бонуса)
    while (m_xp >= xpNeeded()) {
        m_xp -= xpNeeded();
        ++m_level;
        showToast(QStringLiteral("LEVEL UP!"),
                  QStringLiteral("Достигнут уровень %1. Так держать, кодер!").arg(m_level),
                  Neon::violet());
    }
    updateHud();
}

void RpgModule::damage(int amount) {
    m_hp -= amount;
    if (m_hp <= 0) {
        m_hp = kMaxHp;                 // возрождение
        m_xp = qMax(0, m_xp - 30);     // штраф опытом
        showToast(QStringLiteral("ВЫ ПОГИБЛИ"),
                  QStringLiteral("Код оказался сильнее. Возрождение, комбо сброшено."),
                  Neon::red());
    }
    updateHud();
}

void RpgModule::updateHud() {
    m_bestCombo = qMax(m_bestCombo, m_combo);
    m_hud->setState(m_level,
                    qreal(m_xp) / xpNeeded(),
                    QStringLiteral("%1 / %2 XP").arg(m_xp).arg(xpNeeded()),
                    qreal(m_hp) / kMaxHp,
                    QStringLiteral("%1 / %2 HP").arg(m_hp).arg(kMaxHp),
                    m_combo);
}

void RpgModule::showToast(const QString& title, const QString& sub, QColor accent) {
    (void)new Toast(m_host, title, sub, accent);   // тост уничтожает себя сам
}