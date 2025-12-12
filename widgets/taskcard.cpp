#include "taskcard.h"
#include <QHBoxLayout>
#include <QFont> // Работа со шрифтами
#include <QMimeData> // Данные для drag-and-drop
#include <QApplication> // Для определения дистанции начала перетаскивания
#include <QDrag>
#include <QRandomGenerator>
#include <QDebug> // Отладочный вывод
#include <QPainterPath> // Сложные пути для рисования

TaskCard::TaskCard(Task* task, Board* board, QWidget* parent) // Конструктор карточки задачи
    : QWidget(parent), task(task), board(board), m_glowIntensity(0.0), particleIntensity(0), lastDaysUntilDeadline(999) {

    // КРИТИЧЕСКИ ВАЖНО для работы border-radius на QWidget!
    setAttribute(Qt::WA_StyledBackground, true);

    // Создание анимации свечения
    glowAnimation = new QPropertyAnimation(this, "glowIntensity");
    glowAnimation->setDuration(1500); // Длительность одного цикла: 1.5 секунды
    glowAnimation->setLoopCount(-1); // Бесконечное повторение
    glowAnimation->setEasingCurve(QEasingCurve::InOutSine);

    particleTimer = new QTimer(this);
    connect(particleTimer, &QTimer::timeout, this, &TaskCard::updateParticles);

    setupUI(); // Настройка интерфейса
    updateDisplay();
    setAcceptDrops(false); // Карточка не принимает другие карточки
}

// Настройка пользовательского интерфейса карточки
void TaskCard::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(5);

    // Название задачи
    titleLabel = new QLabel(this);
    titleLabel->setAutoFillBackground(false); // Прозрачный фон
    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPointSize(11);
    titleLabel->setFont(titleFont);
    titleLabel->setWordWrap(true); // Перенос длинных названий на новую строку
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
    assignedFont.setItalic(true); // Курсив для имени разработчика
    assignedLabel->setFont(assignedFont);
    mainLayout->addWidget(assignedLabel);

    // Спейсер чтобы кнопки были снизу карточки
    mainLayout->addStretch();

    // Кнопки
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(5);

    // Кнопка изменения статуса
    statusBtn = new QPushButton(this);
    statusBtn->setStyleSheet("background-color: #4CAF50; color: white; border: none; padding: 5px;");
    connect(statusBtn, &QPushButton::clicked, this, [this]() {
        emit statusChangeClicked(task);
    });
    btnLayout->addWidget(statusBtn);

    // Кнопка редактирования
    editBtn = new QPushButton("✏️", this);
    editBtn->setMaximumWidth(30);
    connect(editBtn, &QPushButton::clicked, this, [this]() {
        emit editClicked(task);
    });
    btnLayout->addWidget(editBtn);

    // Кнопка удаления
    deleteBtn = new QPushButton("🗑️", this);
    deleteBtn->setMaximumWidth(30);
    deleteBtn->setStyleSheet("background-color: #f44336; color: white;");
    connect(deleteBtn, &QPushButton::clicked, this, [this]() {
        emit deleteClicked(task);
    });
    btnLayout->addWidget(deleteBtn);

    mainLayout->addLayout(btnLayout); // Добавление макета с кнопками в основной макет

    // Адаптивные размеры карточки
    setMinimumWidth(200);
    setMaximumWidth(400);
    setMinimumHeight(180);
    setMaximumHeight(220);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

// Интенсивности свечения
void TaskCard::setGlowIntensity(qreal intensity) {
    m_glowIntensity = intensity;
    updateCardColor();
}

// Обновление отображения всех данных карточки
void TaskCard::updateDisplay() {
    if (!task) return;  // Задача существует

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

    // Кнопка статуса и цвет карточки
    statusBtn->setText(getStatusButtonText());
    updateCardColor();
}

