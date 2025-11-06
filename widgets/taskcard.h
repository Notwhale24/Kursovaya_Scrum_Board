#ifndef TASKCARD_H
#define TASKCARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDrag>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QPainter>
#include <QVector>
#include "../models/task.h"
#include "../models/board.h"

struct Particle {
    QPointF position;
    qreal opacity;
    qreal size;
    qreal velocity;
    QColor color;
};

class TaskCard : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal glowIntensity READ glowIntensity WRITE setGlowIntensity)

public:
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
    void paintEvent(QPaintEvent* event) override;

private slots:
    void updateParticles();

private:
    Task* task;
    Board* board;
    QLabel* titleLabel;
    QLabel* descLabel;
    QLabel* assignedLabel;
    QLabel* deadlineLabel;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QPushButton* statusBtn;

    QPoint dragStartPosition;
    QPropertyAnimation* glowAnimation;
    QTimer* particleTimer;
    QVector<Particle> particles;
    qreal m_glowIntensity;
    int particleIntensity; // 0 = нет, 1-4 = уровень интенсивности
    int lastDaysUntilDeadline; // Отслеживание изменений дедлайна для предотвращения лишней регенерации частиц

    void setupUI();
    void updateCardColor();
    void updateDeadlineText();
    void startGlowAnimation(int daysUntilDeadline);
    void stopGlowAnimation();
    void startParticles(int intensity);
    void stopParticles();
    void generateParticles();
    QString getStatusButtonText() const;
    QString getDeveloperName() const;
};

#endif // TASKCARD_H
