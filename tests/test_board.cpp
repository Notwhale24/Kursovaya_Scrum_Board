#include <gtest/gtest.h> // Подключаем библиотеку Google Test
#include "../models/board.h"
#include <QFile> // Для работы с файлами

// Класс BoardTest наследуется от testing::Test
class BoardTest : public ::testing::Test {
protected:
    Board board; // Экземпляр Board, создается заново для каждого теста

    void TearDown() override { //Для очистки
        QFile::remove("test_board.json");
    }
};

TEST_F(BoardTest, AddDeveloper) {
    Developer dev("Тестовый разработчик", "Developer");
    board.addDeveloper(dev);

    EXPECT_EQ(board.getDevelopers().size(), 1); // Проверяем, что разработчик добавлен
    EXPECT_EQ(board.getDevelopers()[0].getName(), "Тестовый разработчик"); // Проверяем имя
}

TEST_F(BoardTest, RemoveDeveloper) {
    Developer dev("Иван", "Dev");
    board.addDeveloper(dev);
    int devId = board.getDevelopers()[0].getId();

    EXPECT_TRUE(board.removeDeveloper(devId));
    EXPECT_EQ(board.getDevelopers().size(), 0);

    EXPECT_FALSE(board.removeDeveloper(999)); // Пытаемся удалить несуществующего
}

TEST_F(BoardTest, AddTask) {
    Task task("Тестовая задача");
    board.addTask(task);

    EXPECT_EQ(board.getTasks().size(), 1);
    EXPECT_EQ(board.getTasks()[0].getTitle(), "Тестовая задача");
}

TEST_F(BoardTest, RemoveTask) {
    Task task("Задача");
    board.addTask(task);
    int taskId = board.getTasks()[0].getId();

    EXPECT_TRUE(board.removeTask(taskId));
    EXPECT_EQ(board.getTasks().size(), 0);

    EXPECT_FALSE(board.removeTask(999));
}

TEST_F(BoardTest, GetTasksByStatus) {
    Task task1("Задача 1");
    Task task2("Задача 2");
    Task task3("Задача 3");

    task1.setStatus(TaskStatus::Backlog);
    task2.setStatus(TaskStatus::InProgress);
    task3.setStatus(TaskStatus::InProgress);

    board.addTask(task1);
    board.addTask(task2);
    board.addTask(task3);

    // Получаем задачи со статусом Backlog
    QList<Task*> backlogTasks = board.getTasksByStatus(TaskStatus::Backlog);
    EXPECT_EQ(backlogTasks.size(), 1);

    // Получаем задачи со статусом InProgress
    QList<Task*> inProgressTasks = board.getTasksByStatus(TaskStatus::InProgress);
    EXPECT_EQ(inProgressTasks.size(), 2);
}

TEST_F(BoardTest, GetTasksByDeveloper) {
    Developer dev("Разработчик", "Dev");
    board.addDeveloper(dev);
    int devId = board.getDevelopers()[0].getId();

    Task task1("Задача 1");
    Task task2("Задача 2");
    Task task3("Задача 3");

    task1.assignToDeveloper(devId); // Назначаем задачи разработчику
    task2.assignToDeveloper(devId);

    board.addTask(task1);
    board.addTask(task2);
    board.addTask(task3);

    QList<Task*> devTasks = board.getTasksByDeveloper(devId);
    EXPECT_EQ(devTasks.size(), 2);
}

// Удаление разработчика снимает назначение с его задач
TEST_F(BoardTest, RemoveDeveloperUnassignsTasks) {
    Developer dev("Разработчик", "Dev");
    board.addDeveloper(dev);
    int devId = board.getDevelopers()[0].getId();

    Task task("Задача");
    task.assignToDeveloper(devId);
    board.addTask(task);

    EXPECT_TRUE(board.getTasks()[0].isAssigned()); // Проверяем, что задача назначена

    board.removeDeveloper(devId);

    EXPECT_FALSE(board.getTasks()[0].isAssigned()); // Задача должна стать неназначенной
}

