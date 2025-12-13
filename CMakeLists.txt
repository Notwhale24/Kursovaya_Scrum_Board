cmake_minimum_required(VERSION 3.16)

project(scrum_board VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)     # Автоматическая обработка .ui файлов
set(CMAKE_AUTOMOC ON)     # Автоматический moc для Qt
set(CMAKE_AUTORCC ON)     # Автоматическая компиляция ресурсов

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опция для сборки тестов
option(BUILD_TESTS "Build tests" ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Исходники основного приложения
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    models/task.h
    models/task.cpp
    models/developer.h
    models/developer.cpp
    models/board.h
    models/board.cpp
    widgets/taskcard.h
    widgets/taskcard.cpp
    widgets/columnwidget.h
    widgets/columnwidget.cpp
    widgets/startscreen.h
    widgets/startscreen.cpp
)

if(APPLE) # Добавляет фреймворк OpenGL для macOS
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-framework,OpenGL")
endif()

# Основное приложение
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6) # Для Qt6
    qt_add_executable(scrum_board      
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(scrum_board SHARED ${PROJECT_SOURCES})
    else()
        add_executable(scrum_board ${PROJECT_SOURCES})
    endif()
endif()

target_link_libraries(scrum_board PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)


# Настройки для macOS и Windows
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.scrum_board)
endif()
set_target_properties(scrum_board PROPERTIES 
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION} #Для macOS: настройки бандла (иконка, версия, идентификатор)
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE #Для Windows: создаёт .exe файл
)

# Установка
include(GNUInstallDirs)
install(TARGETS scrum_board
    BUNDLE DESTINATION . # macOS bundle
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # исполняемые файлы
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(scrum_board)
endif()

# Тесты
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)

    set(TEST_SOURCES
        tests/test_task.cpp
        tests/test_developer.cpp
        tests/test_board.cpp
        tests/test_history.cpp
        models/task.cpp
        models/developer.cpp
        models/board.cpp
    )

    add_executable(scrum_board_tests ${TEST_SOURCES})

    target_link_libraries(scrum_board_tests
        PRIVATE
        GTest::GTest
        GTest::Main
        Qt${QT_VERSION_MAJOR}::Widgets
    )


    add_test(NAME scrum_board_tests COMMAND scrum_board_tests)
endif()

# Makefile будет автоматически сгенерирован CMake
# Для сборки: mkdir build && cd build && cmake .. && make
# Для тестов: make test или ./scrum_board_tests
