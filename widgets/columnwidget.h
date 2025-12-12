#ifndef COLUMNWIDGET_H
#define COLUMNWIDGET_H

#include <QWidget> // Базовый класс для виджетов
#include <QVBoxLayout>  // Вертикальное расположение элементов
#include <QLabel>  // Текстовая метка для заголовка
#include <QScrollArea> // Прокручиваемая область для карточек
#include <QDragEnterEvent>
#include <QDropEvent>
#include "taskcard.h"
#include "../models/task.h"

class ColumnWidget : public QWidget {
    Q_OBJECT  // Макрос для сигналов/слотов Qt

public:
    // Конструктор столбца
    explicit ColumnWidget(const QString& title, const QString& color, TaskStatus status, QWidget* parent = nullptr);

    // Управление карточками
    void addTaskCard(TaskCard* card);
    void clearTasks();
    TaskStatus getStatus() const { return columnStatus; }

signals:
    // Сигнал при перетаскивании задачи в столбец
    void taskDropped(int taskId, TaskStatus newStatus);

protected:
    // Обработка событий drag-and-drop
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QString columnTitle;
    TaskStatus columnStatus;
    QLabel* titleLabel; // Виджет для отображения названия
    QVBoxLayout* cardsLayout; // Layout для расположения карточек (вертикальный)
    QWidget* cardsContainer;
    QScrollArea* scrollArea; // Прокручиваемая область для карточек
};

#endif // COLUMNWIDGET_H
