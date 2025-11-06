#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "widgets/taskcard.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QListWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QCheckBox>
#include <QDebug>
#include <QTextCursor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setupUI();
    setupMenuBar();
    setupToolBar();
    setWindowTitle("Скрам Доска");
    // resize(1400, 800); // Убрали - окно будет масштабироваться на весь экран
    updateStatistics();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Устанавливаем современный градиентный фон (как на стартовом экране)
    centralWidget->setStyleSheet(
        "QWidget {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "       stop:0 #E3F2FD, "
        "       stop:0.5 #F5F5F5, "
        "       stop:1 #FFF8E1);"
        "}"
    );

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    columns[TaskStatus::Backlog] = new ColumnWidget("БЭКЛОГ", "#FFE082", TaskStatus::Backlog, this);
    columns[TaskStatus::Assigned] = new ColumnWidget("НАДО", "#90CAF9", TaskStatus::Assigned, this);
    columns[TaskStatus::InProgress] = new ColumnWidget("ДЕЛАТЬ", "#80CBC4", TaskStatus::InProgress, this);
    columns[TaskStatus::Review] = new ColumnWidget("ПРОВЕРКА", "#CE93D8", TaskStatus::Review, this);
    columns[TaskStatus::Done] = new ColumnWidget("СДЕЛАНО!", "#A5D6A7", TaskStatus::Done, this);

    for (ColumnWidget* column : columns) {
        connect(column, &ColumnWidget::taskDropped, this, &MainWindow::onTaskDropped);
    }

    mainLayout->addWidget(columns[TaskStatus::Backlog]);
    mainLayout->addWidget(columns[TaskStatus::Assigned]);
    mainLayout->addWidget(columns[TaskStatus::InProgress]);
    mainLayout->addWidget(columns[TaskStatus::Review]);
    mainLayout->addWidget(columns[TaskStatus::Done]);
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu* boardMenu = menuBar->addMenu("Доска");

    QAction* newBoardAction = boardMenu->addAction("Новая доска");
    newBoardAction->setShortcut(QKeySequence("Ctrl+N"));
    connect(newBoardAction, &QAction::triggered, this, &MainWindow::onNewBoard);

    QAction* saveBoardAction = boardMenu->addAction("Сохранить");
    saveBoardAction->setShortcut(QKeySequence("Ctrl+S"));
    connect(saveBoardAction, &QAction::triggered, this, &MainWindow::onSaveBoard);

    QAction* loadBoardAction = boardMenu->addAction("Загрузить");
    loadBoardAction->setShortcut(QKeySequence("Ctrl+O"));
    connect(loadBoardAction, &QAction::triggered, this, &MainWindow::onLoadBoard);

    boardMenu->addSeparator();

    QAction* statsAction = boardMenu->addAction("Статистика");
    statsAction->setShortcut(QKeySequence("Ctrl+I"));
    connect(statsAction, &QAction::triggered, this, &MainWindow::onShowStatistics);

    boardMenu->addSeparator();

    QAction* exitAction = boardMenu->addAction("Выход");
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(exitAction, &QAction::triggered, this, &MainWindow::backToStartScreen);

    QMenu* devMenu = menuBar->addMenu("Разработчики");

    QAction* addDevAction = devMenu->addAction("Добавить разработчика");
    addDevAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(addDevAction, &QAction::triggered, this, &MainWindow::onAddDeveloper);

    QAction* manageDevAction = devMenu->addAction("Управление разработчиками");
    manageDevAction->setShortcut(QKeySequence("Ctrl+M"));
    connect(manageDevAction, &QAction::triggered, this, &MainWindow::onManageDevelopers);

    QMenu* taskMenu = menuBar->addMenu("Задачи");

    QAction* addTaskAction = taskMenu->addAction("Добавить задачу");
    addTaskAction->setShortcut(QKeySequence("Ctrl+T"));
    connect(addTaskAction, &QAction::triggered, this, &MainWindow::onAddTask);
}

