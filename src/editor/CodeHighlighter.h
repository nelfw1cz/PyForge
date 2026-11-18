#pragma once
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>
#include <QTimer>
#include <QColor>

class CodeHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit CodeHighlighter(QTextDocument* parent);
    void setAnimationEnabled(bool enabled);

protected:
    void highlightBlock(const QString& text) override;

private:
    enum BlockState { NoState = 0, InTripleSingle = 1, InTripleDouble = 2 };

    struct Rule {
        QRegularExpression re;
        int group = 0;
        QTextCharFormat fmt;
        QColor baseColor; // Храним базовый цвет для анимации
    };
    QColor m_stringBase;
    QColor m_commentBase;
    QVector<Rule> m_rules;
    QTextCharFormat m_stringFmt;
    QTextCharFormat m_commentFmt;
    QTimer* m_pulseTimer = nullptr;
    qreal   m_phase = 0.0;
    bool    m_animating = true;
};