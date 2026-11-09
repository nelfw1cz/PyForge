#include <QApplication>
#include "EditorWindow.h"
#include "theme/NeonTheme.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("PyForge");

    NeonTheme::apply();          // глобальная тема Anthracite/Neon (QSS)

    EditorWindow w;
    w.setWindowIcon(NeonTheme::icon("code"));
    w.show();
    return app.exec();
}