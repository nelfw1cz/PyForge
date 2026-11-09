#pragma once
#include <QColor>
#include <QIcon>

// ─────────────────────────────────────────────────────────────
// Палитра Anthracite/Neon. Каждый режим имеет «свой» неон,
// чтобы пользователь мгновенно понимал, где находится.
// ─────────────────────────────────────────────────────────────
namespace Neon {

// Глубокий графит (база интерфейса)
inline QColor bg0()  { return {11, 13, 17}; }   // окно
inline QColor bg1()  { return {16, 19, 25}; }   // редактор, фон сцены
inline QColor bg2()  { return {23, 27, 35}; }   // приподнятые панели
inline QColor line() { return {38, 44, 56}; }   // разделители/рамки
inline QColor text() { return {223, 229, 240}; }
inline QColor dim()  { return {126, 136, 156}; }

// Неоновые акценты режимов
inline QColor cyan()   { return {34, 211, 238}; }  // стандартный режим
inline QColor pink()   { return {255, 77, 133}; }  // Code-RPG
inline QColor green()  { return {61, 255, 158}; }  // карта связей
inline QColor amber()  { return {255, 176, 46}; }  // машина времени
inline QColor violet() { return {167, 139, 250}; } // заметки
inline QColor red()    { return {255, 92, 92}; }

} // namespace Neon

namespace NeonTheme {
// Применяет шрифт, стиль-базу Fusion и глобальный QSS ко всему приложению
void apply();
// Рисует иконки кодом (QPainter) — не зависят от системных шрифтов/тем.
// kind: "code" | "rpg" | "graph" | "clock" | "pen"
QIcon icon(const QString& kind);
}