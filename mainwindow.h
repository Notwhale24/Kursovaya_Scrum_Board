#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>         // Контейнер для хранения пар ключ-значение
#include <QLineEdit>    // Виджет для однострочного текстового ввода (поиск)
#include <QLabel>       // Виджет для отображения текста (статистика)
#include "models/board.h"
#include "widgets/columnwidget.h"

QT_BEGIN_NAMESPACE // Начало пространства имен Qt (нужно для Ui класса)
namespace Ui {
class MainWindow; // Предварительное объявление класса UI (создается автоматически)
}
QT_END_NAMESPACE // Конец пространства имен Qt

class MainWindow : public QMainWindow {
    Q_OBJECT // Макрос Qt для включения механизма сигналов и слотов

public:
    // Конструктор главного окна
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();  // Деструктор для очистки ресурсов

    void loadBoard(const QString& filePath); //filePath - путь к файлу с сохраненной доской

signals:
    void backToStartScreen();

private slots:
    void onNewBoard();
    void onSaveBoard();
    void onLoadBoard();
    void onAddDeveloper();
    void onAddTask();
    void onManageDevelopers();
    void onShowStatistics();

    void onTaskEdit(Task* task);
    void onTaskDelete(Task* task);
    void onTaskStatusChange(Task* task);

    void onSearchTextChanged(const QString& text);
    void onTaskDropped(int taskId, TaskStatus newStatus);

private:
    Ui::MainWindow *ui; // Указатель на сгенерированный UI (форму)
    Board board;  // Модель данных доски (хранит задачи, разработчиков, логику)

    // Контейнер для связи статусов задач с виджетами столбцов
    // Ключ: TaskStatus (например, TODO, IN_PROGRESS, DONE)
    // Значение: ColumnWidget* - виджет, отображающий задачи данного статуса
    QMap<TaskStatus, ColumnWidget*> columns;
    QLineEdit* searchBox;
    QLabel* statsLabel;

    QString searchFilter;

    void setupUI(); // Настройка основного интерфейса
    void setupMenuBar();  // Создание и настройка меню (File, Edit, View и т.д.)
    void setupToolBar();  // Создание и настройка панели инструментов (кнопки быстрого доступа)
    void refreshBoard(); // Полное обновление отображения доски (перерисовка всех столбцов и задач)
    void updateStatistics();  // Обновление статистики (отображается в statsLabel)
    void showWarningIfUnassigned();   // Проверка наличия неприсвоенных задач и вывод предупреждения
    bool matchesSearch(Task* task);  // Проверка, соответствует ли задача текущему поисковому запросу
};

#endif // MAINWINDOW_H
