#pragma once
#include <QObject>
#include <QWidget>
#include <QPoint>
#include <QColor>

class CodeEditor;
class LightOverlay; // Forward declaration

// ─────────────────────────────────────────────────────────────
// Режим "RTX": динамический источник света поверх редактора.
// ─────────────────────────────────────────────────────────────
class RtxLightModule : public QObject {
    Q_OBJECT
    
    // Разрешаем внутреннему оверлею доступ к нашим приватным данным
    friend class LightOverlay; 

public:
    RtxLightModule(CodeEditor* editor, QWidget* host, QObject* parent = nullptr);
    
    void activate();
    void deactivate();

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    CodeEditor* m_editor;
    QWidget* m_host;       // Виджет-хост (stage)
    
    QPoint m_lightPos;     // Позиция шара
    bool m_dragging = false; 
    
    LightOverlay* m_overlay = nullptr; // Указатель на наш рисующий виджет
    
    static constexpr int kLightRadius = 40;
};