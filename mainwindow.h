#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QLineEdit>
#include <QLabel>
#include "models/board.h"
#include "widgets/columnwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadBoard(const QString& filePath);

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
    Ui::MainWindow *ui;
    Board board;

    QMap<TaskStatus, ColumnWidget*> columns;
    QLineEdit* searchBox;
    QLabel* statsLabel;

    QString searchFilter;

    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void refreshBoard();
    void updateStatistics();
    void showWarningIfUnassigned();
    bool matchesSearch(Task* task);
};

#endif // MAINWINDOW_H
