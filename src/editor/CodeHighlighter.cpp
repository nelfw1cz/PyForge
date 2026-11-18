#include "CodeHighlighter.h"
#include <QTextDocument>
#include <QtMath>
#include <QTextCursor> 
#include <cmath>  

// Вспомогательная функция создания формата
static QTextCharFormat makeFmt(const QColor& color, bool bold = false, bool italic = false) {
    QTextCharFormat f;
    f.setForeground(color);
    if (bold) f.setFontWeight(QFont::Bold);
    if (italic) f.setFontItalic(true);
    return f;
}

// Функция "дыхания" цвета
static QColor breathe(const QColor& base, qreal phase, qreal amplitude = 0.15) {
    int h, s, v;
    base.getHsv(&h, &s, &v);
    float delta = std::sin(phase) * amplitude * v;
    int newV = qBound(0, int(v + delta), 255);
    return QColor::fromHsv(h, s, newV);
}

CodeHighlighter::CodeHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
    // Инициализация правил с БАЗОВЫМИ цветами
    auto addRule = [&](const QString& pattern, int group, const QColor& baseColor) {
        Rule r;
        r.re = QRegularExpression(pattern, QRegularExpression::MultilineOption);
        r.group = group;
        r.baseColor = baseColor; // Сохраняем оригинальный цвет
        r.fmt = makeFmt(baseColor); // Начальный формат
        m_rules.append(r);
    };

    addRule(R"re(\b(def|class|if|elif|else|for|while|return|import|from|as|with|try|except|finally|raise|pass|break|continue|global|nonlocal|lambda|yield|assert|del|in|is|not|and|or|async|await)\b)re",
            0, QColor(255, 122, 178));   // Keywords (Pink)
            
    addRule(R"re(\b(True|False|None|self|cls)\b)re", 
            0, QColor(130, 206, 255));   // Constants (Blue)
            
    addRule(R"re(\b(print|len|range|int|float|str|list|dict|set|tuple|open|input|type|isinstance|super|enumerate|zip)\b)re",
            0, QColor(102, 217, 239));   // Built-ins (Cyan)
            
    addRule(R"re(^\s*def\s+(\w+))re", 
            1, QColor(94, 234, 212));    // Function Names (Mint)
            
    addRule(R"re(@[\w\.]+)re", 
            0, QColor(255, 209, 102));   // Decorators (Yellow)
            
    addRule(R"re(\b\d[\d_]*(\.\d[\d_]*)?([eE][+-]?\d+)?\b)re", 
            0, QColor(255, 184, 108));   // Numbers (Orange)

    // Строки и комментарии хранятся отдельно
    m_stringBase = QColor(126, 231, 135); // Green
    m_commentBase = QColor(96, 106, 128); // Gray
    
    m_stringFmt = makeFmt(m_stringBase);
    m_commentFmt = makeFmt(m_commentBase, false, true);

    // Таймер просто двигает фазу
        // Таймер просто двигает фазу
    m_pulseTimer = new QTimer(this);
    connect(m_pulseTimer, &QTimer::timeout, this, [this]{
        if (!m_animating) return;

        // двигаем фазу волны
        m_phase += 0.05;
        if (m_phase > 2 * M_PI) m_phase -= 2 * M_PI;

        // 🔑 ГЛАВНОЕ: перекрашиваем САМИ форматы,
        // иначе highlightBlock применит старые статичные цвета
        for (auto& rule : m_rules) {
            rule.fmt.setForeground(breathe(rule.baseColor, m_phase));
        }
        m_stringFmt.setForeground(breathe(m_stringBase, m_phase));
        m_commentFmt.setForeground(breathe(m_commentBase, m_phase));

        // и только теперь просим перерисовать подсветку
        rehighlight();
    });
    m_pulseTimer->start(30);
}

void CodeHighlighter::setAnimationEnabled(bool enabled) {
    m_animating = enabled;
    if (enabled && !m_pulseTimer->isActive()) m_pulseTimer->start();
    else if (!enabled && m_pulseTimer->isActive()) m_pulseTimer->stop();
}

void CodeHighlighter::highlightBlock(const QString& text) {
    // Применяем правила с текущими "дышащими" цветами
    for (const Rule& rule : m_rules) {
        // Вычисляем живой цвет прямо здесь
        QColor liveColor = breathe(rule.baseColor, m_phase);
        QTextCharFormat fmt = makeFmt(liveColor);
        
        auto it = rule.re.globalMatch(text);
        while (it.hasNext()) {
            const auto match = it.next();
            setFormat(match.capturedStart(rule.group), match.capturedLength(rule.group), fmt);
        }
    }

    // Обработка строк и комментариев (логика та же, но цвета тоже дышат)
    const int len = text.length();
    int pos = 0;
    const int prev = previousBlockState();
    
    if (prev == InTripleSingle || prev == InTripleDouble) {
        const QString delim = (prev == InTripleSingle) ? "'''" : "\"\"\"";
        const int close = text.indexOf(delim);
        if (close < 0) {
            setFormat(0, len, makeFmt(breathe(m_stringBase, m_phase)));
            setCurrentBlockState(prev);
            return;
        }
        setFormat(0, close + 3, makeFmt(breathe(m_stringBase, m_phase)));
        pos = close + 3;
    }
    setCurrentBlockState(NoState);
    
    while (pos < len) {
        const QChar c = text.at(pos);
        
        if (c == '#') {
            setFormat(pos, len - pos, makeFmt(breathe(m_commentBase, m_phase), false, true));
            break;
        }
        
        if (pos + 3 <= len && (text.mid(pos, 3) == "'''" || text.mid(pos, 3) == "\"\"\"")) {
            const QString delim = text.mid(pos, 3);
            const int close = text.indexOf(delim, pos + 3);
            if (close < 0) {
                setFormat(pos, len - pos, makeFmt(breathe(m_stringBase, m_phase)));
                setCurrentBlockState(delim == "'''" ? InTripleSingle : InTripleDouble);
                break;
            }
            setFormat(pos, close + 3 - pos, makeFmt(breathe(m_stringBase, m_phase)));
            pos = close + 3;
            continue;
        }
        
        if (c == '"' || c == '\'') {
            int j = pos + 1;
            bool closed = false;
            while (j < len) {
                if (text.at(j) == '\\') { j += 2; continue; }
                if (text.at(j) == c)    { closed = true; break; }
                ++j;
            }
            setFormat(pos, (closed ? j + 1 : len) - pos, makeFmt(breathe(m_stringBase, m_phase)));
            pos = closed ? j + 1 : len;
            continue;
        }
        ++pos;
    }
}