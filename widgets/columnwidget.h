#ifndef COLUMNWIDGET_H
#define COLUMNWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "taskcard.h"
#include "../models/task.h"

class ColumnWidget : public QWidget {
    Q_OBJECT

public:
    explicit ColumnWidget(const QString& title, const QString& color, TaskStatus status, QWidget* parent = nullptr);

    void addTaskCard(TaskCard* card);
    void clearTasks();
    TaskStatus getStatus() const { return columnStatus; }

signals:
    void taskDropped(int taskId, TaskStatus newStatus);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QString columnTitle;
    TaskStatus columnStatus;
    QLabel* titleLabel;
    QVBoxLayout* cardsLayout;
    QWidget* cardsContainer;
    QScrollArea* scrollArea;
};

#endif // COLUMNWIDGET_H
