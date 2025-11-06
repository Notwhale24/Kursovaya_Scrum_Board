#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class StartScreen : public QWidget {
    Q_OBJECT

public:
    explicit StartScreen(QWidget* parent = nullptr);

signals:
    void newBoardRequested();
    void loadBoardRequested(const QString& filePath);
    void exitRequested();

private slots:
    void onNewBoardClicked();
    void onBrowseClicked();
    void onExitClicked();

private:
    QPushButton* newBoardBtn;
    QPushButton* browseBtn;
    QPushButton* exitBtn;

    void setupUI();
    void openFileDialog();
    void styleButton(QPushButton* btn, const QString& color);
};

#endif // STARTSCREEN_H
