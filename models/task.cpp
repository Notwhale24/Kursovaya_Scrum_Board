#include "task.h"
#include <QJsonArray>

int Task::nextId = 1;

Task::Task()
    : id(nextId++),
    title(""),
    description(""),
    status(TaskStatus::Backlog), //Начальный статус - Backlog
    assignedDeveloperId(-1) { //задача не назначена
    addHistoryEntry("Создание", "Задача создана");
}

Task::Task(const QString& title, const QString& description)
    : id(nextId++),
    title(title),
    description(description),
    status(TaskStatus::Backlog),
    assignedDeveloperId(-1) {
    addHistoryEntry("Создание", QString("Задача '%1' создана").arg(title));
}

void Task::setTitle(const QString& newTitle) {
    if (title != newTitle) {
        addHistoryEntry("Изменение названия", QString("'%1' → '%2'").arg(title, newTitle));
        title = newTitle;
    }
}

void Task::setDescription(const QString& newDescription) {
    if (description != newDescription) {
        addHistoryEntry("Изменение описания", "Описание обновлено");
        description = newDescription;
    }
}

void Task::setStatus(TaskStatus newStatus) {
    if (status != newStatus) {
        addHistoryEntry("Смена статуса",
                        QString("%1 → %2").arg(statusToString(status), statusToString(newStatus)));
        status = newStatus;
    }
}

void Task::assignToDeveloper(int developerId) {
    if (assignedDeveloperId != developerId) {
        addHistoryEntry("Назначение", QString("Назначен разработчик ID: %1").arg(developerId));
        assignedDeveloperId = developerId;
    }
}

void Task::unassign() {
    if (assignedDeveloperId != -1) {
        addHistoryEntry("Снятие назначения", "Разработчик снят с задачи");
        assignedDeveloperId = -1;
    }
}

void Task::setDeadline(const QDateTime& newDeadline) {
    if (deadline != newDeadline) {
        QString details = newDeadline.isValid() //isValid() для проверки корректности даты
        ? QString("Установлен дедлайн: %1").arg(newDeadline.toString("dd.MM.yyyy"))
        : "Дедлайн удален";
        addHistoryEntry("Изменение дедлайна", details);
        deadline = newDeadline;
    }
}

int Task::daysUntilDeadline() const {
    if (!deadline.isValid()) return 999;
    return QDateTime::currentDateTime().daysTo(deadline);
}

bool Task::isOverdue() const {
    if (!deadline.isValid()) return false;
    return QDateTime::currentDateTime() > deadline && status != TaskStatus::Done; //Возвращает true если: Дедлайн установлен, текущее время позже дедлайна, задача не в статусе Done
}

void Task::addHistoryEntry(const QString& action, const QString& details) {
    TaskHistoryEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.action = action;
    entry.details = details;
    history.append(entry);
} 

QJsonObject Task::toJson() const {
    QJsonObject json; //Создает JSON-объект со всеми данными задачи
    json["id"] = id;
    json["title"] = title;
    json["description"] = description;
    json["status"] = statusToString(status);
    json["assignedDeveloperId"] = assignedDeveloperId;
    json["deadline"] = deadline.isValid() ? deadline.toString(Qt::ISODate) : ""; //стандарт ISO для дат

    QJsonArray historyArray;
    for (const TaskHistoryEntry& entry : history) {
        historyArray.append(entry.toJson());
    }
    json["history"] = historyArray;

    return json; //Сохраняет всю историю как массив JSON-объектов
}

Task Task::fromJson(const QJsonObject& json) {
    Task task; // Создает временный объект Task с помощью конструктора 
    task.id = json["id"].toInt();
    task.title = json["title"].toString();
    task.description = json["description"].toString();
    task.status = stringToStatus(json["status"].toString());
    task.assignedDeveloperId = json["assignedDeveloperId"].toInt();

    QString deadlineStr = json["deadline"].toString(); //Обрабатывает дедлайн (пустая строка = нет дедлайна)
    if (!deadlineStr.isEmpty()) {
        task.deadline = QDateTime::fromString(deadlineStr, Qt::ISODate);
    }

    task.history.clear();
    QJsonArray historyArray = json["history"].toArray(); //Восстанавливает историю из JSON-массива
    for (const QJsonValue& value : historyArray) {
        task.history.append(TaskHistoryEntry::fromJson(value.toObject()));
    }

    if (task.id >= nextId) { 
        nextId = task.id + 1; //Обновляет статический nextId если загруженный ID больше текущего
    }

    return task;
}

QString Task::statusToString(TaskStatus status) { //TaskStatus (enum class) автоматически конвертируется в int
    switch (status) { //switch - работает только с целыми типами (int, char, enum)
    case TaskStatus::Backlog:    return "Backlog";
    case TaskStatus::Assigned:   return "Assigned";
    case TaskStatus::InProgress: return "InProgress";
    case TaskStatus::Review:     return "Review";
    case TaskStatus::Done:       return "Done";
    default:                     return "Backlog"; // Защита от некорректных значений
    }
}

TaskStatus Task::stringToStatus(const QString& str) { //QString (объект строки)
    if (str == "Backlog")    return TaskStatus::Backlog; // цепочка if - строки нельзя использовать в switch
    if (str == "Assigned")   return TaskStatus::Assigned;
    if (str == "InProgress") return TaskStatus::InProgress;
    if (str == "Review")     return TaskStatus::Review;
    if (str == "Done")       return TaskStatus::Done;
    return TaskStatus::Backlog;
}

// Сериализация объекта TaskHistoryEntry в формат JSON
QJsonObject TaskHistoryEntry::toJson() const {
    QJsonObject json; // Создаем пустой JSON-объект
    json["timestamp"] = timestamp.toString(Qt::ISODate);
    json["action"] = action;
    json["details"] = details;
    return json; // Возвращаем заполненный JSON-объект
}
// Десериализация JSON-объекта обратно в объект TaskHistoryEntry
TaskHistoryEntry TaskHistoryEntry::fromJson(const QJsonObject& json) {
    TaskHistoryEntry entry; // Создаем новый объект записи истории
    entry.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate); //Все поля сохраняются как строки
    entry.action = json["action"].toString(); 
    entry.details = json["details"].toString();
    return entry; // Возвращаем восстановленный объект записи истории
}