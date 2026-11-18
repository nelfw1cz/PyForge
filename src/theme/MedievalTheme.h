#pragma once
#include <QString>
#include <QIcon>
#include <QColor>

namespace MedievalTheme {
    // Палитра: камень замка, факелы, пергамент, кровь дракона
    constexpr QColor stoneDark   = QColor(30, 28, 35);    // Тёмный камень
    constexpr QColor stoneMid    = QColor(50, 46, 58);    // Стены замка
    constexpr QColor torchOrange = QColor(255, 140, 0);   // Пламя факела
    constexpr QColor torchYellow = QColor(255, 200, 50);  // Ядро пламени
    constexpr QColor parchment   = QColor(210, 180, 140); // Пергамент/текст
    constexpr QColor bloodRed    = QColor(180, 30, 30);   // HP/опасность
    constexpr QColor royalBlue   = QColor(60, 80, 160);   // Мана/магия
    constexpr QColor goldCoin    = QColor(255, 215, 0);   // Золото/XP
    
    QString styleSheet();
    QIcon icon(const QString& kind);
}