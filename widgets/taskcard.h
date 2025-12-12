#ifndef TASKCARD_H
#define TASKCARD_H

#include <QWidget> // Базовый класс для всех виджетов
#include <QLabel> // Для отображения текста
#include <QPushButton>
#include <QVBoxLayout>
#include <QMouseEvent> // Обработка кликов и перемещения мыши
#include <QDrag>
#include <QPropertyAnimation> // Анимация свойств
#include <QGraphicsOpacityEffect> // Эффект прозрачности для виджетов
#include <QTimer>
#include <QPainter>
#include <QVector> // Контейнер для хранения частиц
#include "../models/task.h"
#include "../models/board.h"

struct Particle { // Структура для частиц визуальных эффектов
    QPointF position;
    qreal opacity;
    qreal size;
    qreal velocity;
    QColor color;
};

class TaskCard : public QWidget { //Визуальная карточка задачи
    Q_OBJECT
    // glowIntensity - интенсивность свечения карточки
    Q_PROPERTY(qreal glowIntensity READ glowIntensity WRITE setGlowIntensity) 

public:
    // task - указатель на объект задачи
    // board - указатель на доску
    explicit TaskCard(Task* task, Board* board, QWidget* parent = nullptr);

    Task* getTask() const { return task; }
    void updateDisplay();

    qreal glowIntensity() const { return m_glowIntensity; }
    void setGlowIntensity(qreal intensity);

signals:
    void editClicked(Task* task);
    void deleteClicked(Task* task);
    void statusChangeClicked(Task* task);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override; // Метод отрисовки виджета

private slots:
    void updateParticles(); // Слот для обновления анимации частиц

private:
    Task* task; // Задача, которую представляет эта карточка
    Board* board; // Доска, к которой относится задача
    QLabel* titleLabel;
    QLabel* descLabel;
    QLabel* assignedLabel;
    QLabel* deadlineLabel;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QPushButton* statusBtn;

    QPoint dragStartPosition; // Позиция, с которой началось перетаскивание
    QPropertyAnimation* glowAnimation; // Аниматор для эффекта свечения при приближении дедлайна
    QTimer* particleTimer;
    QVector<Particle> particles;
    qreal m_glowIntensity;
    int particleIntensity; // 0 = нет, 1-4 = уровень интенсивности
    int lastDaysUntilDeadline; // Отслеживание изменений дедлайна для предотвращения лишней регенерации частиц

    void setupUI(); // Настройка пользовательского интерфейса
    void updateCardColor();
    void updateDeadlineText();
    void startGlowAnimation(int daysUntilDeadline);
    void stopGlowAnimation();
    void startParticles(int intensity);
    void stopParticles();
    void generateParticles();
    QString getStatusButtonText() const; // Получение текста для кнопки статуса
    QString getDeveloperName() const; // Получение имени разработчика для отображения
};

#endif // TASKCARD_H
