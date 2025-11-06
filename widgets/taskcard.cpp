#include "taskcard.h"
#include <QHBoxLayout>
#include <QFont>
#include <QMimeData>
#include <QApplication>
#include <QDrag>
#include <QRandomGenerator>
#include <QDebug>
#include <QPainterPath>

TaskCard::TaskCard(Task* task, Board* board, QWidget* parent)
    : QWidget(parent), task(task), board(board), m_glowIntensity(0.0), particleIntensity(0), lastDaysUntilDeadline(999) {

    // КРИТИЧЕСКИ ВАЖНО для работы border-radius на QWidget!
    setAttribute(Qt::WA_StyledBackground, true);

    glowAnimation = new QPropertyAnimation(this, "glowIntensity");
    glowAnimation->setDuration(1500);
    glowAnimation->setLoopCount(-1);
    glowAnimation->setEasingCurve(QEasingCurve::InOutSine);

    particleTimer = new QTimer(this);
    connect(particleTimer, &QTimer::timeout, this, &TaskCard::updateParticles);

    setupUI();
    updateDisplay();
    setAcceptDrops(false);
}

void TaskCard::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(5);

    // Название задачи
    titleLabel = new QLabel(this);
    titleLabel->setAutoFillBackground(false);
    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPointSize(11);
    titleLabel->setFont(titleFont);
    titleLabel->setWordWrap(true);
    titleLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    mainLayout->addWidget(titleLabel);

    // Описание задачи (фиксированная высота)
    descLabel = new QLabel(this);
    descLabel->setAutoFillBackground(false);
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    descLabel->setFixedHeight(40);
    mainLayout->addWidget(descLabel);

    // Дедлайн
    deadlineLabel = new QLabel(this);
    deadlineLabel->setAutoFillBackground(false);
    deadlineLabel->setAlignment(Qt::AlignLeft);
    QFont deadlineFont;
    deadlineFont.setBold(true);
    deadlineLabel->setFont(deadlineFont);
    mainLayout->addWidget(deadlineLabel);

    // Разработчик
    assignedLabel = new QLabel(this);
    assignedLabel->setAutoFillBackground(false);
    assignedLabel->setAlignment(Qt::AlignLeft);
    QFont assignedFont;
    assignedFont.setItalic(true);
    assignedLabel->setFont(assignedFont);
    mainLayout->addWidget(assignedLabel);

    // Спейсер чтобы кнопки были внизу
    mainLayout->addStretch();

    // Кнопки
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(5);

    statusBtn = new QPushButton(this);
    statusBtn->setStyleSheet("background-color: #4CAF50; color: white; border: none; padding: 5px;");
    connect(statusBtn, &QPushButton::clicked, this, [this]() {
        emit statusChangeClicked(task);
    });
    btnLayout->addWidget(statusBtn);

    editBtn = new QPushButton("✏️", this);
    editBtn->setMaximumWidth(30);
    connect(editBtn, &QPushButton::clicked, this, [this]() {
        emit editClicked(task);
    });
    btnLayout->addWidget(editBtn);

    deleteBtn = new QPushButton("🗑️", this);
    deleteBtn->setMaximumWidth(30);
    deleteBtn->setStyleSheet("background-color: #f44336; color: white;");
    connect(deleteBtn, &QPushButton::clicked, this, [this]() {
        emit deleteClicked(task);
    });
    btnLayout->addWidget(deleteBtn);

    mainLayout->addLayout(btnLayout);

    // Адаптивные размеры карточки
    setMinimumWidth(200);
    setMaximumWidth(400);
    setMinimumHeight(180);
    setMaximumHeight(220);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void TaskCard::setGlowIntensity(qreal intensity) {
    m_glowIntensity = intensity;
    updateCardColor();
}

