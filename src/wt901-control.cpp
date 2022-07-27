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
                                         "Reset the connection baud rate",
                                         "2400, 4800, 9600, 19200, 38400, 57600, 115200",
                                         "9600");
    QCommandLineOption SetPollingRateOption(QStringList() << "set-frequency",
                                            "Set output polling frequency, Hz",
                                            "-10 for 0.1, -2 for 0.5, 1-200, 0 for stop, -1 for single shot",
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

    QSerialPort::BaudRate rate = static_cast<QSerialPort::BaudRate>(parser.value(BaudRateOption).toUInt());
    QString device = parser.value(DeviceNameOption);

    // Creating the sensor handler
    uint32_t interval = parser.value(IntervalOption).toUInt();
    if(interval < 5)
    {
        std::cout << "Wrong port polling interval specified, falling back to 50 ms!" << std::endl;
        interval = 50;
    }
    QWitmotionWT901Sensor sensor(device, rate, interval);
    sensor.SetValidation(parser.isSet(ValidateOption));

    // Setting up data capturing slots: mutable/immutable C++14 lambda functions
    QObject::connect(&sensor, &QWitmotionWT901Sensor::ErrorOccurred, [](const QString description)
    {
        std::cout << "ERROR: " << description.toStdString() << std::endl;
        QCoreApplication::exit(1);
    });


    // Start acquisition
    sensor.Start();

    // Rendering control packets
    sleep(1);
    if(parser.isSet(CalibrateOption))
    {
        std::cout << "Entering CALIBRATION mode"
                  << std::endl
                  << "PLEASE KEEP THE SENSOR STATIC ON THE HORIZONTAL SURFACE!!!"
                  << std::endl
                  << "Calibration starts in "
                  << std::endl;
        for(size_t i = 5; i >= 1; i--)
        {
            std::cout << i << std::endl;
            sleep(1);
        }
        std::cout << std::endl << "Calibrating..." << std::endl;
        sensor.Calibrate();
        sensor.ConfirmConfiguration();
        sleep(1);
        std::cout << "Calibration completed. Please reconnect now" << std::endl;
        std::exit(0);
    }

    if(parser.isSet(SetBaudRateOption))
    {
        uint32_t new_rate = parser.value(SetBaudRateOption).toUInt();
        if(!((new_rate == 2400) ||
             (new_rate == 4800) ||
             (new_rate == 9600) ||
             (new_rate == 19200) ||
             (new_rate == 38400) ||
             (new_rate == 57600) ||
             (new_rate == 115200) ))
            std::cout << "ERROR: Wrong baudrate setting (use --help for detailed information). Ignoring baudrate reconfiguration request." << std::endl;
        else
        {
            std::cout << "Configuring baudrate. NOTE: Please reconnect the sensor after this operation with the proper baudrate setting!" << std::endl;
            sensor.SetBaudRate(static_cast<QSerialPort::BaudRate>(new_rate));
            sensor.ConfirmConfiguration();
            sleep(1);
            std::cout << "Reconfiguration completed. Please reconnect now" << std::endl;
            std::exit(0);
        }
    }

    if(parser.isSet(SetPollingRateOption))
    {
        int32_t new_poll = parser.value(SetPollingRateOption).toInt();
        if(!((new_poll == -10) ||
             (new_poll == -2) ||
             (new_poll == -1) ||
             (new_poll == 0) ||
             (new_poll == 1) ||
             (new_poll == 2) ||
             (new_poll == 5) ||
             (new_poll == 10) ||
             (new_poll == 20) ||
             (new_poll == 50) ||
             (new_poll == 100) ||
             (new_poll == 125) ||
             (new_poll == 200) ))
            std::cout << "ERROR: Wrong output frequency setting (use --help for detailed information). Ignoring output frequency reconfiguration request." << std::endl;
        else
        {
            std::cout << "Configuring output frequency. NOTE: Please reconnect the sensor after this operation with the proper setting!" << std::endl;
            sensor.SetPollingRate(new_poll);
            sensor.ConfirmConfiguration();
            sleep(1);
            std::cout << "Reconfiguration completed. Please reconnect now" << std::endl;
            std::exit(0);
        }
    }

    int result = app.exec();

    return result;
}
