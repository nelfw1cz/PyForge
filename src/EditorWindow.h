#pragma once
#include <QWidget>
#include <QRect>
#include <QPoint>
#include <QPixmap>      // Для работы с картинками
#include <QPalette> 

class QStackedWidget;
class QButtonGroup;
class QPlainTextEdit;
class QProcess;
class CodeEditor;
class CodeHighlighter;
class SnippetBar;
class TitleBar;
class RpgModule;
class MindMapWidget;
class TimeTravelModule;
class ScratchpadModule;
class RtxLightModule; // Forward declaration

// ─────────────────────────────────────────────────────────────
// EditorWindow — контекст «Стратегии». Владеет редактором,
// консолью и всеми модулями-режимами. Одновременно активен
// ровно один режим; переключение выполняет deactivate() старого
// и activate() нового, а QStackedWidget изолирует карту связей.
// Окно безрамочное: заголовок кастомный, ресайз — за кромку 5px.
// ─────────────────────────────────────────────────────────────
class EditorWindow : public QWidget {
    Q_OBJECT
public:
    explicit EditorWindow(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;         // антрацитовый корпус со скруглениями
    void resizeEvent(QResizeEvent*) override;       // маска скруглений
    void mousePressEvent(QMouseEvent*) override;    // ресайз безрамочного окна
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private slots:
    void runScript();
    void openFile();   // <--- НОВОЕ
    void saveFile();   // <--- НОВОЕ
    void saveAsFile();
    void setBackground(); // <--- ДОБАВЬ ЭТУ СТРОЧКУ

private:
    void buildUi();
    void setMode(int mode);

    int  resizeEdges(const QPoint& pos) const;
    void updateCursorForEdges(int edges);

    // Виджеты
    TitleBar*       m_title = nullptr;
    QStackedWidget* m_stack = nullptr;
    QWidget*        m_stage = nullptr;      // страница 0: редактор + декорации режимов
    SnippetBar*     m_snippets = nullptr;
    CodeEditor*     m_editor = nullptr;
    QPlainTextEdit* m_console = nullptr;
    QButtonGroup*   m_modeGroup = nullptr;

    // Модули-стратегии
    CodeHighlighter*   m_highlighter = nullptr;
    RpgModule*         m_rpg  = nullptr;
    MindMapWidget*     m_map  = nullptr;
    TimeTravelModule*  m_time = nullptr;
    ScratchpadModule*  m_pad  = nullptr;
    RtxLightModule* m_rtx = nullptr; // <--- ДОБАВЬ ЭТУ СТРОЧКУ
    QProcess* m_process = nullptr;
    QString   m_lastStderr;                 // копим stderr для вердикта RPG-режима
    int  m_mode = -1;                       // 0 стандарт · 1 RPG · 2 карта · 3 время · 4 заметки

    // Состояние ресайза безрамочного окна
    int   m_edges = 0;
    QRect m_resizeStart;
    QPoint m_resizeMouse;
    QString m_currentFilePath;
    QString m_bgImagePath; // <--- ДОБАВЬ ЭТУ СТРОЧКУ
};