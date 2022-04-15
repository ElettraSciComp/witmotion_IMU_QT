#ifndef WITMOTION
#define WITMOTION
#include <cmath>
#include <set>
#include <inttypes.h>

#include <QtCore>

namespace witmotion
{

static const uint8_t WITMOTION_HEADER_BYTE = 0x55;

enum witmotion_packet_id
{
    pidRTC = 0x50,
    pidAcceleration = 0x51,
    pidAngularVelocity = 0x52,
    pidAngles = 0x53,
    pidMagnetometer = 0x54,
    pidDataPortStatus = 0x55,
    pidAltimeter = 0x56,
    pidGPSCoordinates = 0x57,
    pidGPSGroundSpeed = 0x58,
    pidOrientation = 0x59,
    pidGPSAccuracy = 0x5A
};

static const std::set<size_t> witmotion_registered_ids = {
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x5A
};

struct witmotion_datapacket
{
    uint8_t header_byte;
    uint8_t id_byte;
    union
    {
        int8_t raw_signed[8];
        uint8_t raw[8];
        int16_t raw_cells[4];
        uint32_t raw_large[2];
    }datastore;
    uint8_t crc;
};

class QAbstractWitmotionSensorReader: public QObject
{   Q_OBJECT
protected slots:
    virtual void ReadData() = 0;
public slots:
    virtual void RunPoll() = 0;
signals:
    void Acquired(const witmotion_datapacket& packet);
    void Error(const QString& description);
};

}

Q_DECLARE_METATYPE(witmotion::witmotion_datapacket);

#endif
