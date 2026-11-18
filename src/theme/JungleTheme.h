#pragma once
#include <QString>
#include <QIcon>
#include <QColor>

namespace JungleTheme {
    // Палитра: глубокий зелёный, листва, земля, солнечные лучи
    constexpr QColor bgDeep      = QColor(18, 32, 22);    // Тёмная зелень
    constexpr QColor bgPanel     = QColor(26, 44, 30);    // Панели
    constexpr QColor leafGreen   = QColor(76, 175, 80);   // Листва
    constexpr QColor sunGold     = QColor(255, 193, 7);   // Солнце сквозь кроны
    constexpr QColor earthBrown  = QColor(121, 85, 72);   // Земля/кора
    constexpr QColor mistWhite   = QColor(200, 230, 201); // Туман/текст
    constexpr QColor flowerPink  = QColor(233, 30, 99);   // Цветы/акценты
    
    QString styleSheet();
    QIcon icon(const QString& kind);
}