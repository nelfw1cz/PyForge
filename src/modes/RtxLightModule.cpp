#include "RtxLightModule.h"
#include "editor/CodeEditor.h"
#include "theme/NeonTheme.h"

#include <QPainter>
#include <QRadialGradient>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <cmath>

// ═══════════════════ ВНУТРЕННИЙ КЛАСС OVERLAY ═══════════════════
// Этот класс рисует свет и ловит мышь. Он находится в .cpp файле,
// так как нужен только этому модулю.

class LightOverlay : public QWidget {
public:
    LightOverlay(RtxLightModule* mod, CodeEditor* ed) 
        : QWidget(ed->parentWidget()), m_mod(mod), m_editor(ed) {
        setMouseTracking(true);
        hide(); // Скрыт по умолчанию
    }

    // ГЛАВНЫЙ МЕТОД РАСЧЕТА ТЕНЕЙ (PUBLIC, чтобы вызывать из activate/deactivate)
    void updateShadows() {
        if (!m_editor || !m_mod) return;

        const QPointF center(m_editor->width() / 2.0, m_editor->height() / 2.0);
        const QPointF vec = center - m_mod->m_lightPos;
        
        const qreal len = qMax(1.0, std::hypot(vec.x(), vec.y()));
        const QPointF dir = vec / len;
        
        const qreal distFactor = qMin(len / 300.0, 1.0); 
        
        const int dx = int(dir.x() * 15 * distFactor);
        const int dy = int(dir.y() * 15 * distFactor);
        const int blur = 10 + int(distFactor * 20);
        
        auto* shadow = dynamic_cast<QGraphicsDropShadowEffect*>(m_editor->graphicsEffect());
        if (!shadow) {
            shadow = new QGraphicsDropShadowEffect;
            shadow->setColor(QColor(0, 0, 0, 180));
            m_editor->setGraphicsEffect(shadow);
        }
        
        shadow->setOffset(dx, dy);
        shadow->setBlurRadius(blur);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        if (!isVisible()) return;
        
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        
        const QPointF light = m_mod->m_lightPos;
        
        // Аура света
        QRadialGradient grad(light, kMaxGlow * 2.5);
        grad.setColorAt(0.0, QColor(255, 220, 180, 60));   
        grad.setColorAt(0.3, QColor(255, 180, 100, 30));   
        grad.setColorAt(1.0, QColor(0, 0, 0, 0));          
        p.setPen(Qt::NoPen);
        p.setBrush(grad);
        p.drawEllipse(light, kMaxGlow * 2.5, kMaxGlow * 2.5);
        
        // Яркое ядро
        QRadialGradient coreGrad(light, kLightRadius);
        coreGrad.setColorAt(0.0, QColor(255, 255, 255, 255));
        coreGrad.setColorAt(0.7, QColor(255, 230, 180, 200));
        coreGrad.setColorAt(1.0, QColor(255, 180, 100, 0));
        p.setBrush(coreGrad);
        p.drawEllipse(light, kLightRadius, kLightRadius);
    }
    
    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            const qreal dist = std::hypot(e->position().x() - m_mod->m_lightPos.x(),
                                           e->position().y() - m_mod->m_lightPos.y());
            if (dist < kLightRadius * 2) {
                m_mod->m_dragging = true;
                e->accept();
            } else {
                m_mod->m_lightPos = e->position().toPoint();
                updateShadows();
                update();
                e->accept();
            }
        }
    }
    
    void mouseMoveEvent(QMouseEvent* e) override {
        if (m_mod->m_dragging) {
            m_mod->m_lightPos = e->position().toPoint();
            updateShadows();
            update();
            e->accept();
        }
    }
    
    void mouseReleaseEvent(QMouseEvent* e) override {
        m_mod->m_dragging = false;
        e->accept();
    }
    
    static constexpr int kMaxGlow = 150;
    static constexpr int kLightRadius = 30;
    
    RtxLightModule* m_mod;
    CodeEditor* m_editor;
};

// ═══════════════════ РЕАЛИЗАЦИЯ МОДУЛЯ ═══════════════════

RtxLightModule::RtxLightModule(CodeEditor* editor, QWidget* host, QObject* parent)
    : QObject(parent), m_editor(editor), m_host(host) {
    // Создаем оверлей и ставим его родителем хост (stage)
    m_overlay = new LightOverlay(this, editor);
    m_overlay->setParent(host); 
    
    // Слушаем ресайз редактора через фильтр событий
    editor->installEventFilter(this);
}

void RtxLightModule::activate() {
    // Ставим свет в центр окна при включении режима
    m_lightPos = QPoint(m_editor->width() / 2, m_editor->height() / 2);
    
    m_overlay->show();
    m_overlay->raise();
    m_overlay->updateGeometry(); // Синхронизируем размер
    m_overlay->updateShadows();  // Рассчитываем тени
}

void RtxLightModule::deactivate() {
    m_overlay->hide();
    
    // Удаляем эффект тени с редактора, чтобы не грузить систему зря
    if (auto* sh = dynamic_cast<QGraphicsDropShadowEffect*>(m_editor->graphicsEffect())) {
        delete sh;
        m_editor->setGraphicsEffect(nullptr);
    }
}

bool RtxLightModule::eventFilter(QObject* obj, QEvent* e) {
    if (obj == m_editor && e->type() == QEvent::Resize) {
        // При изменении размера редактора обновляем геометрию оверлея
        m_overlay->setGeometry(m_editor->geometry());
        m_overlay->updateShadows();
    }
    return QObject::eventFilter(obj, e);
}