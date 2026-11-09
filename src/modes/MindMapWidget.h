#pragma once
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QVector>

class QGraphicsScene;
class FuncNode;

// ─────────────────────────────────────────────────────────────
// Плавный коннектор со стрелкой между двумя функциями.
// Пересчитывается каждый раз, когда ноды двигают.
// ─────────────────────────────────────────────────────────────
class EdgeItem : public QGraphicsPathItem {
public:
    EdgeItem(FuncNode* from, FuncNode* to);
    void rebuild();

private:
    FuncNode* m_from;
    FuncNode* m_to;
    QGraphicsPolygonItem* m_head;     // наконечник стрелки — отдельный дочерний элемент
};

// Узел графа = функция `def name(...)` в коде. Перетаскивается мышью.
class FuncNode : public QGraphicsEllipseItem {
public:
    FuncNode(const QString& name, QColor accent);
    QString name() const { return m_name; }
    void addEdge(EdgeItem* e) { m_edges.append(e); }

protected:
    // Реагируем на перемещение/выделение — обновляем подключённые рёбра
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QString m_name;
    QColor  m_accent;
    QVector<EdgeItem*> m_edges;
};

// ─────────────────────────────────────────────────────────────
// Режим «Карта связей»: полностью заменяет редактор страницей
// QStackedWidget с интерактивным графом.
// ─────────────────────────────────────────────────────────────
class MindMapWidget : public QWidget {
    Q_OBJECT
public:
    explicit MindMapWidget(QWidget* parent = nullptr);
    // Повторный парсинг кода и перестроение графа
    void rebuildFromCode(const QString& code);

private:
    QGraphicsScene* m_scene;
    QGraphicsView*  m_view;
};