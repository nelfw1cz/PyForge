#pragma once
#include <QObject>
#include <QVector>
#include <QTime>

class CodeEditor;
class QWidget;
class QSlider;
class QLabel;
class QTimer;

// ─────────────────────────────────────────────────────────────
// Режим «Машина времени». Снимки текста копятся в кольцевом
// буфере (по дебаунсу 600 мс и сразу при нажатии Enter).
// Слайдер влево = путешествие в прошлое: редактор становится
// read-only и перекрашивается; слайдер вправо = возврат в
// «сейчас». Пока мы в прошлом — новые снимки НЕ создаются.
// ─────────────────────────────────────────────────────────────
class TimeTravelModule : public QObject {
    Q_OBJECT
public:
    explicit TimeTravelModule(CodeEditor* editor, QObject* parent = nullptr);

    QWidget* bar() const { return m_bar; }
    void activate();
    void deactivate();

private slots:
    void onTextChanged();
    void onSliderMoved(int value);

private:
    void capture();            // снять текущий текст в буфер
    void returnToPresent();    // вернуть «живой» текст и разблокировать ввод
    void updateLabel();

    CodeEditor* m_editor;
    QWidget* m_bar   = nullptr;
    QSlider* m_slider = nullptr;
    QLabel*  m_info   = nullptr;
    QTimer*  m_debounce = nullptr;

    struct Snapshot { QString text; QTime time; };
    QVector<Snapshot> m_snapshots;
    QString m_liveText;        // текст «настоящего», пока мы в прошлом
    bool m_inPast = false;

    static constexpr int kMaxSnapshots = 400;
};