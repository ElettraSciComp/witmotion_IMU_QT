#include "witmotion/wt31n-uart.h"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <numeric>

#include <QtCore>
#include <QSerialPort>

using namespace witmotion::wt31n;

double variance(const std::vector<float>& array)
{
    double sum = std::accumulate(array.begin(), array.end(), 0.f);
    double mean = sum / static_cast<double>(array.size());
    double sq_dif = 0.f;
    for(auto i = array.begin(); i != array.end(); i++)
        sq_dif += std::pow((*i) - mean, 2);
    sq_dif /= (array.size() > 1) ? static_cast<double>(array.size() - 1) : 1.f;
    return std::sqrt(sq_dif);
}

void handle_shutdown(int s)
{
    std::cout << std::endl;
    QCoreApplication::exit(0);
}

int main(int argc, char** args)
{
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = handle_shutdown;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    QCoreApplication app(argc, args);
    QCommandLineParser parser;
    parser.setApplicationDescription("WITMOTION WT31N EXAMPLE DATA MONITOR");
    parser.addHelpOption();
    QCommandLineOption BaudRateOption(QStringList() << "b" << "baudrate",
                                      "Baudrate to set up the port",
                                      "QSerialPort::BaudRate set of values",
                                      "9600");
    QCommandLineOption DeviceNameOption(QStringList() << "d" << "device",
                                        "Port serial device name, without \'/dev\'",
                                        "tty",
                                        "ttyUSB0");
    QCommandLineOption CovarianceOption(QStringList() << "c" << "covariance",
                                        "Calculate noise covariance matrix");
    parser.addOption(BaudRateOption);
    parser.addOption(DeviceNameOption);
    parser.addOption(CovarianceOption);
    parser.process(app);

    bool measure_covariance = parser.isSet(CovarianceOption);
    if(measure_covariance)
    {
        std::cout << "ATTENTION!!! You selected COVARIANCE CONTROL."
                  << std::endl
                  << "Please set the sensor static for the time of measurement!"
                  << std::endl
                  << "The measurement will start in 10 seconds!"
                  << std::endl;
        sleep(10);
    }
    std::vector<float> accels_x, accels_y, accels_z;
    std::vector<float> pitches, rolls;

    QWitmotionWT31NSensor sensor(parser.value(DeviceNameOption),
                                 static_cast<QSerialPort::BaudRate>(parser.value(BaudRateOption).toInt()));
    sensor.setParent(dynamic_cast<QObject*>(&app));
    sensor.SetValidation(true); // Instructs the library to accept only the valid packets from sensor, to throw an error signal otherwise
    QObject::connect(&sensor, &QWitmotionWT31NSensor::ErrorOccurred, [](const QString& description){
        std::cout << "ERROR: " << description.toStdString() << std::endl;
        QCoreApplication::exit(1);
    }); // Connection of the error signal to the MOC compatible inlay lambda
    float ax = 0.0, ay = 0.0, az = 0.0, ex = 0.0, ey = 0.0, ez = 0.0;
    QObject::connect(&sensor, &QWitmotionWT31NSensor::AcquiredAccelerations,
                     [&ax, &ay, &az, &accels_x, &accels_y, &accels_z, &measure_covariance](float& x, float& y, float& z, float& t)
    {
        ax = x;
        ay = y;
        az = z;
        if(measure_covariance)
        {
            accels_x.push_back(x);
            accels_y.push_back(y);
            accels_z.push_back(z);
        }
    }); // Connection of the acceleration acquire signal to the MOC compatible mutable lambda
    QObject::connect(&sensor, &QWitmotionWT31NSensor::AcquiredAngles,
                     [&ax, &ay, &az, &ex, &ey, &ez, &pitches, &rolls, &measure_covariance](float& x, float& y, float& z, float& t)
    {
        ex = x;
        ey = y;
        ez = z;
        std::cout << "Acquired:" << std::endl
                  << "X: acceleration " << ax << " R angle " << ex * witmotion::DEG2RAD << std::endl
                  << "Y: acceleration " << ay << " P angle " << ey * witmotion::DEG2RAD << std::endl
                  << "Z: acceleration " << az << " Y angle " << ez * witmotion::DEG2RAD<< std::endl;
        if(measure_covariance)
        {
            rolls.push_back(ex);
            pitches.push_back(ey);
        }
    }); // Connection of the Euler angle acquire signal to the MOC compatible mutable lambda
    sensor.Start();
    int result = app.exec();
    if(measure_covariance)
    {
        std::cout << "Calculating noise covariance matrices..." << std::endl
                  << std::endl
                  << "Accelerations (for " << accels_x.size() << " measurements): " << std::endl
                  << "[\t" << variance(accels_x) << "\t0\t\0" << std::endl
                  << "\t0\t" << variance(accels_y) << "\t0" << std::endl
                  << "\t0\t0\t" << variance(accels_z) << "\t]" << std::endl
                  << std::endl
                  << "Angles (for " << pitches.size() << " measurements): " << std::endl
                  << "[\t" << variance(rolls) << "\t0\t\0" << std::endl
                  << "\t0\t" << variance(pitches) << "\t0" << std::endl
                  << "\t0\t0\t0\t]" << std::endl
                  << std::endl;
    }
    return result;
}
