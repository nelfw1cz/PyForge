#include "SnippetBar.h"
#include "editor/CodeEditor.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextCursor>
#include <QTextBlock>

SnippetBar::SnippetBar(CodeEditor* editor, QWidget* parent)
    : QWidget(parent), m_editor(editor) {
    setObjectName("SnippetBar");
    setFixedHeight(40);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(12, 4, 12, 4);
    lay->setSpacing(6);

    auto* cap = new QLabel(QStringLiteral("ШАБЛОНЫ"), this);
    cap->setObjectName("DimCaption");
    lay->addWidget(cap);

    struct Snippet { const char* title; const char* code; };
    const Snippet items[] = {
        {"for",     "for i in range(10):\n    "},
        {"for…in",  "for item in items:\n    "},
        {"while",   "while condition:\n    "},
        {"def",     "def function_name(args):\n    \"\"\"Описание\"\"\"\n    "},
        {"if/else", "if condition:\n    \nelse:\n    "},
        {"try",     "try:\n    \nexcept Exception as e:\n    print(e)\n"},
        {"class",   "class MyClass:\n    def __init__(self):\n        "},
        {"with",    "with open(\"file.txt\", encoding=\"utf-8\") as f:\n    data = f.read()\n"},
        {"lambda",  "lambda x: x"},
    };

    for (const auto& s : items) {
        auto* b = new QPushButton(QString::fromUtf8(s.title), this);
        b->setObjectName("SnippetChip");
        b->setCursor(Qt::PointingHandCursor);
        const QString code = QString::fromUtf8(s.code);
        connect(b, &QPushButton::clicked, this, [this, code]{ insert(code); });
        lay->addWidget(b);
    }
    lay->addStretch(1);
}

void SnippetBar::insert(const QString& text) {
    QTextCursor c = m_editor->textCursor();
    // Если курсор стоит посреди непустой строки — начинаем шаблон с новой строки
    const QString before = c.block().text().left(c.positionInBlock());
    if (c.positionInBlock() > 0 && !before.trimmed().isEmpty())
        c.insertText("\n");
    c.insertText(text);
    m_editor->setTextCursor(c);
    m_editor->setFocus();
}