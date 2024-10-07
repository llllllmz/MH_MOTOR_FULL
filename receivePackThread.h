#ifndef RECEIVEPACK_H
#define RECEIVEPACK_H

#include <QObject>
#include <QThread>
#include <QSerialPort>

class ReceivePackThread : public QThread
{
    Q_OBJECT
public:
    ReceivePackThread(QSerialPort*port);

signals:
    void receiveResponsePack(char cmd, char param, int data);

protected:
    void run();
    bool parsePack(unsigned char *pack, int size, char *cmd, char *param, int *data);
    QSerialPort *serialPort;
};

#endif // RECEIVEPACK_H
