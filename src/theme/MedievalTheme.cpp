#include "MedievalTheme.h"
#include <QPainter>
#include <QPixmap>
#include <QtMath>

QString MedievalTheme::styleSheet() {
    return QStringLiteral(R"(
        /* ═══════════ MEDIEVAL PIXEL-RPG THEME ═══════════ */
        QWidget { 
            background-color: #1e1c23; 
            color: #d2b48c; 
            font-family: 'Courier New', monospace;
        }
        
        /* Редактор - свиток кода */
        QPlainTextEdit#CodeEditor {
            background-color: #2a2630;
            border: 3px solid #504658;
            border-radius: 4px; /* Меньше скругления = пиксельнее */
            selection-background-color: rgba(255, 140, 0, 60);
        }
        
        /* Sidebar - каменная колонна */
        QWidget#Sidebar {
            background-color: #14121a;
            border-right: 3px solid #3c3448;
        }
        QPushButton#SideBtn {
            background-color: transparent;
            border: 2px solid transparent;
            border-radius: 4px;
        }
        QPushButton#SideBtn:hover {
            background-color: rgba(255, 140, 0, 30);
            border-color: #ff8c00;
        }
        QPushButton#SideBtn:checked {
            background-color: rgba(255, 200, 50, 40);
            border-color: #ffc832;
        }
        
        /* SnippetBar - деревянная доска объявлений */
        QWidget#SnippetBar {
            background-color: rgba(50, 46, 58, 220);
            border: 2px solid #8b7355;
            border-radius: 4px;
        }
        QPushButton#SnippetBtn {
            background-color: rgba(139, 115, 85, 40);
            border: 2px solid #8b7355;
            border-radius: 3px;
            color: #d2b48c;
            padding: 5px 12px;
            font-weight: bold;
        }
        QPushButton#SnippetBtn:hover {
            background-color: rgba(255, 140, 0, 50);
            border-color: #ff8c00;
        }
        
        /* Консоль - летопись */
        QPlainTextEdit#Console {
            background-color: rgba(20, 18, 26, 230);
            border: 2px solid #504658;
            border-radius: 4px;
            color: #b8a080;
        }
        
        /* TitleBar - гербовый щит */
        QWidget#TitleBar {
            background-color: #14121a;
            border-bottom: 3px solid #3c3448;
        }
        QLabel#AppName {
            color: #ffc832;
            font-weight: bold;
            font-size: 14px;
        }
        
        /* Кнопка запуска - меч */
        QPushButton#RunBtn {
            background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #b41e1e, stop:1 #ff8c00);
            border: 2px solid #ffc832;
            border-radius: 4px;
            color: #fff;
            font-weight: bold;
            padding: 8px 20px;
        }
        QPushButton#RunBtn:hover {
            background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #d42e2e, stop:1 #ffa030);
        }
        
        /* Scrollbars - цепи */
        QScrollBar:vertical {
            background: #14121a;
            width: 12px;
        }
        QScrollBar::handle:vertical {
            background: #504658;
            border: 1px solid #8b7355;
            min-height: 30px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        
        /* RPG HUD - панель героя */
        QWidget#RpgHud {
            background-color: rgba(30, 28, 35, 220);
            border: 2px solid #ffc832;
            border-radius: 4px;
        }
        QProgressBar {
            background-color: rgba(0, 0, 0, 150);
            border: 1px solid #504658;
            border-radius: 2px;
            height: 10px;
        }
        QProgressBar::chunk {
            background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #b41e1e, stop:1 #ffc832);
        }
    )");
}

QIcon MedievalTheme::icon(const QString& kind) {
    QPixmap px(44, 44);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing, false); // Пиксельный стиль!
    
    if (kind == "code") {
        p.setPen(QPen(parchment, 3));
        p.drawText(px.rect(), Qt::AlignCenter, "</>");
    } else if (kind == "rpg") {
        // Меч вместо молнии
        p.setPen(QPen(torchOrange, 3));
        p.drawLine(22, 8, 22, 32);
        p.drawLine(14, 16, 30, 16);
        p.drawLine(18, 32, 26, 32);
    } else if (kind == "graph") {
        // Замок/башни
        p.setPen(QPen(stoneMid, 2.5));
        p.drawRect(10, 20, 8, 16);
        p.drawRect(26, 20, 8, 16);
        p.drawRect(18, 14, 8, 22);
        p.drawLine(10, 20, 14, 14);
        p.drawLine(26, 20, 30, 14);
    } else if (kind == "clock") {
        // Песочные часы
        p.setPen(QPen(goldCoin, 2.5));
        p.drawLine(14, 10, 30, 10);
        p.drawLine(14, 34, 30, 34);
        p.drawLine(14, 10, 30, 34);
        p.drawLine(30, 10, 14, 34);
    } else if (kind == "pen") {
        // Перо
        p.setPen(QPen(parchment, 2.5));
        p.drawLine(12, 34, 30, 10);
        p.drawLine(30, 10, 34, 14);
        p.drawLine(12, 34, 8, 38);
    } else if (kind == "sun") {
        // 🔥 ФАКЕЛ вместо солнца!
        p.setBrush(torchOrange);
        p.setPen(Qt::NoPen);
        // Древко факела
        p.setBrush(QColor(101, 67, 33));
        p.drawRect(20, 24, 4, 14);
        // Пламя
        p.setBrush(torchYellow);
        p.drawEllipse(QPointF(22, 18), 6, 8);
        p.setBrush(torchOrange);
        p.drawEllipse(QPointF(22, 16), 4, 6);
    }
    p.end();
    return QIcon(px);
}