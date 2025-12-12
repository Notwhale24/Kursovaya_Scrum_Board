#include <gtest/gtest.h> // Подключаем библиотеку Google Test
#include "../models/task.h"

class TaskTest : public ::testing::Test {
protected:
    void SetUp() override { // Вызывается перед каждым тестом
        // Сброс счетчика перед каждым тестом не делаем, т.к. он статистический
    }
};

TEST_F(TaskTest, CreateTask) {
    Task task("Тестовая задача", "Описание");

    EXPECT_FALSE(task.getTitle().isEmpty());
    EXPECT_EQ(task.getTitle(), "Тестовая задача");
    EXPECT_EQ(task.getDescription(), "Описание");
    EXPECT_EQ(task.getStatus(), TaskStatus::Backlog);
    EXPECT_FALSE(task.isAssigned());
}

TEST_F(TaskTest, AssignTask) {
    Task task("Задача");

    EXPECT_FALSE(task.isAssigned());

    task.assignToDeveloper(1);
    EXPECT_TRUE(task.isAssigned());
    EXPECT_EQ(task.getAssignedDeveloperId(), 1);

    task.unassign();
    EXPECT_FALSE(task.isAssigned());
    EXPECT_EQ(task.getAssignedDeveloperId(), -1);
}

TEST_F(TaskTest, ChangeStatus) {
    Task task("Задача");

    EXPECT_EQ(task.getStatus(), TaskStatus::Backlog);

    task.setStatus(TaskStatus::Assigned);
    EXPECT_EQ(task.getStatus(), TaskStatus::Assigned);

    task.setStatus(TaskStatus::InProgress);
    EXPECT_EQ(task.getStatus(), TaskStatus::InProgress);

    task.setStatus(TaskStatus::Review);
    EXPECT_EQ(task.getStatus(), TaskStatus::Review);

    task.setStatus(TaskStatus::Done);
    EXPECT_EQ(task.getStatus(), TaskStatus::Done);
}

TEST_F(TaskTest, StatusConversion) { // Проверка конвертации статусов в строки и обратно
    EXPECT_EQ(Task::statusToString(TaskStatus::Backlog), "Backlog");
    EXPECT_EQ(Task::statusToString(TaskStatus::Assigned), "Assigned");
    EXPECT_EQ(Task::statusToString(TaskStatus::InProgress), "InProgress");
    EXPECT_EQ(Task::statusToString(TaskStatus::Review), "Review");
    EXPECT_EQ(Task::statusToString(TaskStatus::Done), "Done");

    EXPECT_EQ(Task::stringToStatus("Backlog"), TaskStatus::Backlog);
    EXPECT_EQ(Task::stringToStatus("Assigned"), TaskStatus::Assigned);
    EXPECT_EQ(Task::stringToStatus("InProgress"), TaskStatus::InProgress);
    EXPECT_EQ(Task::stringToStatus("Review"), TaskStatus::Review);
    EXPECT_EQ(Task::stringToStatus("Done"), TaskStatus::Done);
}

TEST_F(TaskTest, JsonSerialization) { // Проверка JSON сериализации/десериализации
    Task task("Задача", "Описание");
    task.assignToDeveloper(5);
    task.setStatus(TaskStatus::InProgress);

    QJsonObject json = task.toJson();
    Task loaded = Task::fromJson(json);

    EXPECT_EQ(loaded.getTitle(), task.getTitle());
    EXPECT_EQ(loaded.getDescription(), task.getDescription());
    EXPECT_EQ(loaded.getStatus(), task.getStatus());
    EXPECT_EQ(loaded.getAssignedDeveloperId(), task.getAssignedDeveloperId());
}

// ========== ТЕСТЫ ДЛЯ ДЕДЛАЙНОВ ==========

TEST_F(TaskTest, DeadlineManagement) {
    Task task("Задача с дедлайном");

    // Изначально дедлайна нет
    EXPECT_FALSE(task.hasDeadline());

    // Устанавливаем дедлайн
    QDateTime deadline = QDateTime::currentDateTime().addDays(7);
    task.setDeadline(deadline);

    EXPECT_TRUE(task.hasDeadline());
    EXPECT_EQ(task.getDeadline(), deadline);

    // Удаляем дедлайн
    task.setDeadline(QDateTime());
    EXPECT_FALSE(task.hasDeadline());
    EXPECT_FALSE(task.getDeadline().isValid());
}

