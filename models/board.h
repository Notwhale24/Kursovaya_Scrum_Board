#ifndef BOARD_H
#define BOARD_H

#include <QList>  // Qt контейнер для хранения списков объектов
#include <QString> // Qt строковый класс (Юникод, безопасный)
#include <QJsonDocument>
#include "task.h"
#include "developer.h"

class Board {
public:
    Board();

    // Управление разработчиками
    void addDeveloper(const Developer& developer);
    bool removeDeveloper(int developerId);
    Developer* getDeveloper(int developerId); // Получение разработчика по ID
    QList<Developer>& getDevelopers() { return developers; } // Возвращает ссылку на список (позволяет модифицировать)
    const QList<Developer>& getDevelopers() const { return developers; } // Возвращает константную ссылку (только для чтения)

    // Управление задачами
    void addTask(const Task& task);
    bool removeTask(int taskId);
    Task* getTask(int taskId);
    QList<Task>& getTasks() { return tasks; }
    const QList<Task>& getTasks() const { return tasks; }

    // Получение задач по статусу
    QList<Task*> getTasksByStatus(TaskStatus status);

    // Получение задач разработчика
    QList<Task*> getTasksByDeveloper(int developerId);

    // Проверка: есть ли у задачи назначенный разработчик
    bool hasUnassignedTasks() const;

    // Сохранение и загрузка
    bool saveToFile(const QString& filename) const;
    bool loadFromFile(const QString& filename);

    // Очистка доски
    void clear();

private:
    QList<Developer> developers; // Список всех разработчиков
    QList<Task> tasks;

    QJsonObject toJson() const; //Создает JSON представление объекта
    void fromJson(const QJsonObject& json); // Восстанавливает объект из JSON
};

#endif // BOARD_H