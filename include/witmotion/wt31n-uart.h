#ifndef WITMOTION_WT31N
#define WITMOTION_WT31N

#include <QSerialPort>
#include <QSerialPortInfo>

#include <iostream>
#include <string>

#include <unistd.h>

#include "witmotion/types.h"
#include "witmotion/util.h"
#include "witmotion/serial.h"

namespace witmotion
{
namespace wt31n
{

class QWitmotionWT31NSensor: public QObject
{
    Q_OBJECT
private:
    QThread reader_thread;
    QBaseSerialWitmotionSensorReader* reader;
    QTextStream ttyout;
public:
    QWitmotionWT31NSensor(const QString tty_name, const QSerialPort::BaudRate rate);
    void Start();
    virtual ~QWitmotionWT31NSensor();
public slots:
    void Packet(const witmotion_datapacket& packet);
    void Error(const QString& description);
signals:
    void RunReader();
    void ErrorOccurred(const QString& description);
};

}
}

#endif
