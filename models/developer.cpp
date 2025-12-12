#include "developer.h"

int Developer::nextId = 1;

Developer::Developer()
    : id(nextId++),
    name(""),
    position("") {
}

Developer::Developer(const QString& name, const QString& position)
    : id(nextId++),
    name(name),
    position(position) {
}

QJsonObject Developer::toJson() const {
    QJsonObject json; // 1. Создаём пустой JSON-объект
    json["id"] = id; // 2. Добавляем поле "id"
    json["name"] = name;
    json["position"] = position;
    return json; // 5. Возвращаем готовый объект
}

Developer Developer::fromJson(const QJsonObject& json) {
    Developer dev; // 1. Создаём временный объект
    dev.id = json["id"].toInt();   // 2. Извлекаем id
    dev.name = json["name"].toString();
    dev.position = json["position"].toString();    // 5. Возвращаем объект

    // Обновляем nextId если нужно
    if (dev.id >= nextId) {
        nextId = dev.id + 1;
    }

    return dev;
}