TEST_F(TaskTest, DaysUntilDeadline) {
    Task task("Задача");

    // Без дедлайна возвращает 999
    EXPECT_EQ(task.daysUntilDeadline(), 999);

    // Дедлайн через 7 дней
    QDateTime futureDeadline = QDateTime::currentDateTime().addDays(7);
    task.setDeadline(futureDeadline);
    int days = task.daysUntilDeadline();
    EXPECT_GE(days, 6); // >= 6 (может быть 7 в зависимости от времени)
    EXPECT_LE(days, 7); // <= 7

    // Дедлайн в прошлом
    QDateTime pastDeadline = QDateTime::currentDateTime().addDays(-5);
    task.setDeadline(pastDeadline);
    EXPECT_LT(task.daysUntilDeadline(), 0); // Отрицательное число
}

TEST_F(TaskTest, IsOverdue) {
    Task task("Задача");

    // Без дедлайна не просрочена
    EXPECT_FALSE(task.isOverdue());

    // С будущим дедлайном не просрочена
    task.setDeadline(QDateTime::currentDateTime().addDays(5));
    EXPECT_FALSE(task.isOverdue());

    // С прошедшим дедлайном и status != Done - просрочена
    task.setDeadline(QDateTime::currentDateTime().addDays(-2));
    EXPECT_TRUE(task.isOverdue());

    // С прошедшим дедлайном но status == Done - НЕ просрочена
    task.setStatus(TaskStatus::Done);
    EXPECT_FALSE(task.isOverdue());

    // Проверка с другими статусами
    task.setStatus(TaskStatus::InProgress);
    EXPECT_TRUE(task.isOverdue());
}

TEST_F(TaskTest, DeadlineEdgeCases) { // Граничные случаи дедлайнов
    Task task("Задача");

    // Дедлайн ровно сейчас (граница)
    QDateTime now = QDateTime::currentDateTime();
    task.setDeadline(now);
    EXPECT_TRUE(task.hasDeadline());

    // Дедлайн через несколько секунд (в пределах текущего дня)
    QDateTime soon = QDateTime::currentDateTime().addSecs(10);
    task.setDeadline(soon);
    int days = task.daysUntilDeadline();
    // Может быть 0 или 1 в зависимости от времени суток
    EXPECT_TRUE(days >= 0 && days <= 1);
}

// ========== ТЕСТЫ ИСТОРИИ ИЗМЕНЕНИЙ ==========

TEST_F(TaskTest, HistoryTracking) {
    Task task("Новая задача");

    // При создании должна быть одна запись в истории
    QList<TaskHistoryEntry> history = task.getHistory(); // Получаем список записей истории изменений задачи
    EXPECT_GE(history.size(), 1);

    // Проверяем первую запись (создание)
    EXPECT_EQ(history[0].action, "Создание");
    EXPECT_TRUE(history[0].details.contains("Новая задача"));
    EXPECT_TRUE(history[0].timestamp.isValid());
}

TEST_F(TaskTest, HistoryOnTitleChange) {
    Task task("Старое название");
    int initialSize = task.getHistory().size(); // Сохраняем начальный размер истории

    task.setTitle("Новое название");

    QList<TaskHistoryEntry> history = task.getHistory(); 
    EXPECT_EQ(history.size(), initialSize + 1);

    TaskHistoryEntry lastEntry = history.last();
    EXPECT_EQ(lastEntry.action, "Изменение названия");
    EXPECT_TRUE(lastEntry.details.contains("Старое название"));
    EXPECT_TRUE(lastEntry.details.contains("Новое название"));
}

TEST_F(TaskTest, HistoryOnStatusChange) {
    Task task("Задача");
    int initialSize = task.getHistory().size();

    task.setStatus(TaskStatus::InProgress);

    QList<TaskHistoryEntry> history = task.getHistory();
    EXPECT_EQ(history.size(), initialSize + 1);

    TaskHistoryEntry lastEntry = history.last();
    EXPECT_EQ(lastEntry.action, "Смена статуса");
    EXPECT_TRUE(lastEntry.details.contains("Backlog"));
    EXPECT_TRUE(lastEntry.details.contains("InProgress"));
}

