#include "MindMapWidget.h"
#include "theme/NeonTheme.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <QWheelEvent>
#include <QtMath>
#include <QHash>
#include <QSet>
#include <cmath>

namespace {
constexpr qreal kNodeRadius = 46;   // радиус круга-ноды

// Вид графа: тёмный фон в точку + зум по Ctrl+Колесо
class MapView : public QGraphicsView {
public:
    using QGraphicsView::QGraphicsView;
protected:
    void drawBackground(QPainter* p, const QRectF& rect) override {
        p->fillRect(rect, QColor(13, 15, 20));
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(255, 255, 255, 16));
        const int step = 28;
        const int left = int(rect.left() / step) * step;
        const int top  = int(rect.top()  / step) * step;
        for (int x = left; x < rect.right(); x += step)
            for (int y = top; y < rect.bottom(); y += step)
                p->drawEllipse(QPointF(x, y), 1.1, 1.1);
    }
    void wheelEvent(QWheelEvent* e) override {
        if (e->modifiers() & Qt::ControlModifier) {
            qreal factor = (e->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15); // <-- ДОЛЖНО БЫТЬ qreal
            scale(factor, factor);
            e->accept();
        } else {
            QGraphicsView::wheelEvent(e);
        }
    }
};

QPointF rotated(QPointF v, qreal ang) {
    const qreal c = std::cos(ang), s = std::sin(ang);
    return {v.x() * c - v.y() * s, v.x() * s + v.y() * c};
}
} // namespace

// ═══════════════════ FuncNode ═══════════════════

FuncNode::FuncNode(const QString& name, QColor accent)
    : QGraphicsEllipseItem(-kNodeRadius, -kNodeRadius, kNodeRadius * 2, kNodeRadius * 2),
      m_name(name), m_accent(accent) {
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);   // чтобы приходили уведомления о перемещении
    setBrush(QColor(20, 24, 32));
    setPen(QPen(accent, 2.5));           // неоновая обводка
    setZValue(1);                        // ноды поверх рёбер

    // Мягкое свечение вокруг круга
    auto* glow = new QGraphicsDropShadowEffect;
    glow->setColor(QColor(accent.red(), accent.green(), accent.blue(), 140));
    glow->setBlurRadius(22);
    glow->setOffset(0);
    setGraphicsEffect(glow);

    // Имя функции по центру круга
    auto* label = new QGraphicsSimpleTextItem(name, this);
    QFont f = label->font();
    f.setBold(true);
    f.setPointSize(10);
    label->setFont(f);
    label->setBrush(QColor(232, 238, 248));
    const QRectF b = label->boundingRect();
    label->setPos(-b.width() / 2, -b.height() / 2);
}

QVariant FuncNode::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged)
        for (EdgeItem* e : m_edges) e->rebuild();      // тянем ноду — тянутся и связи
    if (change == ItemSelectedHasChanged)
        setPen(QPen(m_accent, value.toBool() ? 4.0 : 2.5));
    return QGraphicsEllipseItem::itemChange(change, value);
}

// ═══════════════════ EdgeItem ═══════════════════

EdgeItem::EdgeItem(FuncNode* from, FuncNode* to) : m_from(from), m_to(to) {
    setZValue(0);
    setPen(QPen(QColor(94, 234, 212, 160), 2));
    setBrush(Qt::NoBrush);                 // саму кривую не заливаем
    // Стрелка — дочерний полигон: у неё своя заливка, у кривой — нет
    m_head = new QGraphicsPolygonItem(this);
    m_head->setBrush(QColor(94, 234, 212, 200));
    m_head->setPen(Qt::NoPen);
    from->addEdge(this);
    to->addEdge(this);
    rebuild();
}

void EdgeItem::rebuild() {
    const QPointF a = m_from->pos();
    const QPointF b = m_to->pos();
    const QPointF dir = b - a;
    const qreal len = qMax(1.0, std::hypot(dir.x(), dir.y()));
    const QPointF u = dir / len;                       // единичный вектор A→B

    const QPointF start = a + u * (kNodeRadius + 6);   // выходим из-под окружности
    const QPointF tip   = b - u * (kNodeRadius + 8);   // остриё стрелки у края ноды

    // Лёгкий изгиб квадратичной кривой; нормаль гарантирует, что встречные
    // рёбра A→B и B→A не сольются в одну линию
    const QPointF n(-u.y(), u.x());
    const QPointF mid = (start + tip) / 2 + n * qBound(16.0, len * 0.12, 50.0);

    QPainterPath path;
    path.moveTo(start);
    path.quadTo(mid, tip);
    setPath(path);

    // Наконечник: касательная в конце квадратики — это (tip - mid)
    QPointF t = tip - mid;
    const qreal tl = qMax(1.0, std::hypot(t.x(), t.y()));
    t /= tl;
    QPolygonF head;
    head << tip
         << tip - rotated(t,  0.45) * 13
         << tip - rotated(t, -0.45) * 13;
    m_head->setPolygon(head);
}

