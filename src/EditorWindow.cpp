#include "EditorWindow.h"
#include "theme/NeonTheme.h"
#include "editor/CodeEditor.h"
#include "editor/CodeHighlighter.h"
#include "ui/TitleBar.h"
#include "ui/SnippetBar.h"
#include "modes/RpgModule.h"
#include "modes/MindMapWidget.h"
#include "modes/TimeTravelModule.h"
#include "modes/ScratchpadModule.h"
#include "modes/RtxLightModule.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QProcess>
#include <QShortcut>
#include <QTemporaryDir>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QStyle>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QStatusBar> // Для показа сообщений о сохранении

// Демо-код: на нём сразу видно подсветку, автоотступ и граф вызовов
static const char *kDefaultCode = R"PY(# PyForge — демо-скрипт.
# Переключайте режимы на панели слева: RPG, карта связей, машина времени, заметки.

import random


def greet(name):
    """Приветствие игрока."""
    print(f"Привет, {name}!")


def roll_dice():
    return random.randint(1, 6)


def game_round(name):
    greet(name)
    score = roll_dice() + roll_dice()
    print("Очки за раунд:", score)
    return score


if __name__ == "__main__":
    game_round("Гость")
)PY";

EditorWindow::EditorWindow(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground); // чтобы скругления были честными
    setMouseTracking(true);                     // для смены курсора на кромках
    setMinimumSize(1000, 640);
    resize(1280, 820);

    buildUi();

    // Горячие клавиши запуска
    auto *sc1 = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(sc1, &QShortcut::activated, this, &EditorWindow::runScript);
    auto *sc2 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this);
    connect(sc2, &QShortcut::activated, this, &EditorWindow::runScript);
    auto *scOpen = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this);
    connect(scOpen, &QShortcut::activated, this, &EditorWindow::openFile);
    auto *scSave = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this);
    connect(scSave, &QShortcut::activated, this, &EditorWindow::saveFile);
    auto *scBg = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), this);
    connect(scBg, &QShortcut::activated, this, &EditorWindow::setBackground);
    setMode(0); // стандартный режим по умолчанию
}

// ─────────────────────────── Сборка UI ───────────────────────────

