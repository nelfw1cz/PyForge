#include "ScratchpadModule.h"
#include "editor/CodeEditor.h"
#include "theme/NeonTheme.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QResizeEvent>
#include <cmath>

namespace {
QPointF rotated(QPointF v, qreal ang) {
    const qreal c = std::cos(ang), s = std::sin(ang);
    return {v.x() * c - v.y() * s, v.x() * s + v.y() * c};
}
} // namespace

ScratchpadOverlay::ScratchpadOverlay(CodeEditor* editor) : QWidget(editor) {
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    hide();                                // по умолчанию слой выключен

    // Мини-тулбар: кнопки — дочерние виджеты, поэтому клики по ним
    // не начинают рисование (дети получают события раньше родителя)
    auto* marker = new QPushButton(QStringLiteral("маркер"), this);
    auto* arrow  = new QPushButton(QStringLiteral("стрелка"), this);
    auto* clearB = new QPushButton(QStringLiteral("очистить"), this);
    for (auto* b : {marker, arrow, clearB}) {
        b->setObjectName("PadBtn");
        b->setCursor(Qt::PointingHandCursor);
        m_tools.append(b);
    }
    marker->setCheckable(true);
    arrow->setCheckable(true);
    marker->setChecked(true);

    auto* group = new QButtonGroup(this);  // exclusive: маркер ИЛИ стрелка
    group->addButton(marker);
    group->addButton(arrow);

    connect(marker, &QPushButton::clicked, this, [this]{ m_tool = PadShape::Stroke; });
    connect(arrow,  &QPushButton::clicked, this, [this]{ m_tool = PadShape::Arrow;  });
    connect(clearB, &QPushButton::clicked, this, [this]{ m_shapes.clear(); update(); });
}

void ScratchpadOverlay::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    layoutTools();
}

void ScratchpadOverlay::layoutTools() {
    // Кнопки прижаты к правому верхнему углу оверлея
    int x = width() - 12;
    for (QPushButton* b : m_tools) {
        const QSize s = b->sizeHint();
        x -= s.width();
        b->setGeometry(x, 10, s.width(), s.height());
        x -= 8;
    }
}

// ── Мышь: рисование ───────────────────────────────────────────
void ScratchpadOverlay::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_drawing = true;
        PadShape s;
        s.kind = m_tool;
        s.points << e->position() << e->position();
        m_shapes.append(s);
        update();
    }
}

void ScratchpadOverlay::mouseMoveEvent(QMouseEvent* e) {
    if (m_drawing && !m_shapes.isEmpty()) {
        PadShape& s = m_shapes.last();
        if (s.kind == PadShape::Stroke)
            s.points.append(e->position());        // наращиваем полилинию
        else
            s.points.last() = e->position();       // растягиваем стрелку
        update();
    }
}

void ScratchpadOverlay::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_drawing = false;
        update();
    }
}

// ── Отрисовка всех заметок ────────────────────────────────────
void ScratchpadOverlay::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Рамка слоя, чтобы было видно, что «плёнка» заметок активна
    p.setPen(QPen(QColor(167, 139, 250, 120), 2));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);

    p.setPen(QColor(167, 139, 250, 170));
    p.drawText(16, 26, QStringLiteral("РЕЖИМ ЗАМЕТОК · рисуйте мышью поверх кода"));

    const QColor markerColor(255, 70, 70, 200);    // красный полупрозрачный маркер
    for (const PadShape& s : m_shapes) {
        if (s.points.size() < 2) continue;

        if (s.kind == PadShape::Stroke) {
            QPainterPath path(s.points.first());
            for (int i = 1; i < s.points.size(); ++i) path.lineTo(s.points[i]);
            p.setPen(QPen(markerColor, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            p.drawPath(path);
        } else {
            const QPointF a = s.points.first();
            const QPointF b = s.points.last();
            p.setPen(QPen(Neon::amber(), 3, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(a, b);
            // Усики стрелки под ±28° от направления
            QPointF t = b - a;
            const qreal len = qMax(1.0, std::hypot(t.x(), t.y()));
            t /= len;
            p.drawLine(b, b - rotated(t,  0.5) * 16);
            p.drawLine(b, b - rotated(t, -0.5) * 16);
        }
    }
}

// ═══════════════════ ScratchpadModule ═══════════════════

ScratchpadModule::ScratchpadModule(CodeEditor* editor, QObject* parent)
    : QObject(parent), m_editor(editor) {
    m_overlay = new ScratchpadOverlay(editor);       // родитель — сам редактор
    editor->installEventFilter(this);                // следим за его размером
}

bool ScratchpadModule::eventFilter(QObject* obj, QEvent* e) {
    // Оверлей всегда растянут на весь редактор, включая панель номеров
    if (obj == m_editor && e->type() == QEvent::Resize)
        m_overlay->setGeometry(m_editor->rect());
    return QObject::eventFilter(obj, e);
}

void ScratchpadModule::activate() {
    m_overlay->setGeometry(m_editor->rect());
    m_overlay->show();
    m_overlay->raise();
}

void ScratchpadModule::deactivate() {
    m_overlay->hide();   // рисунки остаются в памяти — вернутся при повторном входе
}