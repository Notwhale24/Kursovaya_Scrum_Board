#ifndef DEVELOPER_H
#define DEVELOPER_H

#include <QString>
#include <QJsonObject>

class Developer {
public:
    Developer();
    Developer(const QString& name, const QString& position = "");

    // Геттеры возвращают значения приватных полей
    int getId() const { return id; }
    QString getName() const { return name; }
    QString getPosition() const { return position; }

    // Сеттеры устанавливают новые значения для полей name и position
    void setName(const QString& newName) { name = newName; }
    void setPosition(const QString& newPosition) { position = newPosition; }

    // Сериализация
    QJsonObject toJson() const; //преобразует объект Developer в JSON-объект
    static Developer fromJson(const QJsonObject& json); //создаёт объект Developer из JSON-объекта.

private:
    static int nextId; //статическая переменная для генерации уникальных ID
    int id;
    QString name;
    QString position;
};

#endif // DEVELOPER_H