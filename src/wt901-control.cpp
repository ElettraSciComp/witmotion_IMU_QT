#include "witmotion/wt901-uart.h"

#include <QCommandLineParser>
#include <QCommandLineOption>

#include <iostream>
#include <iomanip>
#include <string>
#include <list>
#include <chrono>
#include <ctime>
#include <fstream>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

using namespace witmotion::wt901;

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
    parser.setApplicationDescription("WITMOTION WT901 STANDALONE SENSOR CONTROLLER/MONITOR");
    parser.addHelpOption();
    QCommandLineOption BaudRateOption(QStringList() << "b" << "baudrate",
                                      "Baudrate to set up the port",
                                      "9600 or 115200",
                                      "9600");
    QCommandLineOption IntervalOption(QStringList() << "i" << "interval",
                                      "Port polling interval",
                                      "50 ms",
                                      "50");
    QCommandLineOption DeviceNameOption(QStringList() << "d" << "device",
                                        "Port serial device name, without \'/dev\'",
                                        "ttyUSB0",
                                        "ttyUSB0");
    QCommandLineOption ValidateOption("validate",
                                      "Accept only valid datapackets");
    QCommandLineOption CalibrateOption("calibrate",
                                       "Run spatial calibration");
    QCommandLineOption SetBaudRateOption(QStringList() << "set-baudrate",
                                         "Reset the connection baud rate and polling interval",
                                         "9600 or 115200",
                                         "9600");
    QCommandLineOption SetPollingRateOption(QStringList() << "set-frequency",
                                            "Set output polling frequency",
                                            "10 or 100 Hz",
                                            "10");
    QCommandLineOption CovarianceOption("covariance",
                                        "Measure spatial covariance");
    QCommandLineOption LogOption("log", "Log acquisition to sensor.log file");
    parser.addOption(BaudRateOption);
    parser.addOption(IntervalOption);
    parser.addOption(DeviceNameOption);
    parser.addOption(ValidateOption);
    parser.addOption(CalibrateOption);
    parser.addOption(CovarianceOption);
    parser.addOption(SetBaudRateOption);
    parser.addOption(SetPollingRateOption);
    parser.addOption(LogOption);
    parser.process(app);

    QSerialPort::BaudRate rate;
    QString device;

    // Creating the sensor handler
    uint32_t interval = parser.value(IntervalOption).toUInt();
    if(interval < 5)
    {
        std::cout << "Wrong port polling interval specified, falling back to 50 ms!" << std::endl;
        interval = 50;
    }
    QWitmotionWT901Sensor sensor(device, rate, interval);
    sensor.SetValidation(parser.isSet(ValidateOption));

    QObject::connect(&sensor, &QWitmotionWT901Sensor::ErrorOccurred, [](const QString description)
    {
        std::cout << "ERROR: " << description.toStdString() << std::endl;
        QCoreApplication::exit(1);
    });

    int result = app.exec();

    return result;
}
