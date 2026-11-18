#include "BackgroundManager.h"
#include "editor/CodeEditor.h"
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QDir>

BackgroundManager::BackgroundManager(CodeEditor* editor, QWidget* parent)
    : QWidget(parent), m_editor(editor) {
    setObjectName("BackgroundManager");
    buildUi();
}

void BackgroundManager::buildUi() {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(10);
    
    auto* title = new QLabel(QStringLiteral("🖼️ Фоны редактора"), this);
    title->setStyleSheet("font-size:16px;font-weight:bold;color:#22d3ee;");
    lay->addWidget(title);
    
    auto* loadBtn = new QPushButton(QStringLiteral(" Загрузить свою картинку..."), this);
    loadBtn->setCursor(Qt::PointingHandCursor);
    connect(loadBtn, &QPushButton::clicked, this, &BackgroundManager::onLoadCustom);
    lay->addWidget(loadBtn);
    
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    auto* gridW = new QWidget;
    m_grid = new QGridLayout(gridW);
    m_grid->setSpacing(10);
    
    // Предустановленные фоны (положи картинки в resources/backgrounds/)
    m_presets = {":/bg/space.jpg", ":/bg/neon.png", ":/bg/dark.jpg"};
    
    int row = 0, col = 0;
    for (int i = 0; i < m_presets.size(); ++i) {
        auto* btn = new QPushButton(gridW);
        btn->setFixedSize(120, 80);
        QPixmap px(m_presets[i]);
        if (!px.isNull()) {
            btn->setIcon(QIcon(px.scaled(110, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            btn->setIconSize(QSize(110, 70));
        } else {
            btn->setText(QString("Фон %1").arg(i+1));
        }
        connect(btn, &QPushButton::clicked, this, [this, i]{ onPresetClicked(i); });
        m_grid->addWidget(btn, row, col);
        if (++col >= 3) { col = 0; ++row; }
    }
    
    scroll->setWidget(gridW);
    lay->addWidget(scroll, 1);
}

void BackgroundManager::onPresetClicked(int idx) {
    if (idx >= 0 && idx < m_presets.size()) applyBackground(m_presets[idx]);
}

void BackgroundManager::onLoadCustom() {
    QString f = QFileDialog::getOpenFileName(this, "Выбрать фон", QDir::homePath(),
        "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!f.isEmpty()) applyBackground(f);
}

void BackgroundManager::applyBackground(const QString& path) {
    QString url = path;
    url.replace('\\', '/');
    m_editor->setStyleSheet(QString(
        "QPlainTextEdit#CodeEditor{background-image:url('%1');"
        "background-repeat:no-repeat;background-position:center;}").arg(url));
}