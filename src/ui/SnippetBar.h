#pragma once
#include <QWidget>

class CodeEditor;

// ─────────────────────────────────────────────────────────────
// Панель быстрой вставки заготовок кода. Показывается ТОЛЬКО
// в стандартном режиме — так требует архитектура изоляции.
// ─────────────────────────────────────────────────────────────
class SnippetBar : public QWidget {
    Q_OBJECT
public:
    SnippetBar(CodeEditor* editor, QWidget* parent = nullptr);

private:
    void insert(const QString& text);
    CodeEditor* m_editor;
};