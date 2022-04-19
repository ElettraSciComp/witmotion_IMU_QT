#include "witmotion/wt31n-uart.h"

namespace witmotion
{
namespace wt31n
{

QWitmotionWT31NSensor::QWitmotionWT31NSensor(const QString tty_name, const QSerialPort::BaudRate rate):
    port_name(tty_name),
    port_rate(rate),
    reader_thread(dynamic_cast<QObject*>(this)),
    reader(nullptr),
    ttyout(stdout)
{
    reader = new QBaseSerialWitmotionSensorReader(port_name, port_rate);
    reader->moveToThread(&reader_thread);
    connect(&reader_thread, &QThread::finished, reader, &QObject::deleteLater);
    connect(this, &QWitmotionWT31NSensor::RunReader, reader, &QAbstractWitmotionSensorReader::RunPoll);
    connect(reader, &QAbstractWitmotionSensorReader::Acquired, this, &QWitmotionWT31NSensor::Packet);
    connect(reader, &QAbstractWitmotionSensorReader::Error, this, &QWitmotionWT31NSensor::Error);
    connect(this, &QWitmotionWT31NSensor::SendConfig, reader, &QAbstractWitmotionSensorReader::SendConfig);
    reader_thread.start();
}

void QWitmotionWT31NSensor::Start()
{
    ttyout << "Running reader thread" << endl;
    if(!((port_rate == QSerialPort::Baud9600) || (port_rate == QSerialPort::Baud115200)))
        emit ErrorOccurred("Only 9600 or 115200 baud rates are supported for WT31N!");
    else
        emit RunReader();
}

QWitmotionWT31NSensor::~QWitmotionWT31NSensor()
{
    reader_thread.quit();
    reader_thread.wait(10000);
}

void QWitmotionWT31NSensor::Calibrate()
{
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridCalibrate;
    config_packet.setting.raw[0] = 0x01;
    config_packet.setting.raw[1] = 0x00;
    emit SendConfig(config_packet);
    config_packet.address_byte = ridSaveSettings;
    config_packet.setting.raw[0] = 0x00;
    emit SendConfig(config_packet);
}

void QWitmotionWT31NSensor::SetBaudRate(const QSerialPort::BaudRate &rate)
{
    if(!((rate == QSerialPort::Baud9600) || (rate == QSerialPort::Baud115200)))
    {
        emit ErrorOccurred("Only 9600 or 115200 baud rates are supported for WT31N!");
        return;
    }
    witmotion_config_packet config_packet;
    config_packet.header_byte = WITMOTION_CONFIG_HEADER;
    config_packet.key_byte = WITMOTION_CONFIG_KEY;
    config_packet.address_byte = ridPortBaudRate;
    port_rate = rate;
    config_packet.setting.raw[0] = witmotion_baud_rate(port_rate);
    config_packet.setting.raw[1] = 0x00;
    emit SendConfig(config_packet);
    config_packet.address_byte = ridOutputFrequency;
    config_packet.setting.raw[0] = (port_rate == QSerialPort::Baud9600) ? witmotion_output_frequency(2) : witmotion_output_frequency(10);
    emit SendConfig(config_packet);
    config_packet.address_byte = ridSaveSettings;
    config_packet.setting.raw[0] = 0x00;
    emit SendConfig(config_packet);
    ttyout << "Reopening port..." << endl;
    reader->Suspend();
    sleep(1);
    emit RunReader();
}

void QWitmotionWT31NSensor::SetValidation(const bool validate)
{
    reader->ValidatePackets(validate);
}

void QWitmotionWT31NSensor::Packet(const witmotion_datapacket &packet)
{
    emit Acquired(packet);
    float x, y, z, t;
    switch(static_cast<witmotion_packet_id>(packet.id_byte))
    {
    case pidAcceleration:
        decode_accelerations(packet, x, y, z, t);
        emit AcquiredAccelerations(x, y, z, t);
        break;
    case pidAngles:
        decode_angles(packet, x, y, z, t);
        emit AcquiredAngles(x, y, z, t);
        break;
    default:
        emit ErrorOccurred("Invalid packet ID acquired. Please be sure that you selected a proper driver class and namespace!");
    }
}

void QWitmotionWT31NSensor::Error(const QString &description)
{
    ttyout << "ERROR: " << description << endl;
    reader->Suspend();
    emit ErrorOccurred(description);
}

}
}