void TaskCard::updateDisplay() {
    if (!task) return;

    // Название задачи
    titleLabel->setText(task->getTitle());

    // Описание (макс 120 символов)
    QString desc = task->getDescription();
    if (desc.isEmpty()) {
        descLabel->setText("Нет описания");
    } else {
        if (desc.length() > 120) {
            desc = desc.left(117) + "...";
        }
        descLabel->setText(desc);
    }

    // Дедлайн
    if (task->hasDeadline()) {
        int days = task->daysUntilDeadline();
        QString deadlineText;

        if (task->isOverdue()) {
            deadlineText = QString("🔥 Просрочено на %1 дн.").arg(-days);
        } else if (days == 0) {
            deadlineText = "🔥 Дедлайн сегодня!";
        } else if (days == 1) {
            deadlineText = "⚠️ Дедлайн завтра";
        } else if (days <= 3) {
            deadlineText = QString("📅 Дедлайн: %1 дн.").arg(days);
        } else {
            deadlineText = QString("📅 Дедлайн: %1 дн.").arg(days);
        }

        deadlineLabel->setText(deadlineText);
        deadlineLabel->show();
    } else {
        deadlineLabel->hide();
    }

    // Разработчик
    if (task->isAssigned()) {
        QString devName = getDeveloperName();
        assignedLabel->setText(QString("👤 %1").arg(devName));
        assignedLabel->show();
    } else {
        assignedLabel->setText("⚠️ Не назначена");
        assignedLabel->show();
    }

    statusBtn->setText(getStatusButtonText());
    updateCardColor();
}

void TaskCard::updateCardColor() {
    if (!task) return;

    QString bgColor = "#FFF3E0"; // Персиковый/бежевый базовый цвет
    QString textColor = "#333";
    QString borderColor = "#FFE0B2";
    int days = 999;

    if (task->hasDeadline() && task->getStatus() != TaskStatus::Done) {
        days = task->daysUntilDeadline();
        int targetIntensity = 0;

        if (task->isOverdue()) {
            // Красноватый с градиентом
            int intensity = (int)(m_glowIntensity * 80);
            bgColor = QString("qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                              "stop:0 rgb(%1,%2,%3), "
                              "stop:0.5 rgb(%4,%5,%6), "
                              "stop:1 rgb(%7,%8,%9))")
                          .arg(255).arg(200 + intensity / 2).arg(200 + intensity / 2)
                          .arg(255).arg(180 + intensity / 2).arg(180 + intensity / 2)
                          .arg(255).arg(200 + intensity / 2).arg(200 + intensity / 2);
            textColor = "#5D0000";
            borderColor = "#FF8A80";
            startGlowAnimation(days);
            targetIntensity = 4;

        } else if (days == 0) {
            // Оранжевый
            int intensity = (int)(m_glowIntensity * 60);
            bgColor = QString("qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                              "stop:0 rgb(255,%1,180), "
                              "stop:1 rgb(255,%2,200))")
                          .arg(220 + intensity / 2)
                          .arg(200 + intensity / 2);
            textColor = "#E65100";
            borderColor = "#FFAB91";
            startGlowAnimation(days);
            targetIntensity = 3;

        } else if (days == 1) {
            // Светло-оранжевый
            int intensity = (int)(m_glowIntensity * 40);
            bgColor = QString("qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                              "stop:0 rgb(255,%1,200), "
                              "stop:1 rgb(255,%2,210))")
                          .arg(230 + intensity / 2)
                          .arg(220 + intensity / 2);
            textColor = "#EF6C00";
            borderColor = "#FFCC80";
            startGlowAnimation(days);
            targetIntensity = 2;

        } else if (days <= 3) {
            // Желтоватый
            int intensity = (int)(m_glowIntensity * 30);
            bgColor = QString("rgb(255,%1,210)").arg(240 + intensity / 3);
            textColor = "#F57C00";
            borderColor = "#FFE082";
            startGlowAnimation(days);
            targetIntensity = 1;

        } else {
            // Персиковый базовый
            bgColor = "#FFF3E0";
            textColor = "#333";
            borderColor = "#FFE0B2";
            stopGlowAnimation();
            targetIntensity = 0;
        }

        // ВАЖНО: Обновляем частицы только если изменились дни до дедлайна
        if (days != lastDaysUntilDeadline) {
            lastDaysUntilDeadline = days;
            if (targetIntensity > 0) {
                startParticles(targetIntensity);
            } else {
                stopParticles();
            }
        }
    } else {
        lastDaysUntilDeadline = 999;
        stopGlowAnimation();
        stopParticles();
    }

    if (task->getStatus() == TaskStatus::Done) {
        // Зеленоватый для завершенных
        bgColor = "#E8F5E9";
        textColor = "#1B5E20";
        borderColor = "#A5D6A7";
        stopGlowAnimation();
        stopParticles();
    }

    setStyleSheet(QString("TaskCard { "
                          "background: %1; "
                          "color: %2; "
                          "border: 2px solid %3; "
                          "border-radius: 12px; "
                          "} "
                          "QLabel { "
                          "background: transparent; "
                          "}")
                      .arg(bgColor, textColor, borderColor));
}

