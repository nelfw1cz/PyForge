⚡ PyForge
The Ultimate Dark-Mode Python Editor with Gamified Modes & Dynamic Lighting
PyForge — это экспериментальный десктопный редактор кода на C++17 и Qt 6, созданный не просто для написания скриптов, а для погружения в процесс разработки. Интерфейс выполнен в строгом стиле Anthracite/Neon (глубокий графит + неоновые акценты).
Главная фишка приложения — архитектура изолированных РЕЖИМОВ. Одновременно активен только один специальный режим, переключаемый через минималистичный Sidebar слева. Никакой "каши" из кнопок и меню — чистая фокусировка.
🛠 Features & Modes
1. Core Editor (Стандартный режим)
База, которая работает всегда.
Syntax Highlighting: Полная подсветка Python (keywords, strings, comments, decorators, numbers) через собственный QSyntaxHighlighter.
Auto-Indentation: Умные отступы. Нажатие Enter после : автоматически добавляет 4 пробела.
Line Numbers: Кастомная панель номеров строк слева.
Snippet Bar: Быстрая вставка заготовок (for, while, def, class, try/except) одним кликом.
Execution Engine: Встроенный запуск скриптов через QProcess (Python 3.x) с выводом stdout/stderr прямо в консоль редактора.
2. Code-RPG (⚡ Lightning Icon)
Геймификация процесса кодинга.
HUD System: Панель с полосками XP (Опыт) и HP (Здоровье), а также счетчик Combo Streak.
Logic:
Каждая набранная строка кода дает +XP и увеличивает комбо.
Успешный запуск скрипта (exit code 0) дает бонусный опыт и лечение (+HP).
Ошибки в коде или краш при запуске тратят -HP и сбрасывают комбо.
Level Up: При накоплении опыта происходит повышение уровня с красивым всплывающим уведомлением (Toast Notification).
3. Mind-Map Graph (🕸 Hexagon Icon)
Интерактивный анализ структуры кода.
Parser: Сканирует текущий файл на наличие определений функций (def name()) и их вызовов внутри тела других функций.
Visualization: Строит граф зависимостей на базе QGraphicsScene. Функции становятся светящимися узлами (нодами), а вызовы — направленными ребрами (Bezier curves).
Interaction: Узлы можно перетаскивать мышью (Drag-and-Drop), связи обновляются в реальном времени. Поддержка масштабирования (Ctrl + Wheel).
4. Time Travel Scrubbing (⏱ Clock Icon)
Машина времени для вашего кода.
Snapshotting: Автоматическое сохранение снимков текста в кольцевой буфер при каждом изменении (debounce 600ms) и завершении строки.
Scrubber UI: Горизонтальный слайдер над редактором, стилизованный под видеоплеер.
Rewind: Перемещение ползунка назад блокирует ввод и "перематывает" код к историческому состоянию. Редактор визуально затемняется, показывая "призрачное" прошлое. Возврат вправо восстанавливает живой текст.
5. Scratchpad Overlay (✎ Pen Icon)
Слой визуальных заметок поверх кода.
Transparent Layer: Прозрачный виджет, перехватывающий события мыши поверх редактора.
Drawing Tools: Маркер (красный) и Стрелки (желтые). Можно рисовать схемы, выделять участки кода или оставлять комментарии прямо поверх синтаксиса.
Persistence: Рисунки сохраняются между переключениями режимов.
6. RTX Dynamic Lighting (☀ Sun Icon)
Уникальная фича этого проекта. Имитация трассировки лучей в 2D пространстве.
Light Source: Появляется яркий светящийся шар (source of light), который можно свободно перетаскивать мышью по окну редактора.
Dynamic Shadows: Текст получает реалистичные тени (QGraphicsDropShadowEffect), которые динамически меняют направление, длину и размытие в зависимости от положения источника света.
Physics-based Logic: Чем ближе свет к букве — тем резче тень; чем дальше — тем мягче и длиннее смещение. Создает эффект объема и глубины ("Megatextured look").
⌨️ Hotkeys
Комбинация
Действие
Режим
F5 / Ctrl+Enter
Запустить скрипт
Любой
Ctrl+O
Открыть файл (.py)
Global
Ctrl+S
Сохранить файл
Global
Ctrl+B
Выбрать фон (Image Background)
Global
Tab
Отступ (4 пробела)
Editor
Shift+Tab
Уменьшить отступ
Editor
Ctrl+Wheel
Масштабирование графа
Mind-Map
🏗 Architecture
Проект построен по паттерну Strategy + Observer:
EditorWindow — оркестратор. Владеет общим экземпляром CodeEditor и списком модулей-стратегий.
Modes (RpgModule, MindMapWidget, etc.) — изолированные классы. Они слушают сигналы редактора (lineCommitted, textChanged), но реагируют на них только если активны (isActive == true). Это гарантирует отсутствие побочных эффектов при переключении вкладок.
UI Stack — использование QStackedWidget позволяет полностью скрывать редактор, заменяя его полноэкранным графиком (Mind-Map), или накладывать декорации (HUD, Slider, Overlay) поверх него.
Tech Stack
Language: C++17
Framework: Qt 6 (Widgets module)
Build System: CMake
Dependencies: None external (pure Qt standard library)
📦 Installation & Build
Требуется установленный Qt 6 SDK и компилятор (MinGW-w64 или MSVC).
bash
git clone https://github.com/YOUR_USERNAME/PyForge.git
cd PyForge
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc20xx_64" # Путь к вашей Qt
cmake --build . --config Release
./pyforge.exe
(Для запуска режима выполнения скриптов необходим Python 3.x в PATH).
👨‍💻 Author
Created by Timofey (@your_github_handle).
A passion project exploring gamification and dynamic lighting in desktop IDEs.
