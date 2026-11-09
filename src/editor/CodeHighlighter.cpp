#include "CodeHighlighter.h"

#include <QTextDocument>

namespace {
QTextCharFormat makeFmt(const QColor& color, bool bold = false, bool italic = false) {
    QTextCharFormat f;
    f.setForeground(color);
    f.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    f.setFontItalic(italic);
    return f;
}
} // namespace

CodeHighlighter::CodeHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
    // Неоновая палитра синтаксиса
    const QColor kw(255, 122, 178);       // ключевые слова — розовый
    const QColor consts(130, 206, 255);   // True/False/None/self
    const QColor builtins(102, 217, 239); // встроенные функции
    const QColor fn(94, 234, 212);        // имя функции после def
    const QColor dec(255, 209, 102);      // декораторы
    const QColor num(255, 184, 108);      // числа
    const QColor str(126, 231, 135);      // строки — зелёный
    const QColor com(96, 106, 128);       // комментарии — серый

    m_stringFmt  = makeFmt(str);
    m_commentFmt = makeFmt(com, false, true);

    auto add = [&](const QString& pattern, int group, const QTextCharFormat& fmt) {
        m_rules.append({QRegularExpression(pattern, QRegularExpression::MultilineOption),
                        group, fmt});
    };

    add(QStringLiteral(R"re(\b(def|class|if|elif|else|for|while|return|import|from|as|with|try|except|finally|raise|pass|break|continue|global|nonlocal|lambda|yield|assert|del|in|is|not|and|or|async|await|match|case)\b)re"),
        0, makeFmt(kw, true));
    add(QStringLiteral(R"re(\b(True|False|None|self|cls)\b)re"), 0, makeFmt(consts));
    add(QStringLiteral(R"re(\b(print|len|range|int|float|str|list|dict|set|tuple|open|input|type|isinstance|super|enumerate|zip)\b)re"),
        0, makeFmt(builtins));
    add(QStringLiteral(R"re(^\s*def\s+(\w+))re"), 1, makeFmt(fn, true)); // только имя
    add(QStringLiteral(R"re(@[\w\.]+)re"), 0, makeFmt(dec));
    add(QStringLiteral(R"re(\b\d[\d_]*(\.\d[\d_]*)?([eE][+-]?\d+)?\b)re"), 0, makeFmt(num));
}

void CodeHighlighter::highlightBlock(const QString& text) {
    // 1) «Простые» правила (всё, кроме строк и комментариев)
    for (const Rule& r : m_rules) {
        auto it = r.re.globalMatch(text);
        while (it.hasNext()) {
            const auto m = it.next();
            setFormat(m.capturedStart(r.group), m.capturedLength(r.group), r.fmt);
        }
    }

    // 2) Строки + комментарии одним проходом слева направо.
    //    Более поздний setFormat перекрывает правила из шага 1,
    //    поэтому ключевые слова внутри строк не подсвечиваются.
    const int len = text.length();
    int pos = 0;

    // Продолжение тройной строки, открытой на предыдущей строке
    const int prev = previousBlockState();
    if (prev == InTripleSingle || prev == InTripleDouble) {
        const QString delim = (prev == InTripleSingle) ? QStringLiteral("'''")
                                                       : QStringLiteral("\"\"\"");
        const int close = text.indexOf(delim);
        if (close < 0) {                       // вся строка внутри литерала
            setFormat(0, len, m_stringFmt);
            setCurrentBlockState(prev);
            return;
        }
        setFormat(0, close + 3, m_stringFmt);
        pos = close + 3;
    }
    setCurrentBlockState(NoState);

    while (pos < len) {
        const QChar c = text.at(pos);

        if (c == '#') {                        // комментарий до конца строки
            setFormat(pos, len - pos, m_commentFmt);
            break;
        }
        // Тройные кавычки
        if (pos + 3 <= len &&
            (text.mid(pos, 3) == QLatin1String("'''") ||
             text.mid(pos, 3) == QLatin1String("\"\"\""))) {
            const QString delim = text.mid(pos, 3);
            const int close = text.indexOf(delim, pos + 3);
            if (close < 0) {                   // строка продолжается на следующих строках
                setFormat(pos, len - pos, m_stringFmt);
                setCurrentBlockState(delim == QLatin1String("'''") ? InTripleSingle
                                                                   : InTripleDouble);
                break;
            }
            setFormat(pos, close + 3 - pos, m_stringFmt);
            pos = close + 3;
            continue;
        }
        // Обычные строки '...' / "..." (с учётом экранирования)
        if (c == '"' || c == '\'') {
            int j = pos + 1;
            bool closed = false;
            while (j < len) {
                if (text.at(j) == '\\') { j += 2; continue; }
                if (text.at(j) == c)    { closed = true; break; }
                ++j;
            }
            setFormat(pos, (closed ? j + 1 : len) - pos, m_stringFmt);
            pos = closed ? j + 1 : len;
            continue;
        }
        ++pos;
    }
}