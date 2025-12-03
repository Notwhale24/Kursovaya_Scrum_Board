#include "mainwindow.h"
#include "widgets/startscreen.h"

#include <QApplication>
#include <QStackedWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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
            mainWindow = new MainWindow();
            stackedWidget->addWidget(mainWindow);

            // Обработчик возврата на стартовый экран
            QObject::connect(mainWindow, &MainWindow::backToStartScreen, [&]() {
                stackedWidget->setCurrentWidget(startScreen);
            });
        }
        stackedWidget->setCurrentWidget(mainWindow);
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
        mainWindow->loadBoard(filePath);
        stackedWidget->setCurrentWidget(mainWindow);
    });

    // Обработчик "Выйти"
    QObject::connect(startScreen, &StartScreen::exitRequested, [&]() {
        a.quit();
    });

    stackedWidget->showMaximized(); // Показываем на весь экран

    return a.exec();
}
