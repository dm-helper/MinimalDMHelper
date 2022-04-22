#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class PublishWindow;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showPublishWindow(bool clicked);

private:
    Ui::MainWindow *ui;

    PublishWindow* _pubWindow;
};

#endif // MAINWINDOW_H
