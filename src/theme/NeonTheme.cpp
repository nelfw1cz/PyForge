#include "NeonTheme.h"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>

namespace {

// Единый QSS-документ приложения. Все цвета захардкожены осознанно:
// тема одна, и её проще держать в одном месте, как «дизайн-токены».
QString baseQss() {
    return QStringLiteral(R"QSS(
/* ═══════ База ═══════ */
QWidget { background: transparent; color: #dfe5f0; font-size: 13px; outline: none; }
QToolTip { background: #1a1f28; color: #dfe5f0; border: 1px solid #39424f;
           padding: 5px 8px; border-radius: 6px; }

/* ═══════ Кастомный заголовок ═══════ */
#TitleBar { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #161a22, stop:1 #11141b);
            border-bottom: 1px solid #232a36;
            border-top-left-radius: 13px; border-top-right-radius: 13px; }
#AppTitle  { font-size: 14px; font-weight: 800; letter-spacing: 3px; color: #eef2f8; }
#AppSub    { color: #5d687c; font-size: 9px; letter-spacing: 2px; }
#ModeLabel { color: #22d3ee; font-size: 10px; letter-spacing: 3px; font-weight: 700; }

#RunBtn { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #22d3ee, stop:1 #0ea5c4);
          color: #04262e; font-weight: 800; letter-spacing: 1px;
          border: none; border-radius: 8px; padding: 6px 16px; }
#RunBtn:hover   { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #5ee5f7, stop:1 #22d3ee); }
#RunBtn:pressed { background: #0ea5c4; }

#WinBtn   { background: transparent; border: none; color: #8b93a7; border-radius: 6px; }
#WinBtn:hover   { background: #232a36; color: #dfe5f0; }
#CloseBtn { background: transparent; border: none; color: #8b93a7; border-radius: 6px; }
#CloseBtn:hover { background: #ff5c5c; color: #2a0b0b; }

/* ═══════ Sidebar-переключатель режимов ═══════ */
#Sidebar { background: #12151c; border-right: 1px solid #232a36; }
#SideBtn { background: transparent; border: 1px solid transparent; border-radius: 11px; }
#SideBtn:hover   { background: #1a2029; border-color: #2a3342; }
#SideBtn:checked { background: #182430; border-color: rgba(34,211,238,.55); }

/* ═══════ Редактор ═══════ */
#CodeEditor { background: #10131a; color: #dfe5f0;
              border: 1px solid #232a36; border-radius: 10px;
              selection-background-color: rgba(34,211,238,.22); selection-color: #ffffff; }
/* Неоновая рамка, пока активен режим RPG */
#CodeEditor[rpgMode="true"] { border: 1px solid rgba(255,77,133,.6); }
/* «Прошлое» в машине времени: приглушённые цвета + янтарная рамка */
#CodeEditor[inPast="true"]  { background: #131018; color: #b7b0c9;
                              border: 1px solid rgba(255,176,46,.5); }

/* ═══════ Консоль вывода ═══════ */
#Console { background: #0d1016; color: #9aa5b8;
           border: 1px solid #232a36; border-radius: 10px; padding: 6px; }

/* ═══════ Панель шаблонов (стандартный режим) ═══════ */
#SnippetBar { background: #12151c; border: 1px solid #232a36; border-radius: 10px; }
#DimCaption { color: #5d687c; font-size: 10px; letter-spacing: 2px; }
#SnippetChip { background: #1a2029; border: 1px solid #2a3342; color: #c6cedd;
               border-radius: 7px; padding: 4px 10px; }
#SnippetChip:hover { border-color: #22d3ee; color: #22d3ee; }

/* ═══════ Игровой HUD (RPG) ═══════ */
#HudPanel { background: #12151c; border: 1px solid rgba(255,77,133,.35); border-radius: 12px; }
#LevelBadge { background: #1a1020; border: 1px solid #a78bfa; border-radius: 9px;
              color: #c4b5fd; font-weight: 800; letter-spacing: 1px; }
#ComboLabel { color: #5d687c; font-size: 18px; font-weight: 800; }
#ComboLabel[hot="true"] { color: #ffb02e; }

/* ═══════ Машина времени ═══════ */
#TimeBar { background: #12151c; border: 1px solid rgba(255,176,46,.35); border-radius: 12px; }
#TimeCaption { color: #ffb02e; letter-spacing: 2px; font-size: 10px; font-weight: 700; }
#TimeInfo { color: #8b93a7; font-size: 11px; }
#TimeInfo[past="true"] { color: #ffb02e; font-weight: 700; }

