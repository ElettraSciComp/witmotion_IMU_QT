#ifndef WITMOTION
#define WITMOTION
#include <cmath>
#include <set>
#include <inttypes.h>

#include <QtCore>
#include <QSerialPort>

#include "witmotion/version.h"

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

/*!
  \brief Packet ID set to retrieve descriptions via \ref witmotion_packet_descriptions.

  Contains values referenced in \ref witmotion_packet_id enumeration to explicitly determine a set of currently supported packet IDs. The packet IDs not referenced here sould not be considered supported.
*/
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

/*!
  \brief Packet ID string set to store built-in descriptions for \ref message_enumerator.

  Contains values referenced in \ref witmotion_packet_id enumeration with corresponding description strings used by \ref message_enumerator application.
*/
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
 * List of configuration slots (registers) available for the library. The actual availability depends from the actual sensor and installation circuit.
 * Please refer to the official documentation for detailed explanation. **NOTE**: values which are currently untested/unsupported by the certain sensors or known malfunction producers are marked here as following:
*/
enum witmotion_config_register_id
{
    ridSaveSettings = 0x00, ///< Saves the settings uploaded in the current bringup session, or resets it to default (if supported). To make factory reset of the sensor, set `raw[0] = 0x01` in \ref witmotion_config_packet instance used.
    /*!
      Sets the sensor to calibration mode. The value stored in \ref witmotion_config_packet.`raw[0]` determines device selection:
      - `0x00` - End calibration
      - `0x01` - Accelerometer calibration
      - `0x02` - Magnetometer calibration
      - `0x03` - Altitude reset (only for barometric altimeter)
    */
    ridCalibrate = 0x01,
    /*!
      Regulates sensor output. The value stored in \ref witmotion_config_packet.`raw` determines packet ID selection to output from low to high bits by offset. `0` means disabling of the selected data packet output.

      |`raw[0]` offset|Packet type|`raw[1]` offset| Packet type|
      |:-------------:|----------:|:-------------:|-----------:|
      |0|\ref pidRTC|0|\ref pidGPSGroundSpeed|
      |1|\ref pidAcceleration|1|\ref pidOrientation|
      |2|\ref pidAngularVelocity|2|\ref pidGPSAccuracy|
      |3|\ref pidAngles|3|Reserved|
      |4|\ref pidMagnetometer|4|Reserved|
      |5|\ref pidDataPortStatus|5|Reserved|
      |6|\ref pidAltimeter|6|Reserved|
      |7|\ref pidGPSCoordinates|7|Reserved|
    */
    ridOutputValueSet = 0x02,
    /*!
      Regulates output frequency. **NOTE**: the maximum available frequency is determined internally by the available bandwidth obtained from \ref ridPortBaudRate.
      The actual value stored in \ref witmotion_config_packet.`raw[0]` can be determined from the following table. \ref witmotion_config_packet.`raw[1]` is set to `0x00`. Also the table contains argument value for \ref witmotion_output_frequency helper function which is used by the controller applications.

      |**Frequency, Hz**|0 (shutdown)|0 (single measurement)|0.1   |0.5   |1     |2     |5     |10    |20    |50    |100   |125   |200   |
      |:----------------|:----------:|:--------------------:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|
      |**Value**        |`0x0D`      |`0x0C`                |`0x01`|`0x02`|`0x03`|`0x04`|`0x05`|`0x06`|`0x07`|`0x08`|`0x09`|`0x0A`|`0x0B`|
      |**Argument**     | 0          | -1                   | -10  | -2   | 1    | 2    | 5    | 10   | 20   | 50   | 100  | 125  | 200  |
    */
    ridOutputFrequency = 0x03,
    /*!
      Regulates port baud rate. **NOTE**: the sensor has no possibility of hardware flow control and it cannot report to the system what baud rate should be explicitly used!
      The actual value stored in \ref witmotion_config_packet.`raw[0]` can be determined from the following table. \ref witmotion_config_packet.`raw[1]` is set to `0x00`.
      The \ref witmotion_baud_rate helper function argument is accepted as `QSerialPort::BaudRate` enumeration member, so only the speed inticated in that enumeration are explicitly supported.
      |**Rate, baud**|1200/1400|4800  |9600  |19200 |38400 |57600 |115200|
      |:-------------|:-------:|:----:|:----:|:----:|:----:|:----:|:----:|
      |**Value**     |`0x00`   |`0x01`|`0x02`|`0x03`|`0x04`|`0x05`|`0x06`|

      This parameter also implicitly sets \ref ridOutputFrequency to the maximal feasible value for the available bandwidth.
    */
    ridPortBaudRate = 0x04,
    ridAccelerationBiasX = 0x05, ///< Sets acceleration zero point bias for X axis
    ridAccelerationBiasY = 0x06, ///< Sets acceleration zero point bias for Y axis
    ridAccelerationBiasZ = 0x07, ///< Sets acceleration zero point bias for Z axis
    ridAngularVelocityBiasX = 0x08, ///< Sets angular velocity zero point bias for X axis \note PROOFLESS
    ridAngularVelocityBiasY = 0x09, ///< Sets angular velocity zero point bias for Y axis \note PROOFLESS
    ridAngularVelocityBiasZ = 0x0A, ///< Sets angular velocity zero point bias for Z axis \note PROOFLESS
    ridMagnetometerBiasX = 0x0B, ///< Sets magnetometer zero point bias for X axis \note PROOFLESS
    ridMagnetometerBiasY = 0x0C, ///< Sets magnetometer zero point bias for Y axis \note PROOFLESS
    ridMagnetometerBiasZ = 0x0D, ///< Sets magnetometer zero point bias for Z axis \note PROOFLESS
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

    ridGyroscopeAutoCalibrate = 0x63,

    ridUnlockConfiguration = 0x69
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
