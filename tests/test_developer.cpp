#include <gtest/gtest.h> // Подключаем библиотеку Google Test
#include "../models/developer.h"

class DeveloperTest : public ::testing::Test {
};

TEST_F(DeveloperTest, CreateDeveloper) {
    Developer dev("Иван Иванов", "Senior Developer");

    EXPECT_FALSE(dev.getName().isEmpty());
    EXPECT_EQ(dev.getName(), "Иван Иванов");
    EXPECT_EQ(dev.getPosition(), "Senior Developer");
}

TEST_F(DeveloperTest, ModifyDeveloper) {
    Developer dev("Петр", "Junior");

    dev.setName("Петр Петров");
    EXPECT_EQ(dev.getName(), "Петр Петров");

    dev.setPosition("Middle Developer");
    EXPECT_EQ(dev.getPosition(), "Middle Developer");
}

// Тест для проверки сериализации в JSON и обратной десериализации
TEST_F(DeveloperTest, JsonSerialization) {
    Developer dev("Анна", "QA Engineer");

    QJsonObject json = dev.toJson();
    Developer loaded = Developer::fromJson(json);

    EXPECT_EQ(loaded.getName(), dev.getName());
    EXPECT_EQ(loaded.getPosition(), dev.getPosition());
}

// ========== КРАЙНИЕ СЛУЧАИ ==========

TEST_F(DeveloperTest, EmptyName) {
    Developer dev("", "Developer");

    EXPECT_TRUE(dev.getName().isEmpty());
    EXPECT_EQ(dev.getPosition(), "Developer");

    // Сериализация должна работать даже с пустым именем
    QJsonObject json = dev.toJson();
    Developer loaded = Developer::fromJson(json);

    EXPECT_EQ(loaded.getName(), "");
    EXPECT_EQ(loaded.getPosition(), "Developer");
}

TEST_F(DeveloperTest, EmptyPosition) {
    Developer dev("Разработчик", "");

    EXPECT_EQ(dev.getName(), "Разработчик");
    EXPECT_TRUE(dev.getPosition().isEmpty());

    // Сериализация должна работать даже с пустой позицией
    QJsonObject json = dev.toJson();
    Developer loaded = Developer::fromJson(json);

    EXPECT_EQ(loaded.getName(), "Разработчик");
    EXPECT_EQ(loaded.getPosition(), "");
}

TEST_F(DeveloperTest, BothEmpty) {
    Developer dev("", "");

    EXPECT_TRUE(dev.getName().isEmpty());
    EXPECT_TRUE(dev.getPosition().isEmpty());

    // Сериализация должна работать
    QJsonObject json = dev.toJson();
    Developer loaded = Developer::fromJson(json);

    EXPECT_TRUE(loaded.getName().isEmpty());
    EXPECT_TRUE(loaded.getPosition().isEmpty());
}

TEST_F(DeveloperTest, SpecialCharacters) { // Спец символы
    Developer dev("Иван О'Коннор", "C++ / Qt Developer (Senior)");

    EXPECT_EQ(dev.getName(), "Иван О'Коннор");
    EXPECT_EQ(dev.getPosition(), "C++ / Qt Developer (Senior)");

    // Сериализация должна правильно обрабатывать спецсимволы
    QJsonObject json = dev.toJson();
    Developer loaded = Developer::fromJson(json);

    EXPECT_EQ(loaded.getName(), "Иван О'Коннор");
    EXPECT_EQ(loaded.getPosition(), "C++ / Qt Developer (Senior)");
}

TEST_F(DeveloperTest, UnicodeCharacters) { // Иероглифы
    Developer dev("张伟", "开发者");

    EXPECT_EQ(dev.getName(), "张伟");
    EXPECT_EQ(dev.getPosition(), "开发者");

    // Сериализация должна правильно обрабатывать Unicode
    QJsonObject json = dev.toJson();
    Developer loaded = Developer::fromJson(json);

    EXPECT_EQ(loaded.getName(), "张伟");
    EXPECT_EQ(loaded.getPosition(), "开发者");
}

TEST_F(DeveloperTest, LongStrings) {
    QString longName = QString("A").repeated(1000);
    QString longPosition = QString("B").repeated(1000);

    Developer dev(longName, longPosition);

    EXPECT_EQ(dev.getName().length(), 1000);
    EXPECT_EQ(dev.getPosition().length(), 1000);

    // Сериализация должна работать с длинными строками
    QJsonObject json = dev.toJson();
    Developer loaded = Developer::fromJson(json);

    EXPECT_EQ(loaded.getName().length(), 1000);
    EXPECT_EQ(loaded.getPosition().length(), 1000);
}

TEST_F(DeveloperTest, MultipleChanges) {
    Developer dev("Старое имя", "Старая позиция");

    dev.setName("Промежуточное имя");
    dev.setPosition("Промежуточная позиция");
    dev.setName("Финальное имя");
    dev.setPosition("Финальная позиция");

    EXPECT_EQ(dev.getName(), "Финальное имя");
    EXPECT_EQ(dev.getPosition(), "Финальная позиция");

    // После множественных изменений сериализация должна работать
    QJsonObject json = dev.toJson();
    Developer loaded = Developer::fromJson(json);

    EXPECT_EQ(loaded.getName(), "Финальное имя");
    EXPECT_EQ(loaded.getPosition(), "Финальная позиция");
}

TEST_F(DeveloperTest, IdPersistence) { // Постоянство и уникальность ID разработчика
    Developer dev1("Dev 1", "Position 1");
    Developer dev2("Dev 2", "Position 2");

    int id1 = dev1.getId();
    int id2 = dev2.getId();

    // ID должны быть разными
    EXPECT_NE(id1, id2);

    // ID не должны меняться при изменении полей
    dev1.setName("New Name");
    dev1.setPosition("New Position");

    EXPECT_EQ(dev1.getId(), id1);

    // ID должен сохраняться при сериализации
    QJsonObject json = dev1.toJson();
    Developer loaded = Developer::fromJson(json);

    EXPECT_EQ(loaded.getId(), id1);
}
