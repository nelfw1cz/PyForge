#pragma once
#include <QWidget>
#include <QPoint>

class QLabel;
class QPushButton;

// ─────────────────────────────────────────────────────────────
// Кастомный заголовок безрамочного окна: логотип, название,
// индикатор активного режима, кнопка запуска и кнопки окна.
// Также отвечает за перетаскивание окна мышью.
// ─────────────────────────────────────────────────────────────
class TitleBar : public QWidget {
    Q_OBJECT
public:
    explicit TitleBar(QWidget* parent = nullptr);
    void setModeName(const QString& name);
    void setBusy(bool busy);          // скрипт выполняется → кнопка превращается в «Стоп»

signals:
    void runClicked();

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;

private:
    QLabel* m_modeLabel = nullptr;
    QPushButton* m_run = nullptr;
    QPoint m_dragPos;                 // точка захвата окна
};