TEST_F(TaskTest, HistoryOnAssignment) {
    Task task("Задача");
    int initialSize = task.getHistory().size();

    task.assignToDeveloper(42);

    QList<TaskHistoryEntry> history = task.getHistory();
    EXPECT_EQ(history.size(), initialSize + 1);

    TaskHistoryEntry lastEntry = history.last();
    EXPECT_EQ(lastEntry.action, "Назначение");
    EXPECT_TRUE(lastEntry.details.contains("42"));
}

TEST_F(TaskTest, HistoryOnUnassign) {
    Task task("Задача");
    task.assignToDeveloper(10);
    int sizeAfterAssign = task.getHistory().size();

    task.unassign();

    QList<TaskHistoryEntry> history = task.getHistory();
    EXPECT_EQ(history.size(), sizeAfterAssign + 1);

    TaskHistoryEntry lastEntry = history.last();
    EXPECT_EQ(lastEntry.action, "Снятие назначения");
}

TEST_F(TaskTest, HistoryOnDeadlineChange) {
    Task task("Задача");
    int initialSize = task.getHistory().size();

    QDateTime deadline = QDateTime::currentDateTime().addDays(5);
    task.setDeadline(deadline);

    QList<TaskHistoryEntry> history = task.getHistory();
    EXPECT_EQ(history.size(), initialSize + 1);

    TaskHistoryEntry lastEntry = history.last();
    EXPECT_EQ(lastEntry.action, "Изменение дедлайна");
    EXPECT_TRUE(lastEntry.details.contains("дедлайн"));
}

TEST_F(TaskTest, HistoryNoChangeNoDuplicate) { // Одинаковые изменения не создают дубликатов в истории
    Task task("Задача");
    task.setStatus(TaskStatus::InProgress);
    int sizeAfterFirstChange = task.getHistory().size();

    // Устанавливаем тот же статус - не должно быть новой записи
    task.setStatus(TaskStatus::InProgress);

    EXPECT_EQ(task.getHistory().size(), sizeAfterFirstChange);
}

// ========== JSON СЕРИАЛИЗАЦИЯ С ДЕДЛАЙНАМИ И ИСТОРИЕЙ ==========

TEST_F(TaskTest, JsonSerializationWithDeadline) {
    Task task("Задача с дедлайном", "Описание");
    QDateTime deadline = QDateTime::currentDateTime().addDays(10);
    task.setDeadline(deadline);
    task.setStatus(TaskStatus::Review);

    QJsonObject json = task.toJson();
    Task loaded = Task::fromJson(json);

    EXPECT_EQ(loaded.getTitle(), task.getTitle());
    EXPECT_EQ(loaded.getDescription(), task.getDescription());
    EXPECT_EQ(loaded.getStatus(), task.getStatus());
    EXPECT_TRUE(loaded.hasDeadline());

    // Сравнение времени с точностью до секунды (избегаем проблем с миллисекундами/ISO)
    EXPECT_EQ(loaded.getDeadline().toString(Qt::ISODate),
              task.getDeadline().toString(Qt::ISODate));
}

TEST_F(TaskTest, JsonSerializationWithHistory) {
    Task task("Задача", "Описание");
    task.setStatus(TaskStatus::InProgress);
    task.assignToDeveloper(5);
    task.setTitle("Обновленная задача");

    int historySize = task.getHistory().size();
    EXPECT_GE(historySize, 3); // Создание + статус + назначение + название

    QJsonObject json = task.toJson();
    Task loaded = Task::fromJson(json);

    EXPECT_EQ(loaded.getHistory().size(), historySize);

    // Проверяем первую запись истории
    QList<TaskHistoryEntry> loadedHistory = loaded.getHistory();
    EXPECT_TRUE(loadedHistory[0].timestamp.isValid());
    EXPECT_FALSE(loadedHistory[0].action.isEmpty());
    EXPECT_FALSE(loadedHistory[0].details.isEmpty());
}

TEST_F(TaskTest, JsonSerializationWithoutDeadline) {
    Task task("Задача без дедлайна");

    QJsonObject json = task.toJson();
    Task loaded = Task::fromJson(json);

    EXPECT_FALSE(loaded.hasDeadline());
    EXPECT_FALSE(loaded.getDeadline().isValid());
}
