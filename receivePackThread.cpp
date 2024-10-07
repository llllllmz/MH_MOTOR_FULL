#include "receivePackThread.h"
#include <QDebug>

ReceivePackThread::ReceivePackThread(QSerialPort* port)
{
    serialPort = port;
}

void ReceivePackThread::run()
{
    unsigned char pack[9];
    char cmd;
    char param;
    int data;
    while(1) {
        //waitForReadyRead 不能不用
        //如果不用，在循环中频繁调用bytesAvailable，测试后发现反而没有效果
        //如果waitForReadyRead使用默认参数，是30*1000(30秒）
        //将导致30s失去响应
        //waitForReadyRead, 仅在调用时判断是否有数据可读，然后就等待一定时间
        //在等待期间，如果有数据过来了，不会被唤醒，而是要等待完指定的时间
        serialPort->waitForReadyRead(100);
        int ret = serialPort->bytesAvailable();
        if(ret == 0) continue;

        ret=   serialPort->read((char*)pack, 9); //没有数据可读，就直接返回0
        qDebug() << "read " << ret << " bytes：" <<
               pack[0] << " " << pack[1] << " " << pack[2] << " " << pack[3] << " " <<
               pack[4] << " " << pack[5] << " " << pack[6] << " " << pack[7] << " " << pack[8];


        if(parsePack(pack, sizeof(pack), &cmd, &param,  &data)) {
            qDebug() << "发信号";
            emit receiveResponsePack(cmd, param, data); //发送信号
        } else {
            qDebug() <<"非法包";
        }




    }
}

 bool ReceivePackThread::parsePack(unsigned char *pack, int size, char *cmd, char*param, int *data)
 {
    if (!pack || size < 9) return false;

    if(pack[0] == 0xEF && pack[8]==0xFE) {
        unsigned char *p = pack+1;
        int count = 0;
        for (int i=0; i<7; i++, p++) {
            char tmp = *p;
            for (int k=0; k<8; k++) {
                if (tmp & 1)count++;
                tmp >>= 1;
            }
        }
        if (count % 2) {
            return false;
        }

        *cmd = pack[1];
        *param = pack[2];
        *data = pack[3] | (pack[4]<<8) | (pack[5]<<16) | (pack[6]<<24);
        qDebug() << "data: " << *data;
        return true;
    }

    return false;
 }
