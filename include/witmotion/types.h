#ifndef WITMOTION
#define WITMOTION
#include <cmath>
#include <set>
#include <inttypes.h>

#include <QtCore>
#include <QSerialPort>

/*!
    \file types.h
    \brief Abstract types collection for Witmotion sensor library
    \author Andrey Vukolov andrey.vukolov@elettra.eu

    This header file contains all abstract types and hardware-defined constants to operate Witmotion sensor device.
*/


/*!
  \brief Main namespace of Witmotion UART connection library

Upper level namespace containing all the declared constants, parameters, classes, functions.

__NOTE__: it is strictly NOT RECOMMENDED to use this namespace implicitly through `using namespace` directive.
*/
namespace witmotion
{

static const uint8_t WITMOTION_HEADER_BYTE = 0x55; ///< Packet header byte value (vendor protocol-specific)
static const uint8_t WITMOTION_CONFIG_HEADER = 0xFF; ///< Configuration header byte value (vendor protocol-specific)
static const uint8_t WITMOTION_CONFIG_KEY = 0xAA; ///< Configuration marker key byte value (vendor protocol-specific)
static const float DEG2RAD = M_PI / 180.f;

/*!
  \brief Packet type IDs from the vendor-defined protocol

  If one of the packet type IDs defined here is registered after \ref WITMOTION_HEADER_BYTE in the data flow
received from the sensor, the packet header is considered found and the remaining bytes are considered as body of the packet.
See \ref util.h for decoder function reference.
*/
enum witmotion_packet_id
{
    pidRTC = 0x50, ///< Real-Time-Clock
    pidAcceleration = 0x51, ///< Linear accelerations + temperature/reserved field [X-Y-Z] (16-bit binary normalized quasi-floats)
    pidAngularVelocity = 0x52, ///< Angular velocities + temperature/reserved field [Roll-Pitch-Yaw] (16-bit binary normalized quasi-floats)
    pidAngles = 0x53, ///< Euler angles + temperature/reserved field [Roll-Pitch-Yaw] (16-bit binary normalized quasi-floats)
    pidMagnetometer = 0x54, ///< Magnetic field tensity + temperature/reserved field [world X-Y-Z] (16-bit binary normalized quasi-floats)
    pidDataPortStatus = 0x55, ///< Data port status packet, vendor-defined value
    pidAltimeter = 0x56, ///< Altimeter + Barometer output (32-bit binary normalized quasi-floats)
    pidGPSCoordinates = 0x57, ///< GPS: longitude + latitude, if supported by hardware (32-bit binary normalized quasi-floats)
    pidGPSGroundSpeed = 0x58, ///< GPS: ground speed (32-bit binary normalized quasi-float) + altitude + angular velocity around vertical axis (16-bit binary normalized quasi-floats), if supported by hardware
    pidOrientation = 0x59, ///< Orientation defined as quaternion [X-Y-Z-W], when available from the sensor firmware (16-bit binary normalized quasi-floats)
    pidGPSAccuracy = 0x5A ///< GPS: visible satellites + variance vector [East-North-Up] (16-bit binary normalized quasi-floats)
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
    0x59,
    0x5A
};

static const std::map<uint8_t, std::string> witmotion_packet_descriptions = {
    {0x50, "Real Time Clock"},
    {0x51, "Accelerations"},
    {0x52, "Angular velocities"},
    {0x53, "Spatial orientation (Euler angles)"},
    {0x54, "Magnetometer/Magnetic orientation"},
    {0x55, "Data ports (D0-D3) status"},
    {0x56, "Barometry/Altimeter"},
    {0x57, "GPS Coordinates"},
    {0x58, "GPS Ground Speed"},
    {0x59, "Spatial orientation (Quaternion)"},
    {0x5A, "GPS accuracy estimation"}
};

/*!
 * \brief Generic structure respresenting the standard 11-byte datapacket defined in Witmotion protocol.
*/
struct witmotion_datapacket
{
    uint8_t header_byte; ///< Header byte, set constantly to \ref WITMOTION_HEADER_BYTE
    uint8_t id_byte; ///< Packet type ID, referring to \ref witmotion_packet_id, otherwise the packet is considered of unknown type.
    union
    {
        int8_t raw_signed[8];
        uint8_t raw[8];
        int16_t raw_cells[4];
        int32_t raw_large[2];
    }datastore; ///< 8-byte internal data storage array represented as C-style memory union. The stored data represented as `int8_t*`, `uint8_t*`, `int16_t*` or `int32_t*` array head pointer.
    uint8_t crc; ///< Validation CRC for the packet. Calculated as an equivalent to the following operation: \f$ crc = \sum_{i=0}^{i < 10} *\mbox{reinterpret_cast<uint8_t*>(this)} + i \f$
};

/*!
 * \brief List of configuration slots (registers) available for the library.
 *
 * List of configuration slots (registers) available for the library. The actual availability depends from the actual sensor and installation circuit. Please refer to the official documentation for detailed explanation.
*/
enum witmotion_config_register_id
{
    /*!
      Saves the settings uploaded in the current bringup session, or resets it to default (if supported). To make factory reset of the sensor, set `raw[0] = 0x01` in \ref witmotion_config_packet instance used.
    */
    ridSaveSettings = 0x00,
    /*!
      Sets the sensor to calibration mode. The value stored in \ref witmotion_config_packet.`raw[0]` determines device selection:
      - `0x00` - End calibration
      - `0x01` - Accelerometer calibration
      - `0x02` - Magnetometer calibration
      - `0x03` - Altitude reset (only for barometric altimeter)
    */
    ridCalibrate = 0x01,
    ridOutputValueSet = 0x02,
    ridOutputFrequency = 0x03,
    ridPortBaudRate = 0x04,
    ridAccelerationBiasX = 0x05,
    ridAccelerationBiasY = 0x06,
    ridAccelerationBiasZ = 0x07,
    ridAngularVelocityBiasX = 0x08,
    ridAngularVelocityBiasY = 0x09,
    ridAngularVelocityBiasZ = 0x0A,
    ridMagnetometerBiasX = 0x0B,
    ridMagnetometerBiasY = 0x0C,
    ridMagnetometerBiasZ = 0x0D,
    ridPortModeD0 = 0x0E,
    ridPortModeD1 = 0x0F,
    ridPortModeD2 = 0x10,
    ridPortModeD3 = 0x11,
    ridPortPWMLevelD0 = 0x12,
    ridPortPWMLevelD1 = 0x13,
    ridPortPWMLevelD2 = 0x14,
    ridPortPWMLevelD3 = 0x15,
    ridPortPWMPeriodD0 = 0x16,
    ridPortPWMPeriodD1 = 0x17,
    ridPortPWMPeriodD2 = 0x18,
    ridPortPWMPeriodD3 = 0x19,
    ridIICAddress = 0x1A,
    ridLED = 0x1B,
    ridGPSBaudRate = 0x1C,