// ═══════════════════ MindMapWidget ═══════════════════

MindMapWidget::MindMapWidget(QWidget* parent) : QWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-900, -600, 1800, 1200);

    m_view = new MapView;
    m_view->setScene(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);      // пан пустой области
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    lay->addWidget(m_view, 1);

    auto* hint = new QLabel(QStringLiteral(
        "Перетаскивайте узлы мышью  ·  рёбра — вызовы функций  ·  Ctrl+колесо — масштаб  ·  граф перестраивается при входе в режим"),
        this);
    hint->setObjectName("MapHint");
    hint->setAlignment(Qt::AlignCenter);
    lay->addWidget(hint);
}

void MindMapWidget::rebuildFromCode(const QString& code) {
    m_scene->clear();                       // удаляет ноды, рёбра и их эффекты
    m_view->resetTransform();
    m_view->centerOn(0, 0);

    struct Fn { QString name; int defLine; int endLine; };
    QVector<Fn> fns;

    // 1) Регуляркой ищем все определения: `def имя(`
    QRegularExpression defRe(QStringLiteral(R"re(^\s*def\s+([A-Za-z_]\w*)\s*\()re"),
                             QRegularExpression::MultilineOption);
    auto it = defRe.globalMatch(code);
    while (it.hasNext()) {
        const auto m = it.next();
        const int line = code.left(m.capturedStart()).count('\n');
        fns.append({m.captured(1), line, -1});
    }

    if (fns.isEmpty()) {
        auto* t = m_scene->addSimpleText(QStringLiteral(
            "В коде пока нет ни одной функции — добавьте `def …` и вернитесь в режим."));
        QFont f = t->font(); f.setPointSize(14); t->setFont(f);
        t->setBrush(Neon::dim());
        t->setPos(-t->boundingRect().width() / 2, 0);
        return;
    }

    // 2) Границы тела функции: от её def до следующего def.
    //    Упрощение прото-типа: вложенные функции не поддерживаются.
    const int lastLine = code.count('\n');
    for (int i = 0; i < fns.size(); ++i)
        fns[i].endLine = (i + 1 < fns.size()) ? fns[i + 1].defLine - 1 : lastLine;

    // 3) Раскладываем ноды по окружности
    const QStringList lines = code.split('\n');
    QHash<QString, FuncNode*> nodes;
    const QColor palette[] = {Neon::cyan(), Neon::pink(), Neon::green(),
                              Neon::amber(), Neon::violet()};
    const qreal R = 170 + 26 * fns.size();
    for (int i = 0; i < fns.size(); ++i) {
        const qreal ang = (2 * M_PI * i) / fns.size() - M_PI / 2;
        auto* node = new FuncNode(fns[i].name, palette[i % 5]);
        node->setPos(R * std::cos(ang), R * std::sin(ang));
        m_scene->addItem(node);
        nodes.insert(fns[i].name, node);
    }

    // 4) Ищем вызовы: в теле функции ищем `имя(` для каждой известной функции
    QSet<QString> added;                   // защита от дублей рёбер
    for (const Fn& f : fns) {
        for (const Fn& g : fns) {
            if (f.name == g.name) continue;              // рекурсию не рисуем
            const QRegularExpression callRe(
                QStringLiteral(R"re(\b%1\s*\()re").arg(QRegularExpression::escape(g.name)));
            bool found = false;
            for (int l = f.defLine + 1; l <= f.endLine && l < lines.size() && !found; ++l)
                if (callRe.match(lines[l]).hasMatch()) found = true;
            const QString key = f.name + ">" + g.name;
            if (found && !added.contains(key)) {
                added.insert(key);
                m_scene->addItem(new EdgeItem(nodes[f.name], nodes[g.name]));
            }
        }
    }
}