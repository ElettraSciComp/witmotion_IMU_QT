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
    pidMagnetometer,
    pidOrientation,
    pidRTC
};

void QWitmotionWT901Sensor::UnlockConfiguration()
{
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridUnlockConfiguration;
    config_packet.setting.raw[0] = 0x88;
    config_packet.setting.raw[1] = 0xB5;
    ttyout << "Configuration ROM: lock removal started" << ENDL;
    emit SendConfig(config_packet);
    sleep(1);
}

const std::set<witmotion_packet_id> *QWitmotionWT901Sensor::RegisteredPacketTypes()
{
    return &registered_types;
}

void QWitmotionWT901Sensor::Start()
{
    ttyout << "Running reader thread" << ENDL;
    emit RunReader();
}

void QWitmotionWT901Sensor::Calibrate()
{
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridCalibrate;
    config_packet.setting.raw[0] = 0x01;
    config_packet.setting.raw[1] = 0x00;
    ttyout << "Entering spatial calibration, please hold the sensor in fixed position for 5 seconds" << ENDL;
    emit SendConfig(config_packet);
    sleep(5);
    config_packet.setting.raw[0] = 0x00;
    ttyout << "Exiting spatial calibration mode" << ENDL;
    emit SendConfig(config_packet);
    sleep(1);
}

void QWitmotionWT901Sensor::CalibrateMagnetometer()
{
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridCalibrate;
    config_packet.setting.raw[0] = 0x02;
    config_packet.setting.raw[1] = 0x00;
    ttyout << "Entering magnetic calibration, please hold the sensor in fixed position for 5 seconds" << ENDL;
    emit SendConfig(config_packet);
    sleep(5);
    config_packet.setting.raw[0] = 0x00;
    ttyout << "Exiting magnetic calibration mode" << ENDL;
    emit SendConfig(config_packet);
    sleep(1);
}

void QWitmotionWT901Sensor::SetBaudRate(const QSerialPort::BaudRate &rate)
{
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridPortBaudRate;
    port_rate = rate;
    config_packet.setting.raw[0] = witmotion_baud_rate(port_rate);
    config_packet.setting.raw[1] = 0x00;
    emit SendConfig(config_packet);
    sleep(1);
}

void QWitmotionWT901Sensor::SetPollingRate(const int32_t hz)
{
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridOutputFrequency;
    config_packet.setting.raw[0] = witmotion_output_frequency(hz);
    config_packet.setting.raw[1] = 0x00;
    emit SendConfig(config_packet);
    sleep(1);
}

void QWitmotionWT901Sensor::ConfirmConfiguration()
{
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridSaveSettings;
    config_packet.setting.raw[0] = 0x00;
    config_packet.setting.raw[1] = 0x00;
    emit SendConfig(config_packet);
    sleep(1);
}

void QWitmotionWT901Sensor::SetOrientation(const bool vertical)
{
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridInstallationDirection;
    config_packet.setting.raw[0] = vertical ? 0x01 : 0x00;
    config_packet.setting.raw[1] = 0x00;
    emit SendConfig(config_packet);
    sleep(1);
}

void QWitmotionWT901Sensor::ToggleDormant()
{
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridStandbyMode;
    config_packet.setting.raw[0] = 0x01;
    config_packet.setting.raw[1] = 0x00;
    emit SendConfig(config_packet);
    sleep(1);
}

void QWitmotionWT901Sensor::SetGyroscopeAutoRecalibration(const bool recalibrate)
{
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridGyroscopeAutoCalibrate;
    config_packet.setting.raw[0] = recalibrate ? 0x00 : 0x01;
    config_packet.setting.raw[1] = 0x00;
    emit SendConfig(config_packet);
    sleep(1);
}

void QWitmotionWT901Sensor::SetAxisTransition(const bool axis9)
{
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridTransitionAlgorithm;
    config_packet.setting.raw[0] = axis9 ? 0x00 : 0x01;
    config_packet.setting.raw[1] = 0x00;
    emit SendConfig(config_packet);
    sleep(1);
}

void QWitmotionWT901Sensor::SetLED(const bool on)
{
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridLED;
    config_packet.setting.raw[0] = on ? 0x00 : 0x01;
    config_packet.setting.raw[1] = 0x00;
    emit SendConfig(config_packet);
    sleep(1);
}

void QWitmotionWT901Sensor::SetMeasurements(const bool realtime_clock,
                                            const bool acceleration,
                                            const bool angular_velocity,
                                            const bool euler_angles,
                                            const bool magnetometer,
                                            const bool orientation)
{
    uint8_t measurement_setting_low = 0x00;
    uint8_t measurement_setting_high = 0x00;
    realtime_clock ? measurement_setting_low |= 0x01 : measurement_setting_low &= ~(0x01);
    acceleration ? measurement_setting_low |= (0x01 << 1) : measurement_setting_low &= ~(0x01 << 1);
    angular_velocity ? measurement_setting_low |= (0x01 << 2) : measurement_setting_low &= ~(0x01 << 2);
    euler_angles ? measurement_setting_low |= (0x01 << 3) : measurement_setting_low &= ~(0x01 << 3);
    magnetometer ? measurement_setting_low |= (0x01 << 4) : measurement_setting_low &= ~(0x01 << 4);
    orientation ? measurement_setting_high |= (0x01 << 1) : measurement_setting_high &= ~(0x01 << 1);
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridOutputValueSet;
    config_packet.setting.raw[0] = measurement_setting_low;
    config_packet.setting.raw[1] = measurement_setting_high;
    emit SendConfig(config_packet);
    sleep(1);
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
           << ENDL;
    reader->SetSensorPollInterval(polling_period);
}

}
}
