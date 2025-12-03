#include "board.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

Board::Board() {
}

void Board::addDeveloper(const Developer& developer) {
    developers.append(developer);
}

bool Board::removeDeveloper(int developerId) {
    for (int i = 0; i < developers.size(); i++) {
        if (developers[i].getId() == developerId) {
            // Снимаем назначение со всех задач этого разработчика
            for (Task& task : tasks) {
                if (task.getAssignedDeveloperId() == developerId) {
                    task.unassign();
                }
            }
            developers.removeAt(i);
            return true;
        }
    }
    return false;
}

Developer* Board::getDeveloper(int developerId) {
    for (Developer& dev : developers) {
        if (dev.getId() == developerId) {
            return &dev;
        }
    }
    return nullptr;
}

void Board::addTask(const Task& task) {
    tasks.append(task);
}

bool Board::removeTask(int taskId) {
    for (int i = 0; i < tasks.size(); i++) {
        if (tasks[i].getId() == taskId) {
            tasks.removeAt(i);
            return true;
        }
    }
    return false;
}

Task* Board::getTask(int taskId) {
    for (Task& task : tasks) {
        if (task.getId() == taskId) {
            return &task;
        }
    }
    return nullptr;
}

QList<Task*> Board::getTasksByStatus(TaskStatus status) {
    QList<Task*> result;
    for (Task& task : tasks) {
        if (task.getStatus() == status) {
            result.append(&task);
        }
    }
    return result;
}

QList<Task*> Board::getTasksByDeveloper(int developerId) {
    QList<Task*> result;
    for (Task& task : tasks) {
        if (task.getAssignedDeveloperId() == developerId) {
            result.append(&task);
        }
    }
    return result;
}

bool Board::hasUnassignedTasks() const {
    for (const Task& task : tasks) {
        if (!task.isAssigned() && task.getStatus() != TaskStatus::Backlog) {
            return true;
        }
    }
    return false;
}

void Board::clear() {
    developers.clear();
    tasks.clear();
}

QJsonObject Board::toJson() const {
    QJsonObject json;

    // Сохраняем разработчиков
    QJsonArray devsArray;
    for (const Developer& dev : developers) {
        devsArray.append(dev.toJson());
    }
    json["developers"] = devsArray;

    // Сохраняем задачи
    QJsonArray tasksArray;
    for (const Task& task : tasks) {
        tasksArray.append(task.toJson());
    }
    json["tasks"] = tasksArray;

    return json;
}

void Board::fromJson(const QJsonObject& json) {
    clear();

    // Загружаем разработчиков
    QJsonArray devsArray = json["developers"].toArray();
    for (const QJsonValue& value : devsArray) {
        developers.append(Developer::fromJson(value.toObject()));
    }

    // Загружаем задачи
    QJsonArray tasksArray = json["tasks"].toArray();
    for (const QJsonValue& value : tasksArray) {
        tasks.append(Task::fromJson(value.toObject()));
    }
}

bool Board::saveToFile(const QString& filename) const {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(toJson());
    file.write(doc.toJson());
    file.close();
    return true;
}

bool Board::loadFromFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    fromJson(doc.object());
    return true;
}