void EditorWindow::buildUi()
{
    // Корневой макет: отступ 5px по периметру — это и есть зоны захвата ресайза
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(5, 5, 5, 5);
    root->setSpacing(0);

    m_title = new TitleBar(this);
    connect(m_title, &TitleBar::runClicked, this, &EditorWindow::runScript);
    root->addWidget(m_title);

    auto *body = new QWidget(this);
    auto *bodyLay = new QHBoxLayout(body);
    bodyLay->setContentsMargins(0, 0, 0, 0);
    bodyLay->setSpacing(0);
    root->addWidget(body, 1);

    // ── Sidebar: узкая колонка с иконками режимов ──
    auto *side = new QWidget(body);
    side->setObjectName("Sidebar");
    side->setFixedWidth(58);
    auto *sideLay = new QVBoxLayout(side);
    sideLay->setContentsMargins(0, 12, 0, 12);
    sideLay->setSpacing(6);

    m_modeGroup = new QButtonGroup(this);
    const struct { const char* tip; const char* icon; } defs[] = {
        {"Стандартный режим...", "code"},
        {"Code-RPG...", "rpg"},
        {"Карта связей...", "graph"},
        {"Машина времени...", "clock"},
        {"Заметки...", "pen"},
        // === ДОБАВЬ ЭТУ СТРОЧКУ НИЖЕ ===
        {"RTX Освещение: перетаскивай свет!", "sun"} 
    };
    for (int i = 0; i < 6; ++i) { // Было 5, стало 6!
        auto* b = new QPushButton(side);
        b->setObjectName("SideBtn");
        b->setCheckable(true);
        b->setFixedSize(42, 42);
        b->setIcon(NeonTheme::icon(defs[i].icon));
        b->setIconSize(QSize(22, 22));
        b->setToolTip(QString::fromUtf8(defs[i].tip));
        b->setCursor(Qt::PointingHandCursor);
        sideLay->addWidget(b, 0, Qt::AlignHCenter);
        m_modeGroup->addButton(b, i);
    }
    connect(m_modeGroup, &QButtonGroup::idClicked, this, &EditorWindow::setMode);
    sideLay->addStretch(1);
    bodyLay->addWidget(side);

    // ── Контент: стек страниц (редактор | карта связей) ──
    m_stack = new QStackedWidget(body);
    bodyLay->addWidget(m_stack, 1);

    // Страница 0 — сцена редактора. Вертикальный стек:
    // [HUD?] → [шаблоны?] → [панель времени?] → редактор → консоль
    m_stage = new QWidget;
    auto *stageLay = new QVBoxLayout(m_stage);
    stageLay->setContentsMargins(12, 10, 12, 10);
    stageLay->setSpacing(8);

    m_editor = new CodeEditor(m_stage);
    m_highlighter = new CodeHighlighter(m_editor->document());
    m_editor->setPlainText(QString::fromUtf8(kDefaultCode));

    // Модули создают свои виджеты сами — окно лишь размещает и дёргает их
    m_rpg = new RpgModule(m_editor, m_stage, this);
    m_time = new TimeTravelModule(m_editor, this);
    m_pad = new ScratchpadModule(m_editor, this);
    m_snippets = new SnippetBar(m_editor, m_stage);
    m_rtx = new RtxLightModule(m_editor, m_stage, this);
    stageLay->addWidget(m_rpg->hud());
    stageLay->addWidget(m_snippets);
    stageLay->addWidget(m_time->bar());
    stageLay->addWidget(m_editor, 1);

    m_console = new QPlainTextEdit(m_stage);
    m_console->setObjectName("Console");
    m_console->setReadOnly(true);
    m_console->setFixedHeight(110);
    m_console->setPlainText(QStringLiteral(
        "▸ Вывод скрипта появится здесь. Запуск — кнопка «Запустить» или F5."));
    stageLay->addWidget(m_console);

    m_stack->addWidget(m_stage);

    // Страница 1 — карта связей (полностью заменяет редактор)
    m_map = new MindMapWidget;
    m_stack->addWidget(m_map);

    // Стартовое состояние: всё «режимное» скрыто
    m_rpg->hud()->hide();
    m_time->bar()->hide();
}

// ─────────────────── Переключение режимов ───────────────────

void EditorWindow::setMode(int mode) {
    if (mode < 0 || mode == m_mode) return;

    // 1) Деактивация предыдущего режима
    switch (m_mode) {
    case 1: if (m_rpg)  m_rpg->deactivate();  break;
    case 3: if (m_time) m_time->deactivate(); break;
    case 4: if (m_pad)  m_pad->deactivate();  break;
    case 5: if (m_rtx)  m_rtx->deactivate();  break; // <-- Безопасно
    default: break;
    }

    m_mode = mode;

    // 2) Общие правила изоляции
    m_stack->setCurrentIndex(mode == 2 ? 1 : 0);
    m_snippets->setVisible(mode == 0);
    m_editor->setProperty("rpgMode", mode == 1);
    m_editor->style()->polish(m_editor);

    // 3) Активация нового режима
    switch (mode) {
    case 1: if (m_rpg)  m_rpg->activate();  break;
    case 2: if (m_map)  m_map->rebuildFromCode(m_editor->toPlainText()); break;
    case 3: if (m_time) m_time->activate(); break;
    case 4: if (m_pad)  m_pad->activate();  break;
    case 5: if (m_rtx)  m_rtx->activate();  break; // <-- Безопасно
    default: break;
    }

    static const QString names[] = {
        QStringLiteral("Стандарт"), QStringLiteral("Code-RPG"),
        QStringLiteral("Карта связей"), QStringLiteral("Машина времени"),
        QStringLiteral("Заметки"), QStringLiteral("RTX LIGHT")
    };
    m_title->setModeName(names[mode]);
    if (mode != 2) m_editor->setFocus();
}

// ─────────────────── Запуск скрипта (QProcess) ───────────────────

