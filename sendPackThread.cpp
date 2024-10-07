#include "sendPackThread.h"
#include "protocal.h"
#include <QDebug>
SendPackThread::SendPackThread(QSerialPort*port)
{
    serialPort = port;
}

void SendPackThread::run()
{
    static unsigned char cmdAndParam[][2] = {
        {ZHU_JI_WEN_DU_REQ, 0},
        {ZHOU_SU_DU_REQ, 1},
        {ZHOU_SU_DU_REQ, 2},
        {YA_LI_CHANG_REQ, 1},
        {YA_LI_CHANG_REQ, 2},
        {YA_LI_CHANG_REQ, 3},
        {YA_LI_CHANG_REQ, 4},
        {YA_LI_CHANG_REQ, 5},
        {GANG_YOU_LIANG_REQ, 1},
        {GANG_YOU_LIANG_REQ, 2},
        {GANG_YOU_LIANG_REQ, 3},
        {JI_XIE_BI_REQ, 0},
//        {DONG_LI_GAN_SU_DU_REQ, 0XF0},
//        {DONG_LI_GAN_SU_DU_REQ, 0X0F}
    };

    int cmdCount = sizeof(cmdAndParam)/sizeof(cmdAndParam[0]);
    char pack[9];
    while(1) {
        for(int i=0; i<cmdCount; i++) {
            makePack(cmdAndParam[i][0], cmdAndParam[i][1], 0, pack);

            int ret = serialPort->write(pack, sizeof(pack));
            qDebug() << "send " << ret << " bytes.";

            //int ret = serialPort->write("123456", 7);
            //QSerialPort 在子线程中调用write后并未等待数据发送出去,
            //子线程就被休眠导致数据丢失，
            //从而我们需要在子线程中调用waitForBytesWritten
            //等待write数据发送完毕。
            serialPort->waitForBytesWritten(10);
            QThread::msleep(10); //单位：ms
        }

        //qDebug() << count++;
        QThread::msleep(2000); //单位：ms
    }
}

 bool SendPackThread::makePack(char cmd, char param, char data[], char *pack)
 {
    if (!pack)return false;

    pack[0] = 0xEF;
    pack[1] = cmd;
    pack[2] = param;
    pack[3] = 0;
    pack[4] = 0;
    pack[5] = 0;
    pack[6] = 0;

    int count = 0;
    for (int i=0; i<8; i++) {
        if (cmd & 1) count++;
        cmd >>= 1;
    }
    for (int i=0; i<8; i++) {
        if (param & 1) count++;
        param >>= 1;
    }

    if (count & 1) { //如果count是计数，即有奇数个1
        pack[7] = 1;
    } else{
        pack[7] = 0;
    }

    pack[8] = 0xFE;
    return true;
 }
