#include "witmotion/wt901-uart.h"

namespace witmotion
{
namespace wt901
{

using namespace Qt;

const std::set<witmotion_packet_id> QWitmotionWT901Sensor::registered_types =
{
    pidAcceleration,
    pidAngularVelocity,
    pidAngles,
    pidMagnetometer
};

const std::set<witmotion_packet_id> *QWitmotionWT901Sensor::RegisteredPacketTypes()
{
    return &registered_types;
}

void QWitmotionWT901Sensor::Start()
{

}

void QWitmotionWT901Sensor::Calibrate()
{

}

void QWitmotionWT901Sensor::SetBaudRate(const QSerialPort::BaudRate &rate)
{

}

void QWitmotionWT901Sensor::SetPollingRate(const uint32_t hz)
{

}

QWitmotionWT901Sensor::QWitmotionWT901Sensor(const QString device,
                                             const QSerialPort::BaudRate rate,
                                             const uint32_t polling_period):
    QAbstractWitmotionSensorController(device, rate)
{
    ttyout << "Creating multithreaded interface for Witmotion WT901 IMU sensor connected to "
           << port_name
           << " at "
           << static_cast<int32_t>(port_rate)
           << " baud"
           << Qt::endl;
    reader->SetSensorPollInterval(polling_period);
}

}
}