void TaskCard::startGlowAnimation(int daysUntilDeadline) {
    if (glowAnimation->state() == QAbstractAnimation::Running) {
        return;
    }

    if (daysUntilDeadline < 0) {
        glowAnimation->setDuration(2000);
        glowAnimation->setKeyValueAt(0, 0.3);
        glowAnimation->setKeyValueAt(0.5, 1.0);
        glowAnimation->setKeyValueAt(1, 0.3);
    } else if (daysUntilDeadline == 0) {
        glowAnimation->setDuration(800);
        glowAnimation->setKeyValueAt(0, 0.5);
        glowAnimation->setKeyValueAt(0.5, 1.0);
        glowAnimation->setKeyValueAt(1, 0.5);
    } else if (daysUntilDeadline == 1) {
        glowAnimation->setDuration(1200);
        glowAnimation->setKeyValueAt(0, 0.2);
        glowAnimation->setKeyValueAt(0.5, 0.8);
        glowAnimation->setKeyValueAt(1, 0.2);
    } else {
        glowAnimation->setDuration(1500);
        glowAnimation->setKeyValueAt(0, 0.0);
        glowAnimation->setKeyValueAt(0.5, 0.5);
        glowAnimation->setKeyValueAt(1, 0.0);
    }

    glowAnimation->start();
}

void TaskCard::stopGlowAnimation() {
    if (glowAnimation->state() == QAbstractAnimation::Running) {
        glowAnimation->stop();
        m_glowIntensity = 0.0;
    }
}

void TaskCard::startParticles(int intensity) {
    if (particleIntensity == intensity && particleTimer->isActive()) {
        return;
    }

    particleIntensity = intensity;

    if (particleTimer->isActive()) {
        particleTimer->stop();
        particles.clear();
    }

    generateParticles();
    particleTimer->start(50);
}

void TaskCard::stopParticles() {
    particleIntensity = 0;
    particleTimer->stop();
    particles.clear();
    update();
}

void TaskCard::generateParticles() {
    particles.clear();

    int count = particleIntensity * 40;

    // ВАЖНО: Ограничиваем область генерации нижними 30% карточки
    qreal particleAreaHeight = height() * 0.3;
    qreal particleAreaTop = height() - particleAreaHeight;

    for (int i = 0; i < count; i++) {
        Particle p;

        // Генерируем позицию с учетом скругленных углов
        QPointF position;
        bool validPosition = false;
        int attempts = 0;

        while (!validPosition && attempts < 10) {
            position = QPointF(
                QRandomGenerator::global()->generateDouble() * width(),
                particleAreaTop + QRandomGenerator::global()->generateDouble() * particleAreaHeight
                );

            // Проверяем, не попадает ли частица в скругленные углы
            qreal cornerRadius = 12.0;
            qreal distToTopLeft = QLineF(position, QPointF(0, 0)).length();
            qreal distToTopRight = QLineF(position, QPointF(width(), 0)).length();
            qreal distToBottomLeft = QLineF(position, QPointF(0, height())).length();
            qreal distToBottomRight = QLineF(position, QPointF(width(), height())).length();

            // Если позиция в углу и слишком близко к краю - пропускаем
            bool inTopLeft = position.x() < cornerRadius && position.y() < cornerRadius && distToTopLeft < cornerRadius;
            bool inTopRight = position.x() > width() - cornerRadius && position.y() < cornerRadius && distToTopRight < cornerRadius;
            bool inBottomLeft = position.x() < cornerRadius && position.y() > height() - cornerRadius && distToBottomLeft < cornerRadius;
            bool inBottomRight = position.x() > width() - cornerRadius && position.y() > height() - cornerRadius && distToBottomRight < cornerRadius;

            if (!inTopLeft && !inTopRight && !inBottomLeft && !inBottomRight) {
                validPosition = true;
            }
            attempts++;
        }

        if (!validPosition) continue; // Пропускаем эту частицу

        p.position = position;
        p.opacity = 0.4 + QRandomGenerator::global()->generateDouble() * 0.6;
        p.size = 3.0 + QRandomGenerator::global()->generateDouble() * 4.0;
        p.velocity = 0.3 + QRandomGenerator::global()->generateDouble() * 1.2;

        if (particleIntensity == 4) {
            p.color = QColor(
                QRandomGenerator::global()->bounded(80, 120),
                QRandomGenerator::global()->bounded(20, 40),
                QRandomGenerator::global()->bounded(10, 25)
                );
            p.size = 4.0 + QRandomGenerator::global()->generateDouble() * 3.0;
            p.velocity = 0.1 + QRandomGenerator::global()->generateDouble() * 0.3;
            p.opacity = 0.3 + QRandomGenerator::global()->generateDouble() * 0.5;
        } else if (particleIntensity == 3) {
            p.color = QColor(255, QRandomGenerator::global()->bounded(40, 120), 0);
        } else if (particleIntensity == 2) {
            p.color = QColor(255, QRandomGenerator::global()->bounded(140, 255), QRandomGenerator::global()->bounded(0, 50));
        } else {
            p.color = QColor(255, QRandomGenerator::global()->bounded(120, 220), QRandomGenerator::global()->bounded(20, 60));
        }

        particles.append(p);
    }
}

