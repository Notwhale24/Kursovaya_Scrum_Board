#ifndef DEVELOPER_H
#define DEVELOPER_H

#include <QString>
#include <QJsonObject>

class Developer {
public:
    Developer();
    Developer(const QString& name, const QString& position = "");

    // Геттеры
    int getId() const { return id; }
    QString getName() const { return name; }
    QString getPosition() const { return position; }

    // Сеттеры
    void setName(const QString& newName) { name = newName; }
    void setPosition(const QString& newPosition) { position = newPosition; }

    // Сериализация
    QJsonObject toJson() const;
    static Developer fromJson(const QJsonObject& json);

private:
    static int nextId;
    int id;
    QString name;
    QString position;
};

#endif // DEVELOPER_H