void MainWindow::setupToolBar() {
    QToolBar* toolBar = new QToolBar("Панель инструментов", this);
    toolBar->setMovable(false);
    addToolBar(Qt::TopToolBarArea, toolBar);

    QLabel* searchLabel = new QLabel(" 🔍 Поиск: ", this);
    toolBar->addWidget(searchLabel);

    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Введите название задачи...");
    searchBox->setMinimumWidth(200);
    connect(searchBox, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    toolBar->addWidget(searchBox);

    toolBar->addSeparator();

    statsLabel = new QLabel(this);
    statsLabel->setStyleSheet("padding: 5px; font-weight: bold;");
    toolBar->addWidget(statsLabel);
}

void MainWindow::refreshBoard() {
    qDebug() << "refreshBoard вызван";

    for (ColumnWidget* column : columns) {
        column->clearTasks();
    }

    for (Task& task : board.getTasks()) {
        qDebug() << "Обрабатываем задачу:" << task.getTitle();
        qDebug() << "  Дедлайн есть:" << task.hasDeadline();
        if (task.hasDeadline()) {
            qDebug() << "  Дней до дедлайна:" << task.daysUntilDeadline();
        }

        if (!matchesSearch(&task)) {
            continue;
        }

        TaskCard* card = new TaskCard(&task, &board, this);

        connect(card, &TaskCard::editClicked, this, &MainWindow::onTaskEdit);
        connect(card, &TaskCard::deleteClicked, this, &MainWindow::onTaskDelete);
        connect(card, &TaskCard::statusChangeClicked, this, &MainWindow::onTaskStatusChange);

        columns[task.getStatus()]->addTaskCard(card);
    }

    showWarningIfUnassigned();
    updateStatistics();
}

bool MainWindow::matchesSearch(Task* task) {
    if (searchFilter.isEmpty()) {
        return true;
    }

    return task->getTitle().toLower().contains(searchFilter.toLower());
}

void MainWindow::onSearchTextChanged(const QString& text) {
    searchFilter = text;
    refreshBoard();
}

void MainWindow::updateStatistics() {
    int backlog = board.getTasksByStatus(TaskStatus::Backlog).size();
    int assigned = board.getTasksByStatus(TaskStatus::Assigned).size();
    int inProgress = board.getTasksByStatus(TaskStatus::InProgress).size();
    int review = board.getTasksByStatus(TaskStatus::Review).size();
    int done = board.getTasksByStatus(TaskStatus::Done).size();
    int total = board.getTasks().size();

    int percentDone = total > 0 ? (done * 100 / total) : 0;

    QString stats = QString("📊 Всего: %1 | Выполнено: %2 (%3%)")
                        .arg(total)
                        .arg(done)
                        .arg(percentDone);

    statsLabel->setText(stats);
}

void MainWindow::showWarningIfUnassigned() {
    if (board.hasUnassignedTasks()) {
        statusBar()->showMessage("⚠️ Внимание: есть неназначенные задачи!");
        statusBar()->setStyleSheet("QStatusBar { background-color: #ffcccc; color: #cc0000; font-weight: bold; }");
    } else {
        statusBar()->clearMessage();
        statusBar()->setStyleSheet("");
    }
}

void MainWindow::onNewBoard() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Новая доска",
        "Создать новую доску? Все несохраненные данные будут потеряны.",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        board.clear();
        searchBox->clear();
        refreshBoard();
        QMessageBox::information(this, "Успех", "Новая доска создана");
    }
}

void MainWindow::onSaveBoard() {
    QString filename = QFileDialog::getSaveFileName(
        this, "Сохранить доску", "", "JSON Files (*.json)"
        );

    if (!filename.isEmpty()) {
        if (board.saveToFile(filename)) {
            QMessageBox::information(this, "Успех", "Доска успешно сохранена");
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось сохранить доску");
        }
    }
}

void MainWindow::onLoadBoard() {
    QString filename = QFileDialog::getOpenFileName(
        this, "Загрузить доску", "", "JSON Files (*.json)"
        );

    if (!filename.isEmpty()) {
        loadBoard(filename);
    }
}

void MainWindow::loadBoard(const QString& filePath) {
    if (board.loadFromFile(filePath)) {
        searchBox->clear();
        refreshBoard();
        QMessageBox::information(this, "Успех", "Доска успешно загружена");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить доску");
    }
}

