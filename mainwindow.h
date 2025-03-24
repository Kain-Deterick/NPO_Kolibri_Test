#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QTimer>
#include "FileProcessor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartButtonClicked();
    void onTimerTimeout();
    void updateProgress(int value); // Слот для обновления прогресса
    void updateStatus(const QString &status); // Слот для обновления статуса

private:
    Ui::MainWindow *ui;
    FileProcessor *fileProcessor;
    QThread *processorThread;
    QTimer *timer = new QTimer(this);
};
#endif // MAINWINDOW_H