void EditorWindow::runScript()
{
    // Повторный клик во время выполнения = аварийная остановка
    if (m_process && m_process->state() != QProcess::NotRunning)
    {
        m_process->kill();
        return;
    }

    // Сохраняем код во временный файл (директория живёт, пока живо окно)
    static QTemporaryDir dir;
    const QString path = dir.path() + "/main.py";
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        m_console->setPlainText(QStringLiteral("▸ Ошибка: не удалось создать временный файл."));
        return;
    }
    f.write(m_editor->toPlainText().toUtf8());
    f.close();

    m_console->clear();
    m_console->appendPlainText(QStringLiteral("▸ Запуск %1 …").arg(path));
    m_lastStderr.clear();
    m_title->setBusy(true);

    m_process = new QProcess(this);
    const QString python =
#ifdef Q_OS_WIN
        QStringLiteral("python");
#else
        QStringLiteral("python3");
#endif

    // Стримим вывод по мере поступления
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]
            { m_console->appendPlainText(QString::fromUtf8(m_process->readAllStandardOutput())); });
    connect(m_process, &QProcess::readyReadStandardError, this, [this]
            {
        const QString err = QString::fromUtf8(m_process->readAllStandardError());
        m_lastStderr += err;
        m_console->appendHtml(QStringLiteral("<span style=\"color:#ff5c5c;\">%1</span>")
                                  .arg(QString(err).toHtmlEscaped().replace('\n', "<br>"))); });
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError err)
            {
        if (err == QProcess::FailedToStart)
            m_console->appendPlainText(QStringLiteral(
                "▸ Интерпретатор не найден. Установите python3 и добавьте его в PATH."));
        m_title->setBusy(false); });
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus)
            {
        // Критерий успеха по ТЗ: код 0 И пустой stderr
        const bool success = (code == 0) && m_lastStderr.trimmed().isEmpty();
        m_console->appendPlainText(success
            ? QStringLiteral("\n▸ Готово (код 0). Отличная работа!")
            : QStringLiteral("\n▸ Процесс завершён с кодом %1%2.")
                  .arg(code)
                  .arg(m_lastStderr.trimmed().isEmpty() ? QString() : QString(" (есть stderr)")));
        m_title->setBusy(false);
        m_rpg->onRunFinished(success);   // RPG реагируется только если режим активен
        m_process->deleteLater();
        m_process = nullptr; });

    m_process->setWorkingDirectory(dir.path());
    m_process->start(python, {path});
}

// ─────────────── Внешний вид и ресайз безрамочного окна ───────────────

void EditorWindow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Neon::line(), 1));
    p.setBrush(Neon::bg0());
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 14, 14);
}

void EditorWindow::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    // Маска обрезает дочерние виджеты по скруглению (иначе углы были бы квадратными)
    if (!isMaximized())
    {
        QPainterPath path;
        path.addRoundedRect(rect(), 14, 14);
        setMask(QRegion(path.toFillPolygon().toPolygon()));
    }
    else
    {
        clearMask();
    }
}

int EditorWindow::resizeEdges(const QPoint &pos) const
{
    if (isMaximized())
        return 0;
    const int m = 7; // толщина зоны захвата
    int e = 0;
    if (pos.x() < m)
        e |= 1; // лево
    if (pos.x() > width() - m)
        e |= 2; // право
    if (pos.y() < m)
        e |= 4; // верх
    if (pos.y() > height() - m)
        e |= 8; // низ
    return e;
}

void EditorWindow::updateCursorForEdges(int e)
{
    if (e == (1 | 4) || e == (2 | 8))
        setCursor(Qt::SizeFDiagCursor);
    else if (e == (2 | 4) || e == (1 | 8))
        setCursor(Qt::SizeBDiagCursor);
    else if (e == 1 || e == 2)
        setCursor(Qt::SizeHorCursor);
    else if (e == 4 || e == 8)
        setCursor(Qt::SizeVerCursor);
    else
        unsetCursor();
}

void EditorWindow::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        m_edges = resizeEdges(e->position().toPoint());
        if (m_edges)
        {
            m_resizeStart = geometry();
            m_resizeMouse = e->globalPosition().toPoint();
            e->accept();
            return;
        }
    }
    QWidget::mousePressEvent(e);
}

void EditorWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (m_edges && (e->buttons() & Qt::LeftButton))
    {
        const QPoint d = e->globalPosition().toPoint() - m_resizeMouse;
        QRect g = m_resizeStart;
        if (m_edges & 1)
            g.setLeft(qMin(g.left() + d.x(), g.right() - minimumWidth()));
        if (m_edges & 2)
            g.setRight(g.right() + d.x());
        if (m_edges & 4)
            g.setTop(qMin(g.top() + d.y(), g.bottom() - minimumHeight()));
        if (m_edges & 8)
            g.setBottom(g.bottom() + d.y());
        if (g.width() >= minimumWidth() && g.height() >= minimumHeight())
            setGeometry(g);
        e->accept();
        return;
    }
    updateCursorForEdges(resizeEdges(e->position().toPoint()));
    QWidget::mouseMoveEvent(e);
}

void EditorWindow::mouseReleaseEvent(QMouseEvent *e)
{
    m_edges = 0;
    QWidget::mouseReleaseEvent(e);
}

void EditorWindow::openFile()
{
    // Диалог выбора файла (.py)
    QString fileName = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Открыть Python-файл"),
        QDir::homePath(),
        QStringLiteral("Python Files (*.py);;All Files (*)"));

    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            m_editor->setPlainText(in.readAll()); // Загружаем содержимое в редактор

            m_currentFilePath = fileName; // Запоминаем путь

            // Обновляем заголовок окна, чтобы было видно имя файла
            setWindowTitle(QStringLiteral("PyForge — %1").arg(QFileInfo(fileName).fileName()));

            file.close();
        }
        else
        {
            QMessageBox::warning(this, "Ошибка", "Не удалось прочитать файл.");
        }
    }
}

void EditorWindow::saveFile()
{
    // Если у нас еще нет пути (новый документ) — спрашиваем, куда сохранить
    if (m_currentFilePath.isEmpty())
    {
        saveAsFile();
        return;
    }

    QFile file(m_currentFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << m_editor->toPlainText(); // Пишем текст из редактора на диск
        file.close();

        // Можно показать тихое уведомление или просто обновить статус
        QMessageBox::information(this, "Сохранение", QStringLiteral("Файл сохранен:\n%1").arg(m_currentFilePath));
    }
    else
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось записать файл.");
    }
}

// Вспомогательная функция "Сохранить как..."
void EditorWindow::saveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Сохранить как..."),
        QDir::homePath(),
        QStringLiteral("Python Files (*.py)"));

    if (!fileName.isEmpty())
    {
        // Гарантируем расширение .py, если пользователь забыл его ввести
        if (!fileName.endsWith(".py"))
            fileName += ".py";

        m_currentFilePath = fileName;
        saveFile(); // Вызываем обычное сохранение теперь, когда путь известен
    }
}

void EditorWindow::setBackground()
{
    // 1. Открываем диалог выбора картинки
    QString fileName = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Выбрать фоновое изображение"),
        QDir::homePath(),
        QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp)"));

    if (fileName.isEmpty())
        return; // Если пользователь отменил — ничего не делаем

    // 2. Проверяем, что картинка реально существует и читается
    QPixmap pixmap(fileName);
    if (pixmap.isNull())
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить выбранное изображение.");
        return;
    }

    m_bgImagePath = fileName; // Запоминаем путь

    // 3. Настраиваем прозрачность фона редактора, чтобы картинка просвечивала
    // Делаем фон CodeEditor полупрозрачным черным (альфа-канал 180 из 255)
    m_editor->setStyleSheet(QStringLiteral(
        "#CodeEditor { background-color: rgba(16, 19, 26, 180); }"));

    // 4. Применяем картинку как фон для всей страницы Stage (где лежит редактор)
    // Используем свойство border-image, которое растянет картинку на весь блок
    QString bgUrl = fileName.replace('\\', '/'); // Windows любит обратные слеши, CSS их не понимает

    m_stage->setStyleSheet(QStringLiteral(
                               "QWidget#Stage {"
                               "   background-image: url('%1');"
                               "   background-repeat: no-repeat;"
                               "   background-position: center;"
                               "   background-size: cover;" // Растягиваем картинку, сохраняя пропорции
                               "}")
                               .arg(bgUrl));

    // Принудительно обновляем внешний вид (polish нужен при динамической смене стилей)
    m_stage->style()->unpolish(m_stage);
    m_stage->style()->polish(m_stage);
}