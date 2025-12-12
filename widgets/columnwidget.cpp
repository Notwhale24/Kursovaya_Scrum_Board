#include "columnwidget.h"
#include <QFont> // Для настройки шрифта заголовка
#include <QMimeData> // Для работы с данными при перетаскивании (drag-and-drop)

ColumnWidget::ColumnWidget(const QString& title, const QString& color, TaskStatus status, QWidget* parent)
    : QWidget(parent), columnTitle(title), columnStatus(status) {

    setAcceptDrops(true); // Включаем поддержку drag-and-drop

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5); // Отступы со всех сторон
    mainLayout->setSpacing(5); // Расстояние между элементами

    // Заголовок колонки
    titleLabel = new QLabel(title, this);
    QFont titleFont;
    titleFont.setBold(true); // Жирный шрифт
    titleFont.setPointSize(14);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter); // Выравнивание по центру
    // Динамическое создание стиля
    titleLabel->setStyleSheet(QString("background-color: %1; color: black; padding: 10px; border-radius: 5px;").arg(color));
    mainLayout->addWidget(titleLabel);

    // Область прокрутки для карточек
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true); // Виджет внутри может менять размер
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #f5f5f5; }");

    cardsContainer = new QWidget();
    cardsLayout = new QVBoxLayout(cardsContainer);
    cardsLayout->setAlignment(Qt::AlignTop); // Карточки выравниваются вверху
    cardsLayout->setSpacing(10); // Расстояние между карточками
    cardsLayout->setContentsMargins(5, 5, 5, 5);

    scrollArea->setWidget(cardsContainer);
    mainLayout->addWidget(scrollArea);

    // Адаптивные столбцы - могут расширяться
    setMinimumWidth(250);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // Может растягиваться
    setStyleSheet("ColumnWidget { background-color: #e0e0e0; border-radius: 5px; }");
}

void ColumnWidget::addTaskCard(TaskCard* card) {
    cardsLayout->addWidget(card);
}

void ColumnWidget::clearTasks() {
    while (cardsLayout->count() > 0) { // Пока есть элементы в layout
        QLayoutItem* item = cardsLayout->takeAt(0); // Берем первый элемент
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
        // Текст содержит ID задачи
        int taskId = event->mimeData()->text().toInt();
        emit taskDropped(taskId, columnStatus); // Испускаем сигнал с ID задачи и новым статусом
        event->acceptProposedAction();
    }

    // Убираем подсветку
    setStyleSheet("ColumnWidget { background-color: #e0e0e0; border-radius: 5px; }");
}
