#include "startscreen.h"
#include <QFileDialog>
#include <QKeySequence> // Для работы с горячими клавишами

// Вспомогательная функция для изменения яркости цвета
// static делает функцию видимой только в этом файле компиляции
static QString adjustColor(const QString& color, int adjustment) { 
    QColor c(color);
    // Изменяем компоненты RGB
    int r = qBound(0, c.red() + adjustment, 255);
    int g = qBound(0, c.green() + adjustment, 255);
    int b = qBound(0, c.blue() + adjustment, 255);
    return QString("rgb(%1,%2,%3)").arg(r).arg(g).arg(b);
}

StartScreen::StartScreen(QWidget* parent) : QWidget(parent) {
    setupUI(); // Метод настройки пользовательского интерфейса
}

void StartScreen::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(50, 50, 50, 50); // Устанавливаем поля

    // Заголовок
    QLabel* titleLabel = new QLabel("Скрам Доска", this);
    QFont titleFont;
    titleFont.setPointSize(32);
    titleFont.setBold(true); // Делаем шрифт жирным
    titleLabel->setFont(titleFont); // Применяем шрифт к метке
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #2C3E50; margin-bottom: 30px;");
    mainLayout->addWidget(titleLabel);

    // Контейнер для кнопок (фиксированная ширина для минимализма)
    QWidget* buttonsContainer = new QWidget(this);
    buttonsContainer->setMaximumWidth(400);
    QVBoxLayout* buttonsLayout = new QVBoxLayout(buttonsContainer); // Создаем вертикальный layout внутри контейнера
    buttonsLayout->setSpacing(15);

    // Кнопка "Новая доска" (Ctrl+N)
    newBoardBtn = new QPushButton("Новая доска (Ctrl+N)", buttonsContainer);
    newBoardBtn->setMinimumHeight(50);
    newBoardBtn->setShortcut(QKeySequence("Ctrl+N"));
    styleButton(newBoardBtn, "#4CAF50");
    connect(newBoardBtn, &QPushButton::clicked, this, &StartScreen::onNewBoardClicked); // Подключаем сигнал clicked
    buttonsLayout->addWidget(newBoardBtn);

    // Кнопка "Обзор..." для выбора файла (Ctrl+O)
    browseBtn = new QPushButton("Обзор... (Ctrl+O)", buttonsContainer);
    browseBtn->setMinimumHeight(50);
    browseBtn->setShortcut(QKeySequence("Ctrl+O"));
    styleButton(browseBtn, "#2196F3");
    connect(browseBtn, &QPushButton::clicked, this, &StartScreen::onBrowseClicked);
    buttonsLayout->addWidget(browseBtn);

    // Кнопка "Выйти" (Ctrl+Q)
    exitBtn = new QPushButton("Выйти (Ctrl+Q)", buttonsContainer);
    exitBtn->setMinimumHeight(50);
    exitBtn->setShortcut(QKeySequence("Ctrl+Q"));
    styleButton(exitBtn, "#F44336");
    connect(exitBtn, &QPushButton::clicked, this, &StartScreen::onExitClicked);
    buttonsLayout->addWidget(exitBtn);

    mainLayout->addWidget(buttonsContainer); // Добавляем контейнер с кнопками в главный layout
    mainLayout->addStretch(); // Добавляем растягиваемое пространство для выравнивания кнопок по центру

    // Общий стиль фона - современный градиент
    setStyleSheet(
        "StartScreen {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "       stop:0 #E3F2FD, "
        "       stop:0.5 #F5F5F5, "
        "       stop:1 #FFF8E1);"
        "}"
    );
}

// Метод для стилизации кнопок
void StartScreen::styleButton(QPushButton* btn, const QString& color) {
    btn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: %1;"  // Основной цвет
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;" // Закругленные углы
        "   font-size: 16px;" // Размер шрифта
        "   font-weight: bold;"
        "   padding: 10px;" // Внутренние отступы
        "}"
        // Стиль для состояния при наведении курсора
        "QPushButton:hover {"
        "   background-color: %2;"
        "}"
        // Стиль для состояния при нажатии
        "QPushButton:pressed {"
        "   background-color: %3;"
        "}"
    ).arg(color)
     .arg(adjustColor(color, 20))
     .arg(adjustColor(color, -20)));
}

void StartScreen::onNewBoardClicked() {
    emit newBoardRequested();
}

void StartScreen::onBrowseClicked() {
    openFileDialog();
}

void StartScreen::openFileDialog() {
    // Используем текущую директорию вместо Documents
    QString currentPath = QDir::currentPath();
    QString filePath = QFileDialog::getOpenFileName(
        this, // Родительское окно
        "Открыть доску",
        currentPath, // Начальная директория
        "JSON файлы (*.json);;Все файлы (*.*)"
    );

     // Если пользователь выбрал файл
    if (!filePath.isEmpty()) {
        emit loadBoardRequested(filePath);
    }
}

void StartScreen::onExitClicked() {
    emit exitRequested();
}
