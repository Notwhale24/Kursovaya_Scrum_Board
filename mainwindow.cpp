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

// Конструктор главного окна приложения
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) { // Инициализация пользовательского интерфейса
    ui->setupUi(this);
    setupUI();
    setupMenuBar();
    setupToolBar();
    setWindowTitle("Скрам Доска");
    updateStatistics();
}

// Деструктор главного окна
MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Устанавливаем современный градиентный фон (как на стартовом экране)
    centralWidget->setStyleSheet(
        "QWidget {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, " // Линейный градиент
        "       stop:0 #E3F2FD, "
        "       stop:0.5 #F5F5F5, "
        "       stop:1 #FFF8E1);"
        "}"
    );

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);  // Основной горизонтальный компоновщик
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    columns[TaskStatus::Backlog] = new ColumnWidget("БЭКЛОГ", "#FFE082", TaskStatus::Backlog, this);
    columns[TaskStatus::Assigned] = new ColumnWidget("НАДО", "#90CAF9", TaskStatus::Assigned, this);
    columns[TaskStatus::InProgress] = new ColumnWidget("ДЕЛАТЬ", "#80CBC4", TaskStatus::InProgress, this);
    columns[TaskStatus::Review] = new ColumnWidget("ПРОВЕРКА", "#CE93D8", TaskStatus::Review, this);
    columns[TaskStatus::Done] = new ColumnWidget("СДЕЛАНО!", "#A5D6A7", TaskStatus::Done, this);
    
    // Подключение сигналов перетаскивания задач от каждой колонки
    for (ColumnWidget* column : columns) {
        connect(column, &ColumnWidget::taskDropped, this, &MainWindow::onTaskDropped);
    }
    
    // Добавление колонок в основной компоновщик в порядке слева направо
    mainLayout->addWidget(columns[TaskStatus::Backlog]);
    mainLayout->addWidget(columns[TaskStatus::Assigned]);
    mainLayout->addWidget(columns[TaskStatus::InProgress]);
    mainLayout->addWidget(columns[TaskStatus::Review]);
    mainLayout->addWidget(columns[TaskStatus::Done]);
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);  // Установка панели меню в главное окно

    QMenu* boardMenu = menuBar->addMenu("Доска");

    QAction* newBoardAction = boardMenu->addAction("Новая доска");
    newBoardAction->setShortcut(QKeySequence("Ctrl+N"));  // Горячая клавиша Ctrl+N
    connect(newBoardAction, &QAction::triggered, this, &MainWindow::onNewBoard); // Подключение сигнала

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
    toolBar->setMovable(false); // Запрет перемещения панели
    addToolBar(Qt::TopToolBarArea, toolBar);

    QLabel* searchLabel = new QLabel(" 🔍 Поиск: ", this);
    toolBar->addWidget(searchLabel);

    searchBox = new QLineEdit(this); // Создание поля ввода для поиска
    searchBox->setPlaceholderText("Введите название задачи...");
    searchBox->setMinimumWidth(200);
    connect(searchBox, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    toolBar->addWidget(searchBox); // Добавление поля поиска на панель


    toolBar->addSeparator(); // Добавление разделителя на панель инструментов

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

        // Пропуск задач, не соответствующих поисковому запросу
        if (!matchesSearch(&task)) {
            continue;
        }

        TaskCard* card = new TaskCard(&task, &board, this); // Создание карточки для задачи

        connect(card, &TaskCard::editClicked, this, &MainWindow::onTaskEdit);
        connect(card, &TaskCard::deleteClicked, this, &MainWindow::onTaskDelete);
        connect(card, &TaskCard::statusChangeClicked, this, &MainWindow::onTaskStatusChange);

        columns[task.getStatus()]->addTaskCard(card);
    }

    showWarningIfUnassigned(); // Предупреждение о неназначеннных задачах
    updateStatistics();
}

// Проверка соответствия задачи поисковому запросу
bool MainWindow::matchesSearch(Task* task) {
    if (searchFilter.isEmpty()) {
        return true;
    }

    return task->getTitle().toLower().contains(searchFilter.toLower());     // Поиск по названию задачи (без учета регистра)
}

void MainWindow::onSearchTextChanged(const QString& text) {
    searchFilter = text;
    refreshBoard();
}

