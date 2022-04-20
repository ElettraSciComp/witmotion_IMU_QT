#include "witmotion/serial.h"
#include <exception>
#include <unistd.h>

namespace witmotion
{

void QBaseSerialWitmotionSensorReader::ReadData()
{
    qint64 bytes_read;
    qint64 bytes_avail = witmotion_port->bytesAvailable();
    if(bytes_avail == last_avail)
    {
        if(++avail_rep_count > 3)
            emit Error("No data acquired during last 3 iterations, please check the baudrate!");
    }
    else
    {
        last_avail = bytes_avail;
        avail_rep_count = 0;
    }
    if(bytes_avail > 0)
    {
        bytes_read = witmotion_port->read(reinterpret_cast<char*>(raw_data), 128);
        for(qint64 i = 0; i < bytes_read; i++)
        {
            uint8_t current_byte = raw_data[i];
            if(read_state == rsClear)
            {
                read_state = (current_byte == WITMOTION_HEADER_BYTE) ? rsUnknown : rsClear;
            }
            else if(read_state == rsUnknown)
            {
                if(id_registered(current_byte))
                {
                    read_cell = static_cast<witmotion_packet_id>(current_byte);
                    counts[read_cell] = 0;
                    packets[read_cell].header_byte = WITMOTION_HEADER_BYTE;
                    packets[read_cell].id_byte = read_cell;
                    read_state = rsRead;
                }
                else
                    read_state = rsClear;
            }
            else
            {
                if(counts[read_cell] == 8)
                {
                    packets[read_cell].crc = current_byte;
                    uint8_t current_crc = packets[read_cell].header_byte + packets[read_cell].id_byte;
                    for(uint8_t i = 0; i < 8; i++)
                        current_crc += packets[read_cell].datastore.raw[i];
                    if(!validate || (current_crc == packets[read_cell].crc))
                        emit Acquired(packets[read_cell]);
                    read_state = rsClear;
                }
                else
                    packets[read_cell].datastore.raw[counts[read_cell]++] = current_byte;
            }
        }
    }
}

void QBaseSerialWitmotionSensorReader::SendConfig(const witmotion_config_packet &packet)
{
    static uint8_t serial_datapacket[5];
    serial_datapacket[0] = packet.header_byte;
    serial_datapacket[1] = packet.key_byte;
    serial_datapacket[2] = packet.address_byte;
    serial_datapacket[3] = packet.setting.raw[0];
    serial_datapacket[4] = packet.setting.raw[1];
    quint64 written;
    ttyout << "Sending configuration packet..." << hex << "0x" << packet.address_byte << dec << endl;
    written = witmotion_port->write(reinterpret_cast<const char*>(serial_datapacket), 5);
    witmotion_port->waitForBytesWritten();
    if(written != 5)
        emit Error("Device reconfiguration error!");
    witmotion_port->flush();
    ttyout << "Configuration packet sent, please wait..." << endl;
    sleep(1);
}

void QBaseSerialWitmotionSensorReader::SetBaudRate(const QSerialPort::BaudRate &rate)
{
    port_rate = rate;
}

QBaseSerialWitmotionSensorReader::QBaseSerialWitmotionSensorReader(const QString device, const QSerialPort::BaudRate rate):
    port_name(device),
    witmotion_port(nullptr),
    port_rate(rate),
    last_avail(0),
    avail_rep_count(0),
    validate(false),
    user_defined_return_interval(false),
    return_interval(50),
    ttyout(stdout),
    poll_timer(nullptr),
    read_state(rsClear)
{
    qRegisterMetaType<witmotion_datapacket>("witmotion_datapacket");
    qRegisterMetaType<witmotion_config_packet>("witmotion_config_packet");
}

QBaseSerialWitmotionSensorReader::~QBaseSerialWitmotionSensorReader()
{
    if(poll_timer != nullptr)
        delete poll_timer;
    ttyout << "Closing TTL connection" << endl;
    if(witmotion_port != nullptr)
    {
        witmotion_port->close();
        delete witmotion_port;
    }
}

void QBaseSerialWitmotionSensorReader::RunPoll()
{
    witmotion_port = new QSerialPort(port_name);
    witmotion_port->setBaudRate(port_rate, QSerialPort::Direction::AllDirections);
    witmotion_port->setStopBits(QSerialPort::OneStop);
    witmotion_port->setParity(QSerialPort::NoParity);
    witmotion_port->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
    ttyout << "Opening device \"" << witmotion_port->portName() << "\" at " << witmotion_port->baudRate() << " baud" << endl;
    if(!witmotion_port->open(QIODevice::ReadWrite))
    {
        emit Error("Error opening the port!");
        return;
    }
    poll_timer = new QTimer(this);
    poll_timer->setTimerType(Qt::TimerType::PreciseTimer);
    if(!user_defined_return_interval)
        poll_timer->setInterval((port_rate == QSerialPort::Baud9600) ? 50 : 5);
    else
        poll_timer->setInterval(return_interval);
    timer_connection = connect(poll_timer, &QTimer::timeout, this, &QBaseSerialWitmotionSensorReader::ReadData);
    ttyout << "Instantiating timer at " << poll_timer->interval() << " ms" << endl;
    poll_timer->start();
}

void QBaseSerialWitmotionSensorReader::Suspend()
{
    disconnect(timer_connection);
    if(poll_timer != nullptr)
        delete poll_timer;
    if(witmotion_port != nullptr)
    {
        witmotion_port->close();
        delete witmotion_port;
    }
    ttyout << "Suspending TTL connection, please emit RunPoll() again to proceed!" << endl;
    poll_timer = nullptr;
    witmotion_port = nullptr;
}

void QBaseSerialWitmotionSensorReader::ValidatePackets(const bool value)
{
    validate = value;
}

void QBaseSerialWitmotionSensorReader::SetSensorPollInterval(const uint32_t ms)
{
    user_defined_return_interval = true;
    return_interval = ms;
}

}
