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
    QString port_name;
    QSerialPort::BaudRate port_rate;
    QThread reader_thread;
    QBaseSerialWitmotionSensorReader* reader;
    QTextStream ttyout;
public:
    QWitmotionWT31NSensor(const QString tty_name, const QSerialPort::BaudRate rate);
    void Start();
    virtual ~QWitmotionWT31NSensor();
    void Calibrate();
    void SetBaudRate(const QSerialPort::BaudRate& rate);
    void SetValidation(const bool validate);
public slots:
    void Packet(const witmotion_datapacket& packet);
    void Error(const QString& description);
signals:
    void RunReader();
    void ErrorOccurred(const QString& description);
    void Acquired(const witmotion_datapacket& packet);
    void AcquiredAccelerations(float& x, float& y, float& z, float& t);
    void AcquiredAngles(float& roll, float& pitch, float& yaw, float& t);
    void SendConfig(const witmotion_config_packet& packet);
};

}
}

#endif