    ridStandbyMode = 0x22,
    ridInstallationDirection = 0x23,
    ridTransitionAlgorithm = 0x24,

    ridTimeYearMonth = 0x30,
    ridTimeDayHour = 0x31,
    ridTimeMinuteSecond = 0x32,
    ridTimeMilliseconds = 0x33,
    ridSetAccelerationX = 0x34,
    ridSetAccelerationY = 0x35,
    ridSetAccelerationZ = 0x36,
    ridSetAngularVelocityX = 0x37,
    ridSetAngularVelocityY = 0x38,
    ridSetAngularVelocityZ = 0x39,
    ridSetMagnetometerX = 0x3A,
    ridSetMagnetometerY = 0x3B,
    ridSetMagnetometerZ = 0x3C,
    ridSetAngleRoll = 0x3D,
    ridSetAnglePitch = 0x3E,
    ridSetAngleYaw = 0x3F,
    ridSetTemperature = 0x40,
    ridSetPortStatusD0 = 0x41,
    ridSetPortStatusD1 = 0x42,
    ridSetPortStatusD2 = 0x43,
    ridSetPortStatusD3 = 0x44,
    ridSetPressureLow = 0x45,
    ridSetPressureHigh = 0x46,
    ridSetAltitudeLow = 0x47,
    ridSetAltitudeHigh = 0x48,
    ridSetLongitudeLow = 0x49,
    ridSetLongitudeHigh = 0x4A,
    ridSetLatitudeLow = 0x4B,
    ridSetLatitudeHigh = 0x4C,
    ridSetGPSAltitude = 0x4D,
    ridSetGPSYaw = 0x4E,
    ridSetGPSGroundSpeedLow = 0x4F,
    ridSetGPSGroundSpeedHigh = 0x50,
    ridSetOrientationX = 0x51,
    ridSetOrientationY = 0x52,
    ridSetOrientationZ = 0x53,
    ridSetOrientationW = 0x54,

    ridGyroscopeAutoCalibrate = 0x63
};

/*!
 * \brief Generic structure respresenting the standard 5-byte configuration command defined in Witmotion protocol.
*/
struct witmotion_config_packet
{
    uint8_t header_byte; ///< Header byte, set constantly to \ref WITMOTION_CONFIG_HEADER
    uint8_t key_byte; ///< Packet type, constantly set to \ref WITMOTION_CONFIG_KEY
    uint8_t address_byte; ///< Configuration slot address, refers to the registered values in \ref witmotion_config_register_id
    union
    {
        uint8_t raw[2];
        uint16_t* bin;
    }setting; ///< 2-byte internal data storage array represented as C-style memory union. The values should be formulated byte-by-byte referring to the actual sensor's documentation.
};

class QAbstractWitmotionSensorReader: public QObject
{   Q_OBJECT
protected slots:
    virtual void ReadData() = 0;
public slots:
    virtual void SendConfig(const witmotion_config_packet& packet) = 0;
    virtual void RunPoll() = 0;
signals:
    void Acquired(const witmotion_datapacket& packet);
    void Error(const QString& description);
};

}

Q_DECLARE_METATYPE(witmotion::witmotion_datapacket);
Q_DECLARE_METATYPE(witmotion::witmotion_config_packet);

#endif