void TaskCard::updateCardColor() {
    if (!task) return;

    QString bgColor = "#FFF3E0";
    QString textColor = "#333";
    QString borderColor = "#FFE0B2";
    int days = 999;

    // Если есть дедлайн И задача не завершена - анализируем срочность
    if (task->hasDeadline() && task->getStatus() != TaskStatus::Done) {
        days = task->daysUntilDeadline();
        int targetIntensity = 0; // Интенсивность эффектов (0-4)

        if (task->isOverdue()) {
            // Уровень 4: просроченные задачи
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
            // Уровень 3: дедлайн сегодня
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
            // Уровень 2: дедлайн завтра
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
            // Уровень 1: дедлайн через 2-3 дня
            // Желтоватый
            int intensity = (int)(m_glowIntensity * 30);
            bgColor = QString("rgb(255,%1,210)").arg(240 + intensity / 3);
            textColor = "#F57C00";
            borderColor = "#FFE082";
            startGlowAnimation(days);
            targetIntensity = 1;

        } else {
            // Уровень 0: нет срочности
            // Персиковый базовый
            bgColor = "#FFF3E0";
            textColor = "#333";
            borderColor = "#FFE0B2";
            stopGlowAnimation();
            targetIntensity = 0;
        }

        // Обновляем частицы только если изменились дни до дедлайна
        if (days != lastDaysUntilDeadline) {
            lastDaysUntilDeadline = days; // Сохраняем новое значение
            if (targetIntensity > 0) {
                startParticles(targetIntensity); // Запускаем частицы
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

    // Применение стилей CSS ко всей карточке и ее элементам
    setStyleSheet(QString("TaskCard { "
                          "background: %1; "
                          "color: %2; " // Цвет текста
                          "border: 2px solid %3; "
                          "border-radius: 12px; " // Скругленные углы
                          "} "
                          "QLabel { "
                          "background: transparent; " // Прозрачный фон у меток
                          "}")
                      .arg(bgColor, textColor, borderColor));
}

// Запуск анимации свечения с параметрами в зависимости от срочности
void TaskCard::startGlowAnimation(int daysUntilDeadline) {
    if (glowAnimation->state() == QAbstractAnimation::Running) {
        return;
    }

    // Просроченные задачи: медленное пульсирующее свечение
    if (daysUntilDeadline < 0) {
        glowAnimation->setDuration(2000); // 2 секунды на цикл
        glowAnimation->setKeyValueAt(0, 0.3); // Начальная интенсивность: 30%
        glowAnimation->setKeyValueAt(0.5, 1.0); // Пик: 100%
        glowAnimation->setKeyValueAt(1, 0.3); // Конец: 30%
    // Дедлайн сегодня: быстрое пульсирование    
    } else if (daysUntilDeadline == 0) {
        glowAnimation->setDuration(800);
        glowAnimation->setKeyValueAt(0, 0.5);
        glowAnimation->setKeyValueAt(0.5, 1.0);
        glowAnimation->setKeyValueAt(1, 0.5);
     // Дедлайн завтра: средняя скорость    
    } else if (daysUntilDeadline == 1) {
        glowAnimation->setDuration(1200);
        glowAnimation->setKeyValueAt(0, 0.2);
        glowAnimation->setKeyValueAt(0.5, 0.8);
        glowAnimation->setKeyValueAt(1, 0.2);
    // Дедлайн через 2-3 дня: плавное появление/исчезновение    
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

// Запуск эффекта частиц с заданной интенсивностью
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
    update(); // Перерисовка виджета
}

void TaskCard::generateParticles() {
    particles.clear();

    int count = particleIntensity * 40;

    // Ограничиваем область генерации нижними 30% карточки
    qreal particleAreaHeight = height() * 0.3;
    qreal particleAreaTop = height() - particleAreaHeight; // Начало области

    for (int i = 0; i < count; i++) {
        Particle p;

        // Генерируем позицию с учетом скругленных углов
        QPointF position;
        bool validPosition = false;
        int attempts = 0;

        while (!validPosition && attempts < 10) { // Пытаемся найти валидную позицию
            position = QPointF(
                QRandomGenerator::global()->generateDouble() * width(), // Случайный X
                particleAreaTop + QRandomGenerator::global()->generateDouble() * particleAreaHeight // Случайный Y
                );

            // Проверяем, не попадает ли частица в скругленные углы
            qreal cornerRadius = 12.0;

            // Расстояние от частицы до каждого угла
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

        // Установка параметров частицы
        p.position = position;
        p.opacity = 0.4 + QRandomGenerator::global()->generateDouble() * 0.6; // Прозрачность
        p.size = 3.0 + QRandomGenerator::global()->generateDouble() * 4.0;
        p.velocity = 0.3 + QRandomGenerator::global()->generateDouble() * 1.2;

        // Настройка цвета частиц в зависимости от интенсивности
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

        particles.append(p); // Добавление частицы в массив
    }
}

// Обновление анимации части
void TaskCard::updateParticles() {
    for (Particle& p : particles) {
        p.position.setY(p.position.y() - p.velocity);
        p.position.setX(p.position.x() + (QRandomGenerator::global()->generateDouble() - 0.5) * 0.5);
        p.opacity -= 0.015; // Постепенное исчезновение

        // Если частица вышла за пределы видимости или исчезла - регенерируем ее
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

            p.position = QPointF(x, height() + 5); // Сразу под карточкой
            p.opacity = 0.6 + QRandomGenerator::global()->generateDouble() * 0.4;
            p.velocity = 0.3 + QRandomGenerator::global()->generateDouble() * 1.2;
        }
    }

    update(); // Перерисовка виджета
}

// Метод отрисовки виджета
void TaskCard::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    if (particles.isEmpty()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // Сглаживание для плавных краев

    // Обрезаем рисование по скругленным углам карточки
    QPainterPath path;
    path.addRoundedRect(rect(), 12, 12); // Прямоугольник со скруглением 12px
    painter.setClipPath(path); // Установка области отсечения

    // Рисование каждой частицы
    for (const Particle& p : particles) {
        QColor color = p.color;
        color.setAlphaF(p.opacity);

        // Создание радиального градиента для эффекта размытости
        QRadialGradient gradient(p.position, p.size * 1.5); // Центр и радиус градиента
        gradient.setColorAt(0, color); // Центр - полный цвет
        QColor fadeColor = color;
        fadeColor.setAlphaF(p.opacity * 0.3);
        gradient.setColorAt(0.6, fadeColor); // Средняя часть - полупрозрачная
        fadeColor.setAlphaF(0);
        gradient.setColorAt(1, fadeColor);

        painter.setBrush(gradient); // Установка градиента как кисти
        painter.setPen(Qt::NoPen); // Без контура
        painter.drawEllipse(p.position, p.size * 1.5, p.size * 1.5);
    }
}

// Получение текста для кнопки статуса
QString TaskCard::getStatusButtonText() const {
    if (!task) return "";

    // Преобразование статуса задачи в текст кнопки
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

// Получение имени разработчика для отображения
QString TaskCard::getDeveloperName() const {
    if (!task || !board || !task->isAssigned()) {
        return "Не назначена";
    }

    // Получение объекта разработчика по ID из доски
    Developer* dev = board->getDeveloper(task->getAssignedDeveloperId());
    if (dev) {
        return dev->getName();
    }

    return QString("ID: %1").arg(task->getAssignedDeveloperId());
}

// Обработка нажатия кнопки мыши
void TaskCard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
    }
    QWidget::mousePressEvent(event);
}

// Обработка движения мыши
void TaskCard::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }

    // Проверяем, достаточно ли мышь переместилась для начала перетаскивания
    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    // Создание объекта перетаскивания
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;

    // В качестве данных передаем ID задачи
    mimeData->setText(QString::number(task->getId()));
    drag->setMimeData(mimeData);

    // Создание миниатюры карточки для перетаскивания
    QPixmap pixmap(size());
    render(&pixmap); // Отрисовка карточки в пиксмап
    drag->setPixmap(pixmap); // Установка миниатюры
    drag->setHotSpot(event->pos()); // Точка захвата

    drag->exec(Qt::MoveAction);
}
