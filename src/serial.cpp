#include "witmotion/serial.h"
#include <exception>

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
                    switch(read_cell)
                    {
                    case pidRTC:
                        read_state = rsRTC;
                        break;
                    case pidAcceleration:
                        read_state = rsAcceleration;
                        break;
                    case pidAngularVelocity:
                        read_state = rsAngularVelocity;
                        break;
                    case pidAngles:
                        read_state = rsAngles;
                        break;
                    case pidMagnetometer:
                        read_state = rsMagnetometer;
                        break;
                    case pidDataPortStatus:
                        read_state = rsDataPortStatus;
                        break;
                    case pidAltimeter:
                        read_state = rsAltimeter;
                        break;
                    case pidGPSCoordinates:
                        read_state = rsGPSCoordinates;
                        break;
                    case pidGPSGroundSpeed:
                        read_state = rsGPSGroundSpeed;
                        break;
                    case pidOrientation:
                        read_state = rsOrientation;
                        break;
                    case pidGPSAccuracy:
                        read_state = rsGPSAccuracy;
                        break;
                    }
                }
                else
                    read_state = rsClear;
            }
            else
            {
                if(counts[read_cell] == 8)
                {
                    packets[read_cell].crc = current_byte;
                    emit Acquired(packets[read_cell]);
                    read_state = rsClear;
                }
                else
                    packets[read_cell].datastore.raw[counts[read_cell]++] = current_byte;
            }
        }
    }
}

QBaseSerialWitmotionSensorReader::QBaseSerialWitmotionSensorReader(const QString device, const QSerialPort::BaudRate rate):
    port_name(device),
    witmotion_port(nullptr),
    port_rate(rate),
    last_avail(0),
    avail_rep_count(0),
    ttyout(stdout),
    poll_timer(nullptr),
    read_state(rsClear)
{
    if(!((rate == QSerialPort::Baud9600)||(rate == QSerialPort::Baud115200)))
    {
        emit Error("Only 9600 and 115200 baud rates are supported!");
        return;
    }
    qRegisterMetaType<witmotion_datapacket>("witmotion_datapacket");
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
    poll_timer->setInterval((port_rate == QSerialPort::Baud9600) ? 50 : 5);
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

}
