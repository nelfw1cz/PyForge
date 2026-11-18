#include "JungleTheme.h"
#include <QPainter>
#include <QPixmap>
#include <QtMath>

QString JungleTheme::styleSheet() {
    return QStringLiteral(R"(
        /* ═══════════ JUNGLE THEME ═══════════ */
        QWidget { 
            background-color: #122016; 
            color: #c8e6c9; 
            font-family: 'Consolas', monospace;
        }
        
        /* Редактор */
        QPlainTextEdit#CodeEditor {
            background-color: #1a2c1e;
            border: 2px solid #4caf50;
            border-radius: 8px;
            selection-background-color: rgba(76, 175, 80, 80);
        }
        
        /* Sidebar */
        QWidget#Sidebar {
            background-color: #0d1a10;
            border-right: 2px solid #2e7d32;
        }
        QPushButton#SideBtn {
            background-color: transparent;
            border: 2px solid transparent;
            border-radius: 10px;
        }
        QPushButton#SideBtn:hover {
            background-color: rgba(76, 175, 80, 30);
            border-color: #4caf50;
        }
        QPushButton#SideBtn:checked {
            background-color: rgba(255, 193, 7, 40);
            border-color: #ffc107;
        }
        
        /* SnippetBar - стеклянные листья */
        QWidget#SnippetBar {
            background-color: rgba(26, 44, 30, 200);
            border: 1px solid rgba(76, 175, 80, 60);
            border-radius: 12px;
        }
        QPushButton#SnippetBtn {
            background-color: rgba(76, 175, 80, 25);
            border: 1px solid rgba(76, 175, 80, 50);
            border-radius: 8px;
            color: #c8e6c9;
            padding: 6px 14px;
        }
        QPushButton#SnippetBtn:hover {
            background-color: rgba(255, 193, 7, 50);
            border-color: #ffc107;
        }
        
        /* Консоль */
        QPlainTextEdit#Console {
            background-color: rgba(13, 26, 16, 220);
            border: 1px solid rgba(76, 175, 80, 40);
            border-radius: 10px;
            color: #a5d6a7;
        }
        
        /* TitleBar */
        QWidget#TitleBar {
            background-color: #0d1a10;
            border-bottom: 2px solid #2e7d32;
        }
        QLabel#AppName {
            color: #ffc107;
            font-weight: bold;
            font-size: 14px;
        }
        
        /* Кнопка запуска */
        QPushButton#RunBtn {
            background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #4caf50, stop:1 #8bc34a);
            border: none;
            border-radius: 8px;
            color: #0d1a10;
            font-weight: bold;
            padding: 8px 20px;
        }
        QPushButton#RunBtn:hover {
            background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #66bb6a, stop:1 #9ccc65);
        }
        
        /* Scrollbars */
        QScrollBar:vertical {
            background: #0d1a10;
            width: 10px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background: #4caf50;
            border-radius: 5px;
            min-height: 30px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )");
}

QIcon JungleTheme::icon(const QString& kind) {
    QPixmap px(44, 44);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    
    if (kind == "code") {
        p.setPen(QPen(leafGreen, 3));
        p.drawText(px.rect(), Qt::AlignCenter, "</>");
    } else if (kind == "rpg") {
        p.setPen(QPen(sunGold, 3));
        p.drawText(px.rect(), Qt::AlignCenter, "⚡");
    } else if (kind == "graph") {
        p.setPen(QPen(leafGreen, 2.5));
        p.drawEllipse(QPointF(22, 14), 5, 5);
        p.drawEllipse(QPointF(12, 30), 5, 5);
        p.drawEllipse(QPointF(32, 30), 5, 5);
        p.drawLine(22, 19, 12, 25);
        p.drawLine(22, 19, 32, 25);
    } else if (kind == "clock") {
        p.setPen(QPen(sunGold, 2.5));
        p.drawEllipse(QPointF(22, 22), 12, 12);
        p.drawLine(22, 22, 22, 14);
        p.drawLine(22, 22, 28, 22);
    } else if (kind == "pen") {
        p.setPen(QPen(flowerPink, 2.5));
        p.drawLine(12, 32, 32, 12);
        p.drawLine(32, 12, 36, 16);
        p.drawLine(12, 32, 8, 36);
    } else if (kind == "sun") {
        // Для Jungle это солнце сквозь кроны
        p.setBrush(sunGold);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(22, 22), 8, 8);
        p.setPen(QPen(sunGold, 2));
        for (int i = 0; i < 8; ++i) {
            double a = i * M_PI / 4;
            p.drawLine(QPointF(22 + cos(a)*12, 22 + sin(a)*12),
                       QPointF(22 + cos(a)*17, 22 + sin(a)*17));
        }
    }
    p.end();
    return QIcon(px);
}