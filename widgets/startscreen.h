#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QWidget> // Базовый класс для виджетов
#include <QPushButton>
#include <QVBoxLayout> // Вертикальное расположение элементов
#include <QLabel> // Текстовая метка

class StartScreen : public QWidget {
    Q_OBJECT // Макрос для сигналов/слотов Qt

public:
// Конструктор класса
    explicit StartScreen(QWidget* parent = nullptr);  // explicit запрещает неявное преобразование типов

signals: // Испускаются при наступлении события
    void newBoardRequested();
    void loadBoardRequested(const QString& filePath);
    void exitRequested();

private slots: // Методы, которые реагируют на сигналы
    void onNewBoardClicked();
    void onBrowseClicked();
    void onExitClicked();

private: // Доступны только внутри класса
    QPushButton* newBoardBtn;
    QPushButton* browseBtn;
    QPushButton* exitBtn;

    void setupUI();  // Создает кнопки, располагает их, устанавливает текст
    void openFileDialog();
    void styleButton(QPushButton* btn, const QString& color);
};

#endif // STARTSCREEN_H