void TaskCard::updateParticles() {
    for (Particle& p : particles) {
        p.position.setY(p.position.y() - p.velocity);
        p.position.setX(p.position.x() + (QRandomGenerator::global()->generateDouble() - 0.5) * 0.5);
        p.opacity -= 0.015;

        if (p.position.y() < -10 || p.opacity <= 0 || p.position.x() < 0 || p.position.x() > width()) {
            // Регенерируем частицу в безопасной зоне (не в углах)
            qreal x = QRandomGenerator::global()->generateDouble() * width();
            qreal cornerRadius = 12.0;

            // Избегаем нижних углов при регенерации
            if (x < cornerRadius) {
                x = cornerRadius + 5;
            } else if (x > width() - cornerRadius) {
                x = width() - cornerRadius - 5;
            }

            p.position = QPointF(x, height() + 5);
            p.opacity = 0.6 + QRandomGenerator::global()->generateDouble() * 0.4;
            p.velocity = 0.3 + QRandomGenerator::global()->generateDouble() * 1.2;
        }
    }

    update();
}

void TaskCard::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    if (particles.isEmpty()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Обрезаем рисование по скругленным углам карточки
    QPainterPath path;
    path.addRoundedRect(rect(), 12, 12);
    painter.setClipPath(path);

    for (const Particle& p : particles) {
        QColor color = p.color;
        color.setAlphaF(p.opacity);

        QRadialGradient gradient(p.position, p.size * 1.5);
        gradient.setColorAt(0, color);
        QColor fadeColor = color;
        fadeColor.setAlphaF(p.opacity * 0.3);
        gradient.setColorAt(0.6, fadeColor);
        fadeColor.setAlphaF(0);
        gradient.setColorAt(1, fadeColor);

        painter.setBrush(gradient);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(p.position, p.size * 1.5, p.size * 1.5);
    }
}

QString TaskCard::getStatusButtonText() const {
    if (!task) return "";

    switch (task->getStatus()) {
    case TaskStatus::Backlog:
        return "→ Назначить";
    case TaskStatus::Assigned:
        return "→ В работу";
    case TaskStatus::InProgress:
        return "→ На проверку";
    case TaskStatus::Review:
        return "→ Готово";
    case TaskStatus::Done:
        return "✓ Завершено";
    default:
        return "";
    }
}

QString TaskCard::getDeveloperName() const {
    if (!task || !board || !task->isAssigned()) {
        return "Не назначена";
    }

    Developer* dev = board->getDeveloper(task->getAssignedDeveloperId());
    if (dev) {
        return dev->getName();
    }

    return QString("ID: %1").arg(task->getAssignedDeveloperId());
}

void TaskCard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void TaskCard::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }

    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;

    mimeData->setText(QString::number(task->getId()));
    drag->setMimeData(mimeData);

    QPixmap pixmap(size());
    render(&pixmap);
    drag->setPixmap(pixmap);
    drag->setHotSpot(event->pos());

    drag->exec(Qt::MoveAction);
}
