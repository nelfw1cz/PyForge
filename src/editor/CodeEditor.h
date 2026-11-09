#pragma once
#include <QPlainTextEdit>

class QPaintEvent;
class QResizeEvent;

// ─────────────────────────────────────────────────────────────
// CodeEditor — ядро на базе QPlainTextEdit:
//  • панель номеров строк слева;
//  • подсветка текущей строки;
//  • автоотступ: Enter повторяет отступ строки и добавляет
//    +4 пробела, если строка заканчивается на ':';
//  • Tab = 4 пробела, Shift+Tab снимает до 4 пробелов;
//  • сигнал lineCommitted() — «строка сдана» (потребляют
//    RpgModule — для XP, и TimeTravelModule — для снимка).
// ─────────────────────────────────────────────────────────────
class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget* parent = nullptr);

    // Служебные методы для панели номеров
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int  lineNumberAreaWidth() const;

signals:
    void lineCommitted();

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void onBlockCountChanged(int);
    void onUpdateRequest(const QRect& rect, int dy);
    void highlightCurrentLine();

private:
    static QString leadingIndent(const QString& line);
    QWidget* m_lineArea = nullptr;
};