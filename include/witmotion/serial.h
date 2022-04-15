#ifndef WITMOTION_SERIAL_H
#define WITMOTION_SERIAL_H

#include "witmotion/types.h"
#include "witmotion/util.h"

#include <QtCore>
#include <QSerialPort>

namespace witmotion
{

class QBaseSerialWitmotionSensorReader: public QAbstractWitmotionSensorReader
{
    Q_OBJECT
private:
    QString port_name;
    QSerialPort* witmotion_port;
    QSerialPort::BaudRate port_rate;
    qint64 last_avail;
    quint16 avail_rep_count;
    uint8_t raw_data[128];
protected:
    QTextStream ttyout;
    QTimer* poll_timer;
    QMetaObject::Connection timer_connection;
    enum read_state_t
    {
        rsUnknown,
        rsClear,
        rsRTC,
        rsAcceleration,
        rsAngularVelocity,
        rsAngles,
        rsMagnetometer,
        rsDataPortStatus,
        rsAltimeter,
        rsGPSCoordinates,
        rsGPSGroundSpeed,
        rsOrientation,
        rsGPSAccuracy
    }read_state;
    witmotion_typed_packets packets;
    witmotion_typed_bytecounts counts;
    witmotion_packet_id read_cell;

    virtual void ReadData();
public:
    QBaseSerialWitmotionSensorReader(const QString device, const QSerialPort::BaudRate rate);
    virtual ~QBaseSerialWitmotionSensorReader();
    virtual void RunPoll();
    virtual void Suspend();
};

}
#endif
