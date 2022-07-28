#ifndef WITMOTION_WT901
#define WITMOTION_WT901

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
namespace wt901
{

class QWitmotionWT901Sensor: public QAbstractWitmotionSensorController
{
    Q_OBJECT
private:
    static const std::set<witmotion_packet_id> registered_types;
public:
    virtual const std::set<witmotion_packet_id>* RegisteredPacketTypes();
    virtual void Start();
    virtual void UnlockConfiguration();
    virtual void Calibrate();
    virtual void SetBaudRate(const QSerialPort::BaudRate& rate);
    virtual void SetPollingRate(const int32_t hz);
    virtual void ConfirmConfiguration();
    QWitmotionWT901Sensor(const QString device,
                          const QSerialPort::BaudRate rate,
                          const uint32_t polling_period = 50);
};

}
}

#endif