QSlider#TimeSlider::groove:horizontal { height: 6px; background: #1c2129; border-radius: 3px; }
QSlider#TimeSlider::sub-page:horizontal {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #b45309, stop:1 #ffb02e);
    border-radius: 3px; }
QSlider#TimeSlider::handle:horizontal {
    width: 16px; height: 16px; margin: -6px 0; border-radius: 9px;
    background: #ffb02e; border: 2px solid #ffe0a3; }
QSlider#TimeSlider::handle:horizontal:hover { background: #ffd166; }

/* ═══════ Слой заметок ═══════ */
#PadBtn { background: rgba(20,24,32,.92); border: 1px solid #3a3350; color: #cbb8ff;
          border-radius: 8px; padding: 5px 12px; }
#PadBtn:hover   { border-color: #a78bfa; }
#PadBtn:checked { background: #2a2140; border-color: #a78bfa; color: #e9defc; }

/* ═══════ Карта связей ═══════ */
QGraphicsView { background: #0d0f13; border: none; }
#MapHint { color: #5d687c; font-size: 11px; padding: 6px;
           background: #12151c; border-top: 1px solid #232a36; }

/* ═══════ Скроллбары ═══════ */
QScrollBar:vertical   { background: transparent; width: 10px; margin: 2px; }
QScrollBar:horizontal { background: transparent; height: 10px; margin: 2px; }
QScrollBar::handle:vertical   { background: #2a3140; border-radius: 5px; min-height: 30px; }
QScrollBar::handle:horizontal { background: #2a3140; border-radius: 5px; min-width: 30px; }
QScrollBar::handle:hover { background: #3b4456; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
)QSS");
}

} // namespace

void NeonTheme::apply() {
    QFont f("JetBrains Mono");
    f.setStyleHint(QFont::Monospace);
    f.setPointSize(10);
    qApp->setFont(f);
    qApp->setStyle("Fusion");     // предсказуемая база для QSS
    qApp->setStyleSheet(baseQss());
}

QIcon NeonTheme::icon(const QString& kind) {
    QPixmap px(44, 44);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);

    // У каждого режима — свой неоновый цвет иконки
    QColor c = Neon::cyan();
    if      (kind == "rpg")   c = Neon::pink();
    else if (kind == "graph") c = Neon::green();
    else if (kind == "clock") c = Neon::amber();
    else if (kind == "pen")   c = Neon::violet();

    QPen pen(c, 2.6);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);

    if (kind == "code") {                     // </>
        p.drawLine(15, 12, 7, 22);  p.drawLine(7, 22, 15, 32);
        p.drawLine(29, 12, 37, 22); p.drawLine(37, 22, 29, 32);
        p.drawLine(24, 9, 20, 35);
    } else if (kind == "rpg") {               // молния
        QPolygonF bolt{ {25,5}, {12,25}, {21,25}, {18,39}, {33,18}, {24,18} };
        p.setBrush(c);
        p.drawPolygon(bolt);
    } else if (kind == "graph") {             // три узла, соединённые рёбрами
        p.drawLine(13, 14, 31, 11);
        p.drawLine(13, 14, 24, 33);
        p.drawLine(31, 11, 24, 33);
        p.setBrush(QColor(16, 19, 25));
        p.drawEllipse(QPointF(13, 14), 5.5, 5.5);
        p.drawEllipse(QPointF(31, 11), 5.5, 5.5);
        p.drawEllipse(QPointF(24, 33), 6.5, 6.5);
    } else if (kind == "clock") {             // часы
        p.drawEllipse(QPointF(22, 22), 14, 14);
        p.drawLine(22, 22, 22, 12);
        p.drawLine(22, 22, 30, 25);
    } else if (kind == "pen") {               // перо
        p.drawLine(10, 34, 30, 14);
        QPolygonF tip{ {10,34}, {15,33}, {11,29} };
        p.setBrush(c);
        p.drawPolygon(tip);
        p.drawLine(27, 11, 33, 17);
    } else if (kind == "sun") {               // Солнышко для RTX режима
        p.setPen(QPen(c, 2.5));
        p.setBrush(c); // Желтое ядро
        
        // Центр круга
        QPointF center(22, 22);
        
        // Само солнце (круг)
        p.drawEllipse(center, 7, 7);
        
        // Лучики вокруг
        for (int i = 0; i < 8; ++i) {
            double angle = i * M_PI / 4.0;
            QPointF start(std::cos(angle) * 11 + 22, std::sin(angle) * 11 + 22);
            QPointF end(std::cos(angle) * 17 + 22, std::sin(angle) * 17 + 22);
            p.drawLine(start, end);
        }
    }
    return QIcon(px);
}