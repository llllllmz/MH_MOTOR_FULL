#ifndef SENDPACK_H
#define SENDPACK_H

#include <QThread>
#include <QSerialPort>

class SendPackThread : public QThread
{
    Q_OBJECT
public:
    SendPackThread(QSerialPort*port);
    bool makePack(char cmd, char param, char data[], char *pack);
private:
    void run();
    QSerialPort *serialPort;
};

#endif // SENDPACK_H
