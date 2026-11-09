#pragma once
#include <QWidget>
#include <QObject>
#include <QColor>
#include <QString>
#include <QVariantAnimation>

class QLabel;
class CodeEditor;

// ─────────────────────────────────────────────────────────────
// NeonBar — анимированная неоновая шкала (XP/HP), рисуется
// целиком через QPainter: подложка, градиентное заполнение,
// мягкое свечение и подписи.
// ─────────────────────────────────────────────────────────────
class NeonBar : public QWidget {
    Q_OBJECT
public:
    NeonBar(const QString& label, QColor color, QWidget* parent = nullptr);
    void setValue(qreal v);                 // целевое значение 0..1 (с анимацией)
    void setCaption(const QString& text);   // текст справа (например "45 / 100")
    void setColor(QColor c);
    QSize sizeHint() const override { return {220, 30}; }

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QString m_label;
    QString m_caption;
    QColor  m_color;
    qreal   m_shown = 0;                    // отображаемое (анимируемое) значение
    QVariantAnimation m_anim;
};

// HUD-панель: уровень, шкалы XP/HP, счётчик комбо.
class RpgHud : public QWidget {
    Q_OBJECT
public:
    explicit RpgHud(QWidget* parent = nullptr);
    void setState(int level, qreal xp01, const QString& xpText,
                  qreal hp01, const QString& hpText, int combo);

private:
    QLabel*  m_level;
    NeonBar* m_xp;
    NeonBar* m_hp;
    QLabel*  m_combo;
};

// ─────────────────────────────────────────────────────────────
// Логика режима Code-RPG (изоляция: начисления идут, только
// пока режим активен). Источники событий:
//  • CodeEditor::lineCommitted()      → +XP, +комбо;
//  • EditorWindow::runScript() успех  → +XP, +HP, +комбо;
//  • запуск с ошибками (непустой stderr) → −HP, сброс комбо.
// ─────────────────────────────────────────────────────────────
class RpgModule : public QObject {
    Q_OBJECT
public:
    RpgModule(CodeEditor* editor, QWidget* toastHost, QObject* parent = nullptr);

    RpgHud* hud() const { return m_hud; }
    void activate();
    void deactivate();

    void onLineCommitted();
    void onRunFinished(bool success);

private:
    void gainXp(int amount);
    void damage(int amount);
    int  xpNeeded() const;
    void updateHud();
    void showToast(const QString& title, const QString& sub, QColor accent);

    CodeEditor* m_editor;
    QWidget*    m_host;       // виджет, поверх которого летают тосты
    RpgHud*     m_hud;

    int  m_level = 1;
    int  m_xp    = 0;
    int  m_hp    = kMaxHp;
    int  m_combo = 0;
    int  m_bestCombo = 0;
    bool m_active = false;

    static constexpr int kMaxHp = 100;
};