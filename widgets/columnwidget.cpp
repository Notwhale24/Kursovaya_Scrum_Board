#include "columnwidget.h"
#include <QFont>
#include <QMimeData>

ColumnWidget::ColumnWidget(const QString& title, const QString& color, TaskStatus status, QWidget* parent)
    : QWidget(parent), columnTitle(title), columnStatus(status) {

    setAcceptDrops(true);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // Заголовок колонки
    titleLabel = new QLabel(title, this);
    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("background-color: %1; color: black; padding: 10px; border-radius: 5px;").arg(color));
    mainLayout->addWidget(titleLabel);

    // Область прокрутки для карточек
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #f5f5f5; }");

    cardsContainer = new QWidget();
    cardsLayout = new QVBoxLayout(cardsContainer);
    cardsLayout->setAlignment(Qt::AlignTop);
    cardsLayout->setSpacing(10);
    cardsLayout->setContentsMargins(5, 5, 5, 5);

    scrollArea->setWidget(cardsContainer);
    mainLayout->addWidget(scrollArea);

    // Адаптивные столбцы - могут расширяться
    setMinimumWidth(250);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("ColumnWidget { background-color: #e0e0e0; border-radius: 5px; }");
}

void ColumnWidget::addTaskCard(TaskCard* card) {
    cardsLayout->addWidget(card);
}

void ColumnWidget::clearTasks() {
    while (cardsLayout->count() > 0) {
        QLayoutItem* item = cardsLayout->takeAt(0);
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void ColumnWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
        // Подсвечиваем колонку
        setStyleSheet("ColumnWidget { background-color: #d0d0d0; border: 2px dashed #4CAF50; border-radius: 5px; }");
    }
}

void ColumnWidget::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

void ColumnWidget::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasText()) {
        int taskId = event->mimeData()->text().toInt();
        emit taskDropped(taskId, columnStatus);
        event->acceptProposedAction();
    }

    // Убираем подсветку
    setStyleSheet("ColumnWidget { background-color: #e0e0e0; border-radius: 5px; }");
}
