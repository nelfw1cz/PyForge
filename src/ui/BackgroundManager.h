#pragma once
#include <QWidget>
#include <QStringList>

class QGridLayout;
class CodeEditor;

class BackgroundManager : public QWidget {
    Q_OBJECT
public:
    explicit BackgroundManager(CodeEditor* editor, QWidget* parent = nullptr);

private slots:
    void onPresetClicked(int index);
    void onLoadCustom();

private:
    void buildUi();
    void applyBackground(const QString& path);
    
    CodeEditor* m_editor;
    QGridLayout* m_grid = nullptr;
    QStringList m_presets;
};