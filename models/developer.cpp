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
    QJsonObject json;
    json["id"] = id;
    json["name"] = name;
    json["position"] = position;
    return json;
}

Developer Developer::fromJson(const QJsonObject& json) {
    Developer dev;
    dev.id = json["id"].toInt();
    dev.name = json["name"].toString();
    dev.position = json["position"].toString();

    // Обновляем nextId если нужно
    if (dev.id >= nextId) {
        nextId = dev.id + 1;
    }

    return dev;
}
