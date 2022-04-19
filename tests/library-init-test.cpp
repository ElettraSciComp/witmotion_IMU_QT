#include "witmotion/wt31n-uart.h"
#include <unistd.h>
using namespace witmotion::wt31n;


int main(int argc, char** args)
{
    QCoreApplication app(argc, args);
    QWitmotionWT31NSensor sensor("ttyUSB0", QSerialPort::Baud9600);
    sensor.setParent(&app);
    sensor.Start();
    return app.exec();
}
