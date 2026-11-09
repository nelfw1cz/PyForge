#pragma once
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>

// ─────────────────────────────────────────────────────────────
// Подсветка Python: ключевые слова, числа, декораторы, имена
// функций после def, строки (включая '''многострочные''') и
// комментарии. Строки и комментарии обрабатываются одним
// проходом, поэтому '#' внутри строки не считается комментарием.
// ─────────────────────────────────────────────────────────────
class CodeHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit CodeHighlighter(QTextDocument* parent);

protected:
    void highlightBlock(const QString& text) override;

private:
    enum BlockState { NoState = 0, InTripleSingle = 1, InTripleDouble = 2 };

    struct Rule {
        QRegularExpression re;
        int group = 0;            // какая захваченная группа красится
        QTextCharFormat fmt;
    };

    QVector<Rule> m_rules;
    QTextCharFormat m_stringFmt;
    QTextCharFormat m_commentFmt;
};