void MainWindow::onAddDeveloper() {
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить разработчика");

    QFormLayout* layout = new QFormLayout(&dialog);

    QLineEdit* nameEdit = new QLineEdit(&dialog);
    QLineEdit* positionEdit = new QLineEdit(&dialog);

    layout->addRow("Имя:", nameEdit);
    layout->addRow("Должность:", positionEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog
        );
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        QString position = positionEdit->text().trimmed();

        if (name.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Имя не может быть пустым");
            return;
        }

        Developer dev(name, position);
        board.addDeveloper(dev);
        QMessageBox::information(this, "Успех", "Разработчик добавлен");
    }
}

void MainWindow::onManageDevelopers() {
    QDialog dialog(this);
    dialog.setWindowTitle("Управление разработчиками");
    dialog.resize(400, 300);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QListWidget* devList = new QListWidget(&dialog);

    for (const Developer& dev : board.getDevelopers()) {
        QString text = QString("%1 - %2 (ID: %3)")
        .arg(dev.getName())
            .arg(dev.getPosition())
            .arg(dev.getId());
        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, dev.getId());
        devList->addItem(item);
    }

    layout->addWidget(devList);

    QPushButton* deleteBtn = new QPushButton("Удалить выбранного", &dialog);
    layout->addWidget(deleteBtn);

    connect(deleteBtn, &QPushButton::clicked, [this, devList, &dialog]() {
        QListWidgetItem* item = devList->currentItem();
        if (item) {
            int devId = item->data(Qt::UserRole).toInt();

            QMessageBox::StandardButton reply = QMessageBox::question(
                this, "Удаление",
                "Удалить разработчика? Все его задачи будут сняты с него.",
                QMessageBox::Yes | QMessageBox::No
                );

            if (reply == QMessageBox::Yes) {
                board.removeDeveloper(devId);
                refreshBoard();
                dialog.accept();
            }
        }
    });

    QPushButton* closeBtn = new QPushButton("Закрыть", &dialog);
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}

void MainWindow::onTaskDropped(int taskId, TaskStatus newStatus) {
    Task* task = board.getTask(taskId);
    if (task && task->getStatus() != newStatus) {
        task->setStatus(newStatus);
        refreshBoard();
    }
}

void MainWindow::onAddTask() {
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить задачу");

    QFormLayout* layout = new QFormLayout(&dialog);

    QLineEdit* titleEdit = new QLineEdit(&dialog);

    QTextEdit* descEdit = new QTextEdit(&dialog);
    descEdit->setMaximumHeight(100);
    descEdit->setPlaceholderText("Максимум 120 символов");

    // Ограничение на 120 символов для описания
    connect(descEdit, &QTextEdit::textChanged, [descEdit]() {
        if (descEdit->toPlainText().length() > 120) {
            descEdit->setPlainText(descEdit->toPlainText().left(120));
            QTextCursor cursor = descEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            descEdit->setTextCursor(cursor);
        }
    });

    // Дедлайн
    QDateEdit* deadlineEdit = new QDateEdit(&dialog);
    deadlineEdit->setCalendarPopup(true);
    deadlineEdit->setDisplayFormat("dd.MM.yyyy");
    deadlineEdit->setMinimumDate(QDate::currentDate());
    deadlineEdit->setDate(QDate::currentDate().addDays(7));

    QCheckBox* hasDeadlineCheck = new QCheckBox("Установить дедлайн", &dialog);
    hasDeadlineCheck->setChecked(false);
    deadlineEdit->setEnabled(false);

    connect(hasDeadlineCheck, &QCheckBox::toggled, deadlineEdit, &QDateEdit::setEnabled);

    layout->addRow("Название:", titleEdit);
    layout->addRow("Описание:", descEdit);
    layout->addRow(hasDeadlineCheck);
    layout->addRow("Дедлайн:", deadlineEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog
        );
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString title = titleEdit->text().trimmed();
        QString desc = descEdit->toPlainText().trimmed();

        if (title.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Название не может быть пустым");
            return;
        }

        Task task(title, desc);

        // Устанавливаем дедлайн если выбран
        if (hasDeadlineCheck->isChecked()) {
            QDateTime deadline(deadlineEdit->date(), QTime(23, 59, 59));
            task.setDeadline(deadline);
        }

        board.addTask(task);
        refreshBoard();
        QMessageBox::information(this, "Успех", "Задача добавлена в бэклог");
    }
}

void MainWindow::onTaskEdit(Task* task) {
    if (!task) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Редактировать задачу");

    QFormLayout* layout = new QFormLayout(&dialog);

    QLineEdit* titleEdit = new QLineEdit(task->getTitle(), &dialog);

    QTextEdit* descEdit = new QTextEdit(&dialog);
    descEdit->setPlainText(task->getDescription());
    descEdit->setMaximumHeight(100);

    // Ограничение на 120 символов для описания
    connect(descEdit, &QTextEdit::textChanged, [descEdit]() {
        if (descEdit->toPlainText().length() > 120) {
            descEdit->setPlainText(descEdit->toPlainText().left(120));
            QTextCursor cursor = descEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            descEdit->setTextCursor(cursor);
        }
    });

    QComboBox* devCombo = new QComboBox(&dialog);
    devCombo->addItem("Не назначена", -1);

    for (const Developer& dev : board.getDevelopers()) {
        devCombo->addItem(
            QString("%1 (%2)").arg(dev.getName()).arg(dev.getPosition()),
            dev.getId()
            );
    }

    if (task->isAssigned()) {
        for (int i = 0; i < devCombo->count(); i++) {
            if (devCombo->itemData(i).toInt() == task->getAssignedDeveloperId()) {
                devCombo->setCurrentIndex(i);
                break;
            }
        }
    }

    QDateEdit* deadlineEdit = new QDateEdit(&dialog);
    deadlineEdit->setCalendarPopup(true);
    deadlineEdit->setDisplayFormat("dd.MM.yyyy");
    deadlineEdit->setMinimumDate(QDate::currentDate());

    if (task->hasDeadline()) {
        deadlineEdit->setDate(task->getDeadline().date());
    } else {
        deadlineEdit->setDate(QDate::currentDate().addDays(7));
    }

    QCheckBox* hasDeadlineCheck = new QCheckBox("Установить дедлайн", &dialog);
    hasDeadlineCheck->setChecked(task->hasDeadline());
    deadlineEdit->setEnabled(task->hasDeadline());

    connect(hasDeadlineCheck, &QCheckBox::toggled, deadlineEdit, &QDateEdit::setEnabled);

    layout->addRow("Название:", titleEdit);
    layout->addRow("Описание:", descEdit);
    layout->addRow("Разработчик:", devCombo);
    layout->addRow(hasDeadlineCheck);
    layout->addRow("Дедлайн:", deadlineEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog
        );
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        task->setTitle(titleEdit->text().trimmed());
        task->setDescription(descEdit->toPlainText().trimmed());

        int devId = devCombo->currentData().toInt();
        if (devId == -1) {
            task->unassign();
        } else {
            task->assignToDeveloper(devId);
        }

        if (hasDeadlineCheck->isChecked()) {
            QDateTime deadline(deadlineEdit->date(), QTime(23, 59, 59));
            task->setDeadline(deadline);
        } else {
            task->setDeadline(QDateTime());
        }

        refreshBoard();
    }
}

void MainWindow::onTaskDelete(Task* task) {
    if (!task) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Удаление задачи",
        QString("Удалить задачу '%1'?").arg(task->getTitle()),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        board.removeTask(task->getId());
        refreshBoard();
    }
}

void MainWindow::onTaskStatusChange(Task* task) {
    if (!task) return;

    TaskStatus currentStatus = task->getStatus();

    switch (currentStatus) {
    case TaskStatus::Backlog:
        task->setStatus(TaskStatus::Assigned);
        break;
    case TaskStatus::Assigned:
        task->setStatus(TaskStatus::InProgress);
        break;
    case TaskStatus::InProgress:
        task->setStatus(TaskStatus::Review);
        break;
    case TaskStatus::Review:
        task->setStatus(TaskStatus::Done);
        break;
    case TaskStatus::Done:
        break;
    }

    refreshBoard();
}

