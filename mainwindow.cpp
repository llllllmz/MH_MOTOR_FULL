#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "protocal.h"
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QDateTime>
#include "needle.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pressThreshold = 1150;//960;

    ui->pushButton_add->setStyleSheet(
       "QPushButton{"
            "background-image:url(:res/add_normal.png);"
            "padding:0;"
            "border:0;"
            "background-position:Center;}"
       "QPushButton:pressed{"
            "background-image:url(:res/add_press.png);}"
    );
    ui->pushButtonSub->setStyleSheet(
       "QPushButton{"
            "background-image:url(:res/sub_normal.png);"
            "padding:0;"
            "border:0;"
            "background-position:Center;}"
       "QPushButton:pressed{"
            "background-image:url(:res/sub_press.png);}"
    );

    //创建一个图像场景
    QGraphicsScene* scene = new QGraphicsScene();
    //设置场景的位置区域
    scene->setSceneRect(ui->graphicsView->rect());

    //把指针窗口添加到场景中, 并返回用来操控指针窗口的“代理”
    m_needle = scene->addWidget(new Needle);

    //设置指针窗口在场景中的位置
    m_needle->setPos(100,  103);
    m_needle->setTransformOriginPoint(4, 0); //旋转原点在指针图片内部的(4,0)位置

    //设置图像视图显示的场景
    ui->graphicsView->setScene(scene);

    pixNormalStaus =  QPixmap(":/res/normal.png");
    pixWarningStatus =  QPixmap(":/res/warning.png");

    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);   //1秒刷新一次
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    timer->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::monitor()
{
    serialPort = new QSerialPort;
    //初始化串口
    serialPort->setPortName("COM2");
    if(!serialPort->open(QIODevice::ReadWrite)) {
        QMessageBox::critical(nullptr,
               "串口打开错误",
               "该串口不存在或已被占用！",
               QMessageBox::Ok);
        //qApp->exit(0); 无效，因为在时间循环还没有启动之前，该方式无效
        //0ms之后，调用槽函数 qApp的quit()，等真正调用这个槽函数时，事件循环已经启动了
        QTimer::singleShot(0, qApp, SLOT(quit()));
    }
    serialPort->setBaudRate(19200);                 //波特率，每秒可传输多少二进制位
    serialPort->setDataBits(QSerialPort::Data8);    //数据位    //每个输出字节由几个二进制位表示
    serialPort->setParity(QSerialPort::NoParity);   //校验位
    serialPort->setStopBits(QSerialPort::OneStop);  //停止位     //表明这一帧数据结束
    serialPort->setFlowControl(QSerialPort::NoFlowControl);     //流控制

    senderThread = new SendPackThread(serialPort);
    receiveThread = new ReceivePackThread(serialPort);

    //添加信号槽
    connect(receiveThread, &ReceivePackThread::receiveResponsePack,
            this, &MainWindow::updataData);

    senderThread->start();
    receiveThread->start();

    //手动发送加速按钮的单击信号
    emit ui->pushButton_add->clicked();
}

void MainWindow::updataData(char cmd, char param, int data)
{
    switch (cmd) {
    case ZHU_JI_WEN_DU_RSP:
        ui->label_wen_du->setText(QString::number(data));
        break;
    case ZHOU_SU_DU_RSP:
        {
            QLabel* labels[2] = {
                ui->label_zhu_zhou,
                ui->label_fu_zhou
            };
            if (param != 1 && param != 2) break;
            labels[param-1]->setText(QString::number(data));
            break;
        }
    case YA_LI_CHANG_RSP:
        {
            QWidget* widgets[5][2] = {
                {ui->prgBar_1_chang, ui->chang1},
                {ui->prgBar_2_chang, ui->chang2},
                {ui->prgBar_3_chang, ui->chang3},
                {ui->prgBar_4_chang, ui->chang4},
                {ui->prgBar_5_chang, ui->chang5}
            };
            if (param < 1 || param > 5) break;
            bool warning = data >= pressThreshold;
            pressureStatus[param-1] = warning;
            updatePressure(
                 (QProgressBar*)widgets[param-1][0],
                 (QLabel*)widgets[param-1][1],
                 data, warning);
//            updatePressure(dynamic_cast<QProgressBar*>(widgets[param-1][0]),
//                    dynamic_cast<QLabel*>(widgets[param-1][1]), data, 960);
            break;
        }
    case GANG_YOU_LIANG_RSP:
        {
            QLabel* labels[] = {
                ui->label_1_gang,
                ui->label_2_gang,
                ui->label_3_gang
            };
            if (param < 1 || param > 3) break;
            labels[param-1]->setText(QString::number(data)+" L");
            break;
        }
    case JI_XIE_BI_RSP:
        {
            bool warning = data >= pressThreshold;
            pressureStatus[5] = warning;
            updatePressure(ui->prgBar_ji_xie_bi, ui->lab_ji_xie_bi, data, warning);
            break;
        }
    case DONG_LI_GAN_SU_DU_RSP:
        setNeedleValue(data);
        break;
    }

    updateWarningLabel();
}

void MainWindow::on_pushButton_add_clicked()
{
    char pack[9] = {0,};
    senderThread->makePack(DONG_LI_GAN_SU_DU_REQ, 0xF0, 0, pack);
    int ret = serialPort->write(pack, sizeof(pack));
    qDebug() << "send: " << ret << " bytes.";
}

void MainWindow::on_pushButtonSub_clicked()
{
    char pack[9] = {0,};
    senderThread->makePack(DONG_LI_GAN_SU_DU_REQ, 0x0F, 0, pack);
    int ret = serialPort->write(pack, sizeof(pack));
    qDebug() << "send: " << ret << " bytes.";
}

void MainWindow::setNeedleValue(double value)   //码表值：0-120   度数范围：54-310
{
    if(value < 0) value = 0;
    else if(value > 120) value = 120;

    double rotateAngle = (double)((310.0 - 54.0) / 120) * value + 54.00;

    //微调
    if(value < 40) rotateAngle += 1;
    else if(40 < value) rotateAngle -= 2;

    m_needle->setRotation(rotateAngle);
}

void MainWindow::updatePressure(QProgressBar* bar, QLabel* label, int value, bool warning)
{
    bar->setValue(value);
    label->setText(QString::number(value)+" Pa");
    if(warning) {
        bar->setStyleSheet("QProgressBar::chunk {background-color: #EB3324;}");
    } else {
        bar->setStyleSheet("QProgressBar::chunk {background-color: #00C258;}");
    }
}

void MainWindow::updateWarningLabel()
{
    if (pressureStatus[0] || pressureStatus[1] || pressureStatus[2] ||
        pressureStatus[3] || pressureStatus[4] || pressureStatus[5]) {
        ui->label_status->setPixmap(pixWarningStatus);
    } else {
        ui->label_status->setPixmap(pixNormalStaus);
    }
}

void MainWindow::updateTime()
{
    QString dataStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm ss");
    ui->label_time->setText(dataStr);
}
