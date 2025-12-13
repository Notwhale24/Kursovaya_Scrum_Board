#include "mainwindow.h"
#include "widgets/startscreen.h"

#include <QApplication> // Основной класс приложения Qt
#include <QStackedWidget> // Виджет для переключения экранов

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // Создаем объект приложения Qt

    // Создаем контейнер для переключения между экранами
    QStackedWidget* stackedWidget = new QStackedWidget();
    stackedWidget->setWindowTitle("Скрам Доска");

    // Создаем стартовый экран
    StartScreen* startScreen = new StartScreen();
    stackedWidget->addWidget(startScreen);

    // Создаем главное окно (но не показываем пока)
    MainWindow* mainWindow = nullptr;

    // Обработчик "Новая доска"
    QObject::connect(startScreen, &StartScreen::newBoardRequested, [&]() {
        if (!mainWindow) {
            mainWindow = new MainWindow(); // 1. Создаем главное окно
            stackedWidget->addWidget(mainWindow); // 2. Добавляем его в стек

            // Обработчик возврата на стартовый экран
            QObject::connect(mainWindow, &MainWindow::backToStartScreen, [&]() {
                stackedWidget->setCurrentWidget(startScreen);   // Возвращаемся на старт
            });
        }
        stackedWidget->setCurrentWidget(mainWindow); // 3. Показываем главное окно
    });

    // Обработчик "Загрузить доску"
    QObject::connect(startScreen, &StartScreen::loadBoardRequested, [&](const QString& filePath) {
        if (!mainWindow) {
            mainWindow = new MainWindow();
            stackedWidget->addWidget(mainWindow);

            // Обработчик возврата на стартовый экран
            QObject::connect(mainWindow, &MainWindow::backToStartScreen, [&]() {
                stackedWidget->setCurrentWidget(startScreen);
            });
        }
        mainWindow->loadBoard(filePath); // Загружает сохраненную доску из файла
        stackedWidget->setCurrentWidget(mainWindow);
    });

    // Обработчик "Выйти"
    QObject::connect(startScreen, &StartScreen::exitRequested, [&]() {
        a.quit(); // Завершаем приложение
    });

    stackedWidget->showMaximized(); // Показываем на весь экран

    return a.exec();  // Запускаем главный цикл обработки событий (клики мыши, нажатия клавиши, работает до закрытия окна)
}