void MainWindow::onShowStatistics() {
    QDialog dialog(this);
    dialog.setWindowTitle("Статистика доски");
    dialog.resize(500, 400);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    int backlog = board.getTasksByStatus(TaskStatus::Backlog).size();
    int assigned = board.getTasksByStatus(TaskStatus::Assigned).size();
    int inProgress = board.getTasksByStatus(TaskStatus::InProgress).size();
    int review = board.getTasksByStatus(TaskStatus::Review).size();
    int done = board.getTasksByStatus(TaskStatus::Done).size();
    int total = board.getTasks().size();

    int percentDone = total > 0 ? (done * 100 / total) : 0;

    QLabel* titleLabel = new QLabel("<h2>📊 Статистика проекта</h2>", &dialog);
    layout->addWidget(titleLabel);

    QLabel* totalLabel = new QLabel(QString("<b>Всего задач:</b> %1").arg(total), &dialog);
    layout->addWidget(totalLabel);

    QLabel* progressLabel = new QLabel(
        QString("<b>Процент выполнения:</b> %1%").arg(percentDone), &dialog);
    layout->addWidget(progressLabel);

    layout->addSpacing(10);

    QLabel* statusHeader = new QLabel("<b>Задачи по статусам:</b>", &dialog);
    layout->addWidget(statusHeader);

    QLabel* backlogLabel = new QLabel(QString("  🟡 БЭКЛОГ: %1").arg(backlog), &dialog);
    layout->addWidget(backlogLabel);

    QLabel* assignedLabel = new QLabel(QString("  🔵 НАДО: %1").arg(assigned), &dialog);
    layout->addWidget(assignedLabel);

    QLabel* inProgressLabel = new QLabel(QString("  🟢 ДЕЛАТЬ: %1").arg(inProgress), &dialog);
    layout->addWidget(inProgressLabel);

    QLabel* reviewLabel = new QLabel(QString("  🟣 ПРОВЕРКА: %1").arg(review), &dialog);
    layout->addWidget(reviewLabel);

    QLabel* doneLabel = new QLabel(QString("  ✅ СДЕЛАНО: %1").arg(done), &dialog);
    layout->addWidget(doneLabel);

    layout->addSpacing(10);

    int overdueCount = 0;
    int todayCount = 0;
    int soonCount = 0;

    for (const Task& task : board.getTasks()) {
        if (task.hasDeadline() && task.getStatus() != TaskStatus::Done) {
            if (task.isOverdue()) {
                overdueCount++;
            } else if (task.daysUntilDeadline() == 0) {
                todayCount++;
            } else if (task.daysUntilDeadline() <= 3) {
                soonCount++;
            }
        }
    }

    QLabel* deadlineHeader = new QLabel("<b>Дедлайны:</b>", &dialog);
    layout->addWidget(deadlineHeader);

    if (overdueCount > 0) {
        QLabel* overdueLabel = new QLabel(
            QString("  🔴 Просрочено: %1").arg(overdueCount), &dialog);
        overdueLabel->setStyleSheet("color: red; font-weight: bold;");
        layout->addWidget(overdueLabel);
    }

    if (todayCount > 0) {
        QLabel* todayLabel = new QLabel(
            QString("  🟠 Дедлайн сегодня: %1").arg(todayCount), &dialog);
        todayLabel->setStyleSheet("color: orange; font-weight: bold;");
        layout->addWidget(todayLabel);
    }

    if (soonCount > 0) {
        QLabel* soonLabel = new QLabel(
            QString("  🟡 Близкие дедлайны (1-3 дня): %1").arg(soonCount), &dialog);
        layout->addWidget(soonLabel);
    }

    if (overdueCount == 0 && todayCount == 0 && soonCount == 0) {
        QLabel* okLabel = new QLabel("  ✅ Все дедлайны под контролем!", &dialog);
        okLabel->setStyleSheet("color: green;");
        layout->addWidget(okLabel);
    }

    layout->addSpacing(10);

    QLabel* devHeader = new QLabel("<b>Разработчики:</b>", &dialog);
    layout->addWidget(devHeader);

    QLabel* devCountLabel = new QLabel(
        QString("  Всего: %1").arg(board.getDevelopers().size()), &dialog);
    layout->addWidget(devCountLabel);

    int maxTasks = 0;
    QString topDev = "—";
    for (const Developer& dev : board.getDevelopers()) {
        int taskCount = board.getTasksByDeveloper(dev.getId()).size();
        if (taskCount > maxTasks) {
            maxTasks = taskCount;
            topDev = dev.getName();
        }
    }

    if (maxTasks > 0) {
        QLabel* topDevLabel = new QLabel(
            QString("  🏆 Больше всего задач: %1 (%2 задач)").arg(topDev).arg(maxTasks),
            &dialog);
        layout->addWidget(topDevLabel);
    }

    layout->addStretch();

    QPushButton* closeBtn = new QPushButton("Закрыть", &dialog);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(closeBtn);

    dialog.exec();
}
