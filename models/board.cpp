#include "board.h"
#include <QFile> // Для работы с файлами (сохранение/загрузка)
#include <QJsonArray> // Для работы с JSON массивами
#include <QJsonDocument> // Для работы с JSON документами
 
Board::Board() {
}

void Board::addDeveloper(const Developer& developer) {  // developer передается по константной ссылке для избежания копирования
    developers.append(developer); // Добавляем копию разработчика в список
}

bool Board::removeDeveloper(int developerId) {
    for (int i = 0; i < developers.size(); i++) {
        if (developers[i].getId() == developerId) {
            // Снимаем назначение со всех задач этого разработчика
            for (Task& task : tasks) {
                if (task.getAssignedDeveloperId() == developerId) {
                    task.unassign();  // Снимаем назначение с задачи
                }
            }
            developers.removeAt(i);
            return true;
        }
    }
    return false; // Разработчик с таким ID не найден
}

// Получение разработчика по ID
Developer* Board::getDeveloper(int developerId) {
    for (Developer& dev : developers) {
        if (dev.getId() == developerId) {
            return &dev;
        }
    }
    return nullptr; // Если не нашли - возвращаем нулевой указатель
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
    return nullptr;  // Задача не найдена
}

QList<Task*> Board::getTasksByStatus(TaskStatus status) {
    QList<Task*> result;  // Создаем список указателей на задачи
    for (Task& task : tasks) {
        if (task.getStatus() == status) {
            result.append(&task); // Добавляем указатель на задачу в результат
        }
    }
    return result;  // Возвращаем список
}

QList<Task*> Board::getTasksByDeveloper(int developerId) {
    QList<Task*> result;
    for (Task& task : tasks) {         
        // Проверяем, назначена ли задача данному разработчику
        if (task.getAssignedDeveloperId() == developerId) {
            result.append(&task);
        }
    }
    return result;
}

bool Board::hasUnassignedTasks() const {
    for (const Task& task : tasks) {
                // Проверяем два условия:
        // 1. Задача не назначена (isAssigned() возвращает false)
        // 2. Задача НЕ находится в бэклоге
        // (логика: задачи в бэклоге еще не готовы к назначению)
        if (!task.isAssigned() && task.getStatus() != TaskStatus::Backlog) {
            return true; // Нашли неназначенную задачу
        }
    }
    return false;  // Все задачи либо назначены, либо в бэклоге
}

void Board::clear() {
    developers.clear();
    tasks.clear();
}

QJsonObject Board::toJson() const {
    QJsonObject json; // Создаем пустой JSON объект

    // Сохраняем разработчиков
    QJsonArray devsArray;
    for (const Developer& dev : developers) {
        devsArray.append(dev.toJson()); // Каждый разработчик должен иметь метод toJson()
    }
    json["developers"] = devsArray;  // Добавляем массив в JSON

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
    for (const QJsonValue& value : devsArray) {         // Developer должен иметь статический метод fromJson
        developers.append(Developer::fromJson(value.toObject()));
    }

    // Загружаем задачи
    QJsonArray tasksArray = json["tasks"].toArray();
    for (const QJsonValue& value : tasksArray) {
        tasks.append(Task::fromJson(value.toObject()));
    }
}

bool Board::saveToFile(const QString& filename) const {
    QFile file(filename); // Создаем объект файла
    // Открываем файл для записи
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    // Создаем JSON документ из нашего объекта
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

    QByteArray data = file.readAll();     // Читаем все содержимое файла
    file.close(); // Закрываем файл сразу после чтения

    QJsonDocument doc = QJsonDocument::fromJson(data);
    // Проверяем, что документ валиден и является объектом
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    fromJson(doc.object());
    return true;
}