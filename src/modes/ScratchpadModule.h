#pragma once
#include <QWidget>
#include <QObject>
#include <QVector>
#include <QPointF>

class CodeEditor;
class QPushButton;

// Фигура слоя заметок: росчерк маркера или стрелка
struct PadShape {
    enum Kind { Stroke, Arrow };
    Kind kind = Stroke;
    QVector<QPointF> points;   // Stroke: полилиния; Arrow: [начало, конец]
};

// ─────────────────────────────────────────────────────────────
// Прозрачный оверлей поверх редактора. Перехватывает мышь
// (редактор в этом режиме не редактируется — так и задумано)
// и рисует заметки через QPainter: маркер и стрелки.
// ─────────────────────────────────────────────────────────────
class ScratchpadOverlay : public QWidget {
    Q_OBJECT
public:
    explicit ScratchpadOverlay(CodeEditor* editor);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    void layoutTools();

    QVector<PadShape> m_shapes;
    QVector<QPushButton*> m_tools;
    PadShape::Kind m_tool = PadShape::Stroke;
    bool m_drawing = false;
};

// Сам режим: показать/скрыть оверлей (рисунки сохраняются между входами)
class ScratchpadModule : public QObject {
    Q_OBJECT
public:
    explicit ScratchpadModule(CodeEditor* editor, QObject* parent = nullptr);
    void activate();
    void deactivate();

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    CodeEditor* m_editor;
    ScratchpadOverlay* m_overlay;
};