#include "CodeEditor.h"
#include "theme/NeonTheme.h"

#include <QPainter>
#include <QTextBlock>
#include <QKeyEvent>

namespace {
// Боковая панель: просто делегирует отрисовку редактору
class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEditor* ed) : QWidget(ed), m_ed(ed) {}
    QSize sizeHint() const override { return {m_ed->lineNumberAreaWidth(), 0}; }
protected:
    void paintEvent(QPaintEvent* e) override { m_ed->lineNumberAreaPaintEvent(e); }
private:
    CodeEditor* m_ed;
};
} // namespace

CodeEditor::CodeEditor(QWidget* parent) : QPlainTextEdit(parent) {
    setObjectName("CodeEditor");

    QFont f = font();
    f.setFamily("JetBrains Mono");
    f.setStyleHint(QFont::Monospace);
    f.setPointSize(11);
    setFont(f);

    m_lineArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged,   this, &CodeEditor::onBlockCountChanged);
    connect(this, &QPlainTextEdit::updateRequest,       this, &CodeEditor::onUpdateRequest);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    setLineWrapMode(QPlainTextEdit::NoWrap);
    // Ширина табуляции = 4 пробела (после setFont!)
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);

    onBlockCountChanged(0);
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() const {
    const int digits = qMax(2, QString::number(qMax(1, blockCount())).length());
    return 14 + digits * fontMetrics().horizontalAdvance('9');
}

QString CodeEditor::leadingIndent(const QString& line) {
    int i = 0;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
    return line.left(i);
}

void CodeEditor::keyPressEvent(QKeyEvent* e) {
    const bool ctrl = e->modifiers() & Qt::ControlModifier;

    // Tab → 4 пробела (упрощённо, без блочного отступа)
    if (e->key() == Qt::Key_Tab && !ctrl) {
        insertPlainText("    ");
        return;
    }
    // Shift+Tab → снять до 4 пробелов в начале строки
    if (e->key() == Qt::Key_Backtab) {
        QTextCursor c = textCursor();
        const QString line = c.block().text();
        int spaces = 0;
        while (spaces < 4 && spaces < line.size() && line[spaces] == ' ') ++spaces;
        if (spaces > 0) {
            c.movePosition(QTextCursor::StartOfBlock);
            c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, spaces);
            c.removeSelectedText();
        }
        return;
    }

    // Enter: автоотступ. Вставляем "\n + отступ" в позицию курсора —
    // текст справа от курсора автоматически «съезжает» на новую строку
    // и оказывается после вставленного отступа, курсор — перед ним.
    if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) && !ctrl) {
        QTextCursor c = textCursor();
        const QString before = c.block().text().left(c.positionInBlock());

        QString indent = leadingIndent(before);
        if (before.trimmed().endsWith(QLatin1Char(':')))
            indent += QStringLiteral("    ");   // начало блока Python

        c.insertText(QStringLiteral("\n") + indent);
        setTextCursor(c);
        emit lineCommitted();                   // строка «сдана» → режимы реагируют
        return;
    }

    QPlainTextEdit::keyPressEvent(e);
}

void CodeEditor::resizeEvent(QResizeEvent* e) {
    QPlainTextEdit::resizeEvent(e);
    const QRect cr = contentsRect();
    m_lineArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::onBlockCountChanged(int) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    m_lineArea->update();
}

void CodeEditor::onUpdateRequest(const QRect& rect, int dy) {
    if (dy) m_lineArea->scroll(0, dy);                       // скролл номеров вместе с текстом
    else    m_lineArea->update(0, rect.y(), m_lineArea->width(), rect.height());
}

void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extra;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection sel;
        sel.format.setBackground(QColor(34, 211, 238, 16));  // едва заметный неон
        sel.format.setProperty(QTextFormat::FullWidthSelection, true);
        sel.cursor = textCursor();
        sel.cursor.clearSelection();
        extra.append(sel);
    }
    setExtraSelections(extra);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* e) {
    QPainter p(m_lineArea);
    p.fillRect(e->rect(), QColor(13, 15, 20));               // чуть темнее редактора

    QTextBlock block = firstVisibleBlock();
    int top    = int(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + int(blockBoundingRect(block).height());

    p.setFont(font());
    while (block.isValid() && top <= e->rect().bottom()) {
        if (block.isVisible() && bottom >= e->rect().top()) {
            const bool current = block.blockNumber() == textCursor().blockNumber();
            p.setPen(current ? Neon::cyan() : QColor(70, 78, 94));
            p.drawText(0, top, m_lineArea->width() - 8, fontMetrics().height(),
                       Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(block.blockNumber() + 1));
        }
        block = block.next();
        top = bottom;
        bottom = top + int(blockBoundingRect(block).height());
    }
}