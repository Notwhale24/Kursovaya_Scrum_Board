#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QJsonObject>
#include <QDateTime>
#include <QList>

// Статусы задачи
enum class TaskStatus {
    Backlog,    // БЭКЛОГ
    Assigned,   // НАДО
    InProgress, // ДЕЛАТЬ
    Review,     // ПРОВЕРКА
    Done        // СДЕЛАНО!
};

// Запись в истории задачи
struct TaskHistoryEntry {
    QDateTime timestamp;
    QString action;
    QString details;

    QJsonObject toJson() const;
    static TaskHistoryEntry fromJson(const QJsonObject& json);
};

class Task {
public:
    Task();
    Task(const QString& title, const QString& description = "");

    // Геттеры
    int getId() const { return id; }
    QString getTitle() const { return title; }
    QString getDescription() const { return description; }
    TaskStatus getStatus() const { return status; }
    int getAssignedDeveloperId() const { return assignedDeveloperId; }
    QDateTime getDeadline() const { return deadline; }
    QList<TaskHistoryEntry> getHistory() const { return history; }

    // Сеттеры
    void setTitle(const QString& newTitle);
    void setDescription(const QString& newDescription);
    void setStatus(TaskStatus newStatus);
    void assignToDeveloper(int developerId);
    void unassign();
    void setDeadline(const QDateTime& newDeadline);

    // Проверка назначения и дедлайна
    bool isAssigned() const { return assignedDeveloperId != -1; }
    bool hasDeadline() const { return deadline.isValid(); }
    int daysUntilDeadline() const;
    bool isOverdue() const;

    // История
    void addHistoryEntry(const QString& action, const QString& details);

    // Сериализация
    QJsonObject toJson() const;
    static Task fromJson(const QJsonObject& json);

    // Конвертация статуса в строку и обратно
    static QString statusToString(TaskStatus status);
    static TaskStatus stringToStatus(const QString& str);

private:
    static int nextId;
    int id;
    QString title;
    QString description;
    TaskStatus status;
    int assignedDeveloperId; // -1 если не назначена
    QDateTime deadline;
    QList<TaskHistoryEntry> history;
};

#endif // TASK_H