// Проверка наличия неназначенных задач
TEST_F(BoardTest, HasUnassignedTasks) {
    Task task1("Задача 1");
    task1.setStatus(TaskStatus::Assigned);

    Task task2("Задача 2");
    task2.setStatus(TaskStatus::InProgress);

    board.addTask(task1);
    board.addTask(task2);

    // Обе задачи не назначены (hasUnassignedTasks проверяет задачи не в Backlog)
    EXPECT_TRUE(board.hasUnassignedTasks());

    // Получаем задачу из board и назначаем её
    int taskId = board.getTasks()[0].getId();
    Task* taskInBoard = board.getTask(taskId);
    taskInBoard->assignToDeveloper(1);

    // Теперь у нас всё ещё есть неназначенная задача (task2 в InProgress)
    EXPECT_TRUE(board.hasUnassignedTasks());

    // Назначаем и вторую задачу
    int task2Id = board.getTasks()[1].getId();
    Task* task2InBoard = board.getTask(task2Id);
    task2InBoard->assignToDeveloper(1);

    // Теперь все задачи назначены
    EXPECT_FALSE(board.hasUnassignedTasks());
}

TEST_F(BoardTest, SaveAndLoad) {
    Developer dev("Иван", "Developer");
    board.addDeveloper(dev);

    Task task("Задача");
    task.setStatus(TaskStatus::InProgress);
    board.addTask(task);

    EXPECT_TRUE(board.saveToFile("test_board.json"));

    Board newBoard;
    EXPECT_TRUE(newBoard.loadFromFile("test_board.json"));

    // Проверяем, что данные загрузились корректно
    EXPECT_EQ(newBoard.getDevelopers().size(), 1);
    EXPECT_EQ(newBoard.getDevelopers()[0].getName(), "Иван");

    EXPECT_EQ(newBoard.getTasks().size(), 1);
    EXPECT_EQ(newBoard.getTasks()[0].getTitle(), "Задача");
    EXPECT_EQ(newBoard.getTasks()[0].getStatus(), TaskStatus::InProgress);
}

TEST_F(BoardTest, Clear) {
    board.addDeveloper(Developer("Dev", "Position"));
    board.addTask(Task("Task"));

    EXPECT_GT(board.getDevelopers().size(), 0);
    EXPECT_GT(board.getTasks().size(), 0);

    board.clear();

    EXPECT_EQ(board.getDevelopers().size(), 0);
    EXPECT_EQ(board.getTasks().size(), 0);
}

// ========== ТЕСТЫ ПОИСКА И ПОЛУЧЕНИЯ ==========

TEST_F(BoardTest, GetDeveloperById) {
    Developer dev("Тестовый разработчик", "Developer");
    board.addDeveloper(dev);
    int devId = board.getDevelopers()[0].getId();

    Developer* found = board.getDeveloper(devId); // Ищем разработчика
    EXPECT_NE(found, nullptr); // Проверяем, что указатель не нулевой
    EXPECT_EQ(found->getId(), devId);
    EXPECT_EQ(found->getName(), "Тестовый разработчик");

    // Поиск несуществующего
    Developer* notFound = board.getDeveloper(99999);
    EXPECT_EQ(notFound, nullptr);
}

TEST_F(BoardTest, GetTaskById) {
    Task task("Тестовая задача");
    board.addTask(task);
    int taskId = board.getTasks()[0].getId();

    Task* found = board.getTask(taskId);
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found->getId(), taskId);
    EXPECT_EQ(found->getTitle(), "Тестовая задача");

    // Поиск несуществующей
    Task* notFound = board.getTask(99999);
    EXPECT_EQ(notFound, nullptr);
}

// ========== ТЕСТЫ КРАЙНИХ СЛУЧАЕВ ==========

TEST_F(BoardTest, EmptyBoardBehavior) {
    // Пустая доска
    EXPECT_EQ(board.getTasks().size(), 0);
    EXPECT_EQ(board.getDevelopers().size(), 0);

    // Получение задач по статусу на пустой доске
    QList<Task*> backlogTasks = board.getTasksByStatus(TaskStatus::Backlog);
    EXPECT_EQ(backlogTasks.size(), 0);

    // Получение задач разработчика на пустой доске
    QList<Task*> devTasks = board.getTasksByDeveloper(1);
    EXPECT_EQ(devTasks.size(), 0);

    // hasUnassignedTasks на пустой доске
    EXPECT_FALSE(board.hasUnassignedTasks());
}

TEST_F(BoardTest, LoadInvalidFile) {
    Board testBoard;

    // Загрузка несуществующего файла
    EXPECT_FALSE(testBoard.loadFromFile("nonexistent_file_12345.json"));

    // Загрузка файла с невалидным JSON
    QFile invalidFile("invalid_test.json");
    if (invalidFile.open(QIODevice::WriteOnly)) {
        QTextStream stream(&invalidFile);
        stream << "{ invalid json content [[[";
        invalidFile.close();
    }

    EXPECT_FALSE(testBoard.loadFromFile("invalid_test.json"));
    QFile::remove("invalid_test.json");

    // Загрузка пустого файла
    QFile emptyFile("empty_test.json");
    if (emptyFile.open(QIODevice::WriteOnly)) {
        emptyFile.close();
    }

    EXPECT_FALSE(testBoard.loadFromFile("empty_test.json"));
    QFile::remove("empty_test.json");
}

TEST_F(BoardTest, SaveToInvalidPath) {
    board.addTask(Task("Задача"));

    // Попытка сохранить в несуществующую директорию
    EXPECT_FALSE(board.saveToFile("/nonexistent_directory_12345/test.json"));
}

TEST_F(BoardTest, MultipleTasksWithSameStatus) { // Множество задач с одинаковым статусом
    for (int i = 0; i < 10; i++) {
        Task task(QString("Задача %1").arg(i)); // Создаем задачу с именем "Задача 0", ...
        task.setStatus(TaskStatus::InProgress);
        board.addTask(task);
    }

    QList<Task*> tasks = board.getTasksByStatus(TaskStatus::InProgress);
    EXPECT_EQ(tasks.size(), 10);

    // Все задачи должны быть InProgress
    for (Task* task : tasks) {
        EXPECT_EQ(task->getStatus(), TaskStatus::InProgress);
    }
}

TEST_F(BoardTest, RemoveNonexistentTask) {
    EXPECT_FALSE(board.removeTask(99999));
    EXPECT_FALSE(board.removeTask(-1));
    EXPECT_FALSE(board.removeTask(0));
}

TEST_F(BoardTest, RemoveNonexistentDeveloper) {
    EXPECT_FALSE(board.removeDeveloper(99999));
    EXPECT_FALSE(board.removeDeveloper(-1));
    EXPECT_FALSE(board.removeDeveloper(0));
}

// ========== ТЕСТЫ СТАТИСТИКИ И СЛОЖНЫХ СЦЕНАРИЕВ ==========

TEST_F(BoardTest, TaskCountByStatus) {
    board.addTask(Task("Backlog 1"));

    Task task2("InProgress 1");
    task2.setStatus(TaskStatus::InProgress);
    board.addTask(task2);

    Task task3("InProgress 2");
    task3.setStatus(TaskStatus::InProgress);
    board.addTask(task3);

    Task task4("Done 1");
    task4.setStatus(TaskStatus::Done);
    board.addTask(task4);

    EXPECT_EQ(board.getTasksByStatus(TaskStatus::Backlog).size(), 1);
    EXPECT_EQ(board.getTasksByStatus(TaskStatus::InProgress).size(), 2);
    EXPECT_EQ(board.getTasksByStatus(TaskStatus::Done).size(), 1);
    EXPECT_EQ(board.getTasksByStatus(TaskStatus::Assigned).size(), 0);
}

TEST_F(BoardTest, SaveAndLoadWithDeadlines) {
    Developer dev("Разработчик", "Dev");
    board.addDeveloper(dev);

    Task task1("Задача с дедлайном");
    task1.setDeadline(QDateTime::currentDateTime().addDays(7));
    task1.setStatus(TaskStatus::InProgress);
    board.addTask(task1);

    Task task2("Задача без дедлайна");
    board.addTask(task2);

    EXPECT_TRUE(board.saveToFile("test_board_deadline.json"));

    Board newBoard;
    EXPECT_TRUE(newBoard.loadFromFile("test_board_deadline.json"));

    EXPECT_EQ(newBoard.getTasks().size(), 2);

    // Проверяем, что задача с дедлайном загрузилась правильно
    bool foundTaskWithDeadline = false;
    for (const Task& task : newBoard.getTasks()) {
        if (task.getTitle() == "Задача с дедлайном") {
            EXPECT_TRUE(task.hasDeadline());
            foundTaskWithDeadline = true;
        }
        if (task.getTitle() == "Задача без дедлайна") {
            EXPECT_FALSE(task.hasDeadline());
        }
    }
    EXPECT_TRUE(foundTaskWithDeadline);

    QFile::remove("test_board_deadline.json");
}
