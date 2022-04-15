#include "witmotion/wt31n-uart.h"

namespace witmotion
{
namespace wt31n
{

QWitmotionWT31NSensor::QWitmotionWT31NSensor(const QString tty_name, const QSerialPort::BaudRate rate):
    reader_thread(dynamic_cast<QObject*>(this)),
    reader(nullptr),
    ttyout(stdout)
{
    reader = new QBaseSerialWitmotionSensorReader(tty_name, rate);
    reader->moveToThread(&reader_thread);
    connect(&reader_thread, &QThread::finished, reader, &QObject::deleteLater);
    connect(this, &QWitmotionWT31NSensor::RunReader, reader, &QAbstractWitmotionSensorReader::RunPoll);
    connect(reader, &QAbstractWitmotionSensorReader::Acquired, this, &QWitmotionWT31NSensor::Packet);
    connect(reader, &QAbstractWitmotionSensorReader::Error, this, &QWitmotionWT31NSensor::Error);
    reader_thread.start();
}

void QWitmotionWT31NSensor::Start()
{
    ttyout << "Running reader thread" << endl;
    emit RunReader();
}

QWitmotionWT31NSensor::~QWitmotionWT31NSensor()
{
    reader_thread.quit();
    reader_thread.wait(10000);
}

void QWitmotionWT31NSensor::Packet(const witmotion_datapacket &packet)
{
    ttyout << "Packet 0x" << hex << packet.id_byte << dec << " received" << endl;
}

void QWitmotionWT31NSensor::Error(const QString &description)
{
    ttyout << "ERROR: " << description << endl;
    reader->Suspend();
    emit ErrorOccurred(description);
}

}
}
