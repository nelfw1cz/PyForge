cmake_minimum_required(VERSION 3.16)
project(PyForge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Поддерживаются и Qt 6, и Qt 5.15
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(pyforge
    src/main.cpp
    src/theme/NeonTheme.cpp
    src/editor/CodeEditor.cpp
    src/editor/CodeHighlighter.cpp
    src/ui/TitleBar.cpp
    src/ui/SnippetBar.cpp
    src/modes/RpgModule.cpp
    src/modes/MindMapWidget.cpp
    src/modes/TimeTravelModule.cpp
    src/modes/ScratchpadModule.cpp
    src/modes/RtxLightModule.cpp
    src/EditorWindow.cpp
)

target_link_libraries(pyforge PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(pyforge PRIVATE src)