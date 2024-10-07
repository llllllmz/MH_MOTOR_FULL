#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include "sendPackThread.h"
#include "receivePackThread.h"
#include <QProgressBar>
#include <QLabel>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void monitor(); //开始监控
    void updataData(char cmd, char param, int data);
    void updatePressure(QProgressBar* bar, QLabel* label, int value, bool warning);
    void updateWarningLabel();
    void updateTime();

private slots:
    void on_pushButton_add_clicked();

    void on_pushButtonSub_clicked();
    void setNeedleValue(double value);

private:
    Ui::MainWindow *ui;

    SendPackThread *senderThread;
    ReceivePackThread *receiveThread;
    QSerialPort* serialPort;       //串口
    QGraphicsProxyWidget* m_needle;
    bool pressureStatus[6] = {false,};
    QPixmap pixNormalStaus;
    QPixmap pixWarningStatus;
    int pressThreshold; // 大于该值就要告警
};
#endif // MAINWINDOW_H