void MainWindow::updateStatistics() {
    // Подсчет задач по статусам
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

// Загрузка доски из указанного файла
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

    // Кнопки OK и Cancel
    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog
        );
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Если пользователь нажал OK
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

    // Цикл перебора всех разработчиков из доски и отображение их в списке
    for (const Developer& dev : board.getDevelopers()) {
        QString text = QString("%1 - %2 (ID: %3)")
        .arg(dev.getName())
            .arg(dev.getPosition())
            .arg(dev.getId());
        QListWidgetItem* item = new QListWidgetItem(text); // Создание нового элемента списка (QListWidgetItem) с текстом
        item->setData(Qt::UserRole, dev.getId()); // Qt::UserRole - специальное значение для хранения пользовательских данных
        devList->addItem(item);
    }

    layout->addWidget(devList); // Добавление списка в компоновщик

    QPushButton* deleteBtn = new QPushButton("Удалить выбранного", &dialog);
    layout->addWidget(deleteBtn);
    
    // Обработчик нажатия кнопки удаления
    connect(deleteBtn, &QPushButton::clicked, [this, devList, &dialog]() {
        QListWidgetItem* item = devList->currentItem(); // Получение выбранного элемента
        if (item) {
            int devId = item->data(Qt::UserRole).toInt();  // Получение ID разработчика

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

// Слот для обработки перетаскивания задачи между колонками
void MainWindow::onTaskDropped(int taskId, TaskStatus newStatus) {
    Task* task = board.getTask(taskId); // Получение задачи по ID
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
            descEdit->setPlainText(descEdit->toPlainText().left(120)); // Обрезать до 120 символов
            QTextCursor cursor = descEdit->textCursor();
            cursor.movePosition(QTextCursor::End); // Переместить курсор в конец
            descEdit->setTextCursor(cursor);
        }
    });

    // Дедлайн
    QDateEdit* deadlineEdit = new QDateEdit(&dialog);
    deadlineEdit->setCalendarPopup(true);  // Всплывающий календарь
    deadlineEdit->setDisplayFormat("dd.MM.yyyy");  // Формат даты
    deadlineEdit->setMinimumDate(QDate::currentDate());  // Минимальная дата - сегодня
    deadlineEdit->setDate(QDate::currentDate().addDays(7));  // Значение по умолчанию: через 7 дней

     // Флажок для установки дедлайна
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

        Task task(title, desc);  // Создание задачи

        // Устанавливаем дедлайн если выбран
        if (hasDeadlineCheck->isChecked()) {
            QDateTime deadline(deadlineEdit->date(), QTime(23, 59, 59)); // Дедлайн до конца дня
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
    // Выпадающий список для выбора разработчика
    QComboBox* devCombo = new QComboBox(&dialog);
    devCombo->addItem("Не назначена", -1);
    
    // board.getDevelopers() - метод, возвращающий контейнер разработчиков
    for (const Developer& dev : board.getDevelopers()) {
        // Добавление элемента в выпадающий список
        devCombo->addItem(
            QString("%1 (%2)").arg(dev.getName()).arg(dev.getPosition()), // 1. Видимый текст для отображения пользователю
            dev.getId()  // 2. Данные элемента (user data), связанные с этим пунктом списка
            );
    }
    // Проверка, назначена ли задача какому-либо разработчику
    if (task->isAssigned()) {
        for (int i = 0; i < devCombo->count(); i++) {
            // itemData(i) возвращает QVariant - универсальный тип данных Qt
            // .toInt() преобразует QVariant в целое число (ID разработчика)
            if (devCombo->itemData(i).toInt() == task->getAssignedDeveloperId()) {  
                devCombo->setCurrentIndex(i); // Если ID совпадает - устанавливаем этот элемент как текущий выбранный
                break;
            }
        }
    }

    // Поле для дедлайна
    QDateEdit* deadlineEdit = new QDateEdit(&dialog);
    deadlineEdit->setCalendarPopup(true);
    deadlineEdit->setDisplayFormat("dd.MM.yyyy");
    deadlineEdit->setMinimumDate(QDate::currentDate());

    // Установка текущего дедлайна задачи или значения по умолчанию
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

        int devId = devCombo->currentData().toInt();   // Назначение/снятие разработчик
        if (devId == -1) {
            task->unassign();  // Снять назначение
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

// Слот для изменения статуса задачи (по нажатию кнопки на карточке)
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
    
    // Статистика по дедлайнам
    int overdueCount = 0; // Просроченные задачи
    int todayCount = 0;  // Дедлайн сегодня
    int soonCount = 0; // Дедлайн в ближайшие 1-3 дня

    // Подсчет задач по срочности дедлайнов
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

    // Поиск разработчика с наибольшим количеством задач
    int maxTasks = 0;
    QString topDev = "—";
    for (const Developer& dev : board.getDevelopers()) {
        int taskCount = board.getTasksByDeveloper(dev.getId()).size();
        if (taskCount > maxTasks) {
            maxTasks = taskCount;
            topDev = dev.getName();
        }
    }

    // Отображение разработчика с наибольшим количеством задач
    if (maxTasks > 0) {
        QLabel* topDevLabel = new QLabel(
            QString("  🏆 Больше всего задач: %1 (%2 задач)").arg(topDev).arg(maxTasks),
            &dialog);
        layout->addWidget(topDevLabel);
    }

    layout->addStretch();  // Добавление растягивающегося пространства

    QPushButton* closeBtn = new QPushButton("Закрыть", &dialog);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(closeBtn);

    dialog.exec();
}
