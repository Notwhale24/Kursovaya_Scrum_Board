#include <gtest/gtest.h> // Подключаем библиотеку Google Test
#include "../models/task.h"

class TaskHistoryEntryTest : public ::testing::Test {
};

TEST_F(TaskHistoryEntryTest, CreateEntry) { // Создание записи истории
    TaskHistoryEntry entry; // Создаем объект записи истории
    entry.timestamp = QDateTime::currentDateTime();
    entry.action = "Тестовое действие";
    entry.details = "Детали действия";

    EXPECT_TRUE(entry.timestamp.isValid());
    EXPECT_EQ(entry.action, "Тестовое действие");
    EXPECT_EQ(entry.details, "Детали действия");
}

TEST_F(TaskHistoryEntryTest, JsonSerialization) { // Сериализация и десериализация в/из JSON
    TaskHistoryEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.action = "Создание задачи";
    entry.details = "Задача 'Тест' создана";

    QJsonObject json = entry.toJson();
    TaskHistoryEntry loaded = TaskHistoryEntry::fromJson(json);

    EXPECT_EQ(loaded.action, entry.action);
    EXPECT_EQ(loaded.details, entry.details);

    // Сравниваем время с точностью до секунды
    EXPECT_EQ(loaded.timestamp.toString(Qt::ISODate),
              entry.timestamp.toString(Qt::ISODate));
}

TEST_F(TaskHistoryEntryTest, EmptyFields) {
    TaskHistoryEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.action = "";
    entry.details = "";

    QJsonObject json = entry.toJson();
    TaskHistoryEntry loaded = TaskHistoryEntry::fromJson(json);

    EXPECT_TRUE(loaded.action.isEmpty());
    EXPECT_TRUE(loaded.details.isEmpty());
    EXPECT_TRUE(loaded.timestamp.isValid());
}

TEST_F(TaskHistoryEntryTest, SpecialCharactersInDetails) {
    TaskHistoryEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.action = "Изменение";
    entry.details = "Название: 'Старое' → 'Новое' (с \"кавычками\")";

    QJsonObject json = entry.toJson();
    TaskHistoryEntry loaded = TaskHistoryEntry::fromJson(json);

    EXPECT_EQ(loaded.details, entry.details);
}

TEST_F(TaskHistoryEntryTest, UnicodeInDetails) {
    TaskHistoryEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.action = "更新";
    entry.details = "任务已更新 🎉";

    QJsonObject json = entry.toJson();
    TaskHistoryEntry loaded = TaskHistoryEntry::fromJson(json);

    EXPECT_EQ(loaded.action, "更新");
    EXPECT_EQ(loaded.details, "任务已更新 🎉");
}

TEST_F(TaskHistoryEntryTest, LongDetails) {
    TaskHistoryEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.action = "Действие";
    entry.details = QString("X").repeated(10000);

    QJsonObject json = entry.toJson();
    TaskHistoryEntry loaded = TaskHistoryEntry::fromJson(json);

    EXPECT_EQ(loaded.details.length(), 10000);
}

TEST_F(TaskHistoryEntryTest, InvalidTimestamp) {
    TaskHistoryEntry entry;
    entry.timestamp = QDateTime(); // Невалидный временная метка
    entry.action = "Действие";
    entry.details = "Детали";

    QJsonObject json = entry.toJson();
    TaskHistoryEntry loaded = TaskHistoryEntry::fromJson(json);

    // Даже невалидный timestamp должен сериализоваться
    EXPECT_FALSE(loaded.timestamp.isValid());
    EXPECT_EQ(loaded.action, "Действие");
    EXPECT_EQ(loaded.details, "Детали");
}

TEST_F(TaskHistoryEntryTest, MultilineDetails) { // Многострочные детали
    TaskHistoryEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.action = "Обновление";
    entry.details = "Строка 1\nСтрока 2\nСтрока 3";

    QJsonObject json = entry.toJson();
    TaskHistoryEntry loaded = TaskHistoryEntry::fromJson(json);

    EXPECT_EQ(loaded.details, "Строка 1\nСтрока 2\nСтрока 3");
}

TEST_F(TaskHistoryEntryTest, FutureTimestamp) { // Будущая временная метка
    TaskHistoryEntry entry;
    entry.timestamp = QDateTime::currentDateTime().addYears(10);
    entry.action = "Будущее действие";
    entry.details = "Запланировано на будущее";

    QJsonObject json = entry.toJson();
    TaskHistoryEntry loaded = TaskHistoryEntry::fromJson(json);

    EXPECT_TRUE(loaded.timestamp > QDateTime::currentDateTime());
    EXPECT_EQ(loaded.action, "Будущее действие");
}

TEST_F(TaskHistoryEntryTest, PastTimestamp) {
    TaskHistoryEntry entry;
    entry.timestamp = QDateTime::currentDateTime().addYears(-10);
    entry.action = "Прошлое действие";
    entry.details = "Произошло 10 лет назад";

    QJsonObject json = entry.toJson();
    TaskHistoryEntry loaded = TaskHistoryEntry::fromJson(json);

    EXPECT_TRUE(loaded.timestamp < QDateTime::currentDateTime());
    EXPECT_EQ(loaded.details, "Произошло 10 лет назад");
}

// ========== ИНТЕГРАЦИЯ С ЗАДАЧЕЙ ==========

TEST_F(TaskHistoryEntryTest, IntegrationWithTask) {
    Task task("Задача с историей");

    // Проверяем, что создание задачи добавило запись в историю
    QList<TaskHistoryEntry> history = task.getHistory();
    EXPECT_GE(history.size(), 1);

    // Изменяем задачу и проверяем добавление записей
    task.setStatus(TaskStatus::InProgress);
    task.assignToDeveloper(5);

    history = task.getHistory();
    EXPECT_GE(history.size(), 3); // Создание + статус + назначение

    // Проверяем что все записи валидны
    for (const TaskHistoryEntry& entry : history) {
        EXPECT_TRUE(entry.timestamp.isValid());
        EXPECT_FALSE(entry.action.isEmpty());
        EXPECT_FALSE(entry.details.isEmpty());
    }
}
