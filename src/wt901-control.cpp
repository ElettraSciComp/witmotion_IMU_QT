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
    bool maintenance = false;

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

    std::vector<float> accels_x, accels_y, accels_z, vels_x, vels_y, vels_z, rolls, pitches, yaws, temps, times, mags_x, mags_y, mags_z;
    QObject::connect(&sensor, &QWitmotionWT901Sensor::Acquired,
                     [maintenance,
                     &accels_x,
                     &accels_y,
                     &accels_z,
                     &vels_x,
                     &vels_y,
                     &vels_z,
                     &rolls,
                     &pitches,
                     &yaws,
                     &temps,
                     &times,
                     &mags_x,
                     &mags_y,
                     &mags_z](const witmotion::witmotion_datapacket& packet)
    {
        if(maintenance)
            return;

        std::cout.precision(5);
        std::cout << std::fixed;

        float ax, ay, az, wx, wy, wz, roll, pitch, yaw, t, mx, my, mz;
        static size_t packets = 1;
        static auto time_start = std::chrono::system_clock::now();
        auto time_acquisition = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsed_seconds = time_acquisition - time_start;
        switch (static_cast<witmotion::witmotion_packet_id>(packet.id_byte))
        {
        case witmotion::pidAcceleration:
            witmotion::decode_accelerations(packet, ax, ay, az, t);
            accels_x.push_back(ax);
            accels_y.push_back(ay);
            accels_z.push_back(az);
            std::cout << packets << "\t"
                      << "Accelerations [X|Y|Z]:\t[ "
                      << ax << " | "
                      << ay << " | "
                      << az << " ], temp "
                      << t << " degrees,"
                      << " in " << elapsed_seconds.count() << " s"
                      << std::endl;
            break;
        case witmotion::pidAngularVelocity:
            witmotion::decode_angular_velocities(packet, wx, wy, wz, t);
            vels_x.push_back(wx);
            vels_y.push_back(wy);
            vels_z.push_back(wz);
            std::cout << packets << "\t"
                      << "Angular velocities [X|Y|Z]:\t[ "
                      << wx << " | "
                      << wy << " | "
                      << wz << " ], temp "
                      << t << " degrees,"
                      << " in " << elapsed_seconds.count() << " s"
                      << std::endl;
            break;
        case witmotion::pidAngles:
            witmotion::decode_angles(packet, roll, pitch, yaw, t);
            rolls.push_back(roll);
            pitches.push_back(pitch);
            yaws.push_back(yaw);
            std::cout << packets << "\t"
                      << "Euler angles [R|P|Y]:\t[ "
                      << roll << " | "
                      << pitch << " | "
                      << yaw << " ], temp "
                      << t << " degrees, "
                      << " in " << elapsed_seconds.count() << " s"
                      << std::endl;
            break;
        case witmotion::pidMagnetometer:
            witmotion::decode_magnetometer(packet, mx, my ,mz, t);
            mags_x.push_back(mx);
            mags_y.push_back(my);
            mags_z.push_back(mz);
            temps.push_back(t);
            std::cout << packets << "\t"
                      << "Magnetic field [X|Y|Z]:\t[ "
                      << mx << " | "
                      << my << " | "
                      << mz << " ], temp "
                      << t << " degrees, "
                      << " in " << elapsed_seconds.count() << " s"
                      << std::endl;
            break;
        default:
            break;
        }
        times.push_back(elapsed_seconds.count());

        packets++;
        time_start = time_acquisition;
    });


    // Start acquisition
    sensor.Start();

    // Rendering control packets
    maintenance = true;
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
        sensor.UnlockConfiguration();
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
            std::cout << "Configuring baudrate for " << new_rate << " baud. NOTE: Please reconnect the sensor after this operation with the proper baudrate setting!" << std::endl;
            sensor.UnlockConfiguration();
            sensor.SetBaudRate(static_cast<QSerialPort::BaudRate>(new_rate));
            sensor.ConfirmConfiguration();
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
            sensor.UnlockConfiguration();
            sensor.SetPollingRate(new_poll);
            sensor.ConfirmConfiguration();
            sleep(1);
            std::cout << "Reconfiguration completed. Please reconnect now" << std::endl;
            std::exit(0);
        }
    }
    maintenance = false;

    int result = app.exec();

    std::cout << "Average sensor return rate "
              << std::accumulate(times.begin(), times.end(), 0.f) / times.size()
              << " s" << std::endl << std::endl;

    if(parser.isSet(CovarianceOption))
    {
        std::cout << "Calculating noise covariance matrices..." << std::endl
                  << std::endl
                  << "Accelerations (total for " << accels_x.size() << " measurements): " << std::endl
                  << "[\t" << variance(accels_x) << "\t0.00000\t0.00000" << std::endl
                  << "\t0.00000\t" << variance(accels_y) << "\t0.00000" << std::endl
                  << "\t0.00000\t0.00000\t" << variance(accels_z) << "\t]" << std::endl
                  << std::endl
                  << "Angular velocities (total for " << vels_x.size() << " measurements): " << std::endl
                  << "[\t" << variance(vels_x) << "\t0.00000\t0.00000" << std::endl
                  << "\t0.00000\t" << variance(vels_y) << "\t0.00000" << std::endl
                  << "\t0.00000\t0.00000\t" << variance(vels_z) << "\t]" << std::endl
                  << std::endl
                  << "Angles (total for " << pitches.size() << " measurements): " << std::endl
                  << "[\t" << variance(rolls) << "\t0.00000\t0.00000" << std::endl
                  << "\t0.00000\t" << variance(pitches) << "\t0.00000" << std::endl
                  << "\t0.00000\t0.00000\t" << variance(yaws) << "\t]" << std::endl
                  << std::endl
                  << "Temperature (total for " << temps.size() << " measurements): " << std::endl
                  << "[\t" << variance(temps) << "\t]" << std::endl
                  << std::endl
                  << "Magnetometer (total for " << mags_x.size() << " measurements): " << std::endl
                  << "[\t" << variance(mags_x) << "\t00.00000\t00.00000" << std::endl
                  << "\t00.00000\t" << variance(mags_y) << "\t00.00000" << std::endl
                  << "\t00.00000\t00.00000\t" << variance(mags_z) << "\t]" << std::endl
                  << std::endl;
    }

    if(parser.isSet(LogOption))
    {
        std::cout << "Writing log file to sensor.log" << std::endl;
        std::fstream logfile;
        logfile.open("sensor.log", std::ios::out|std::ios::trunc);
        logfile.precision(5);
        logfile << std::fixed;
        logfile << "WITMOTION WT901 STANDALONE SENSOR CONTROLLER/MONITOR" << std::endl << std::endl;
        auto time_start = std::chrono::system_clock::now();
        std::time_t timestamp_start = std::chrono::system_clock::to_time_t(time_start);
        logfile << "Device /dev/" << device.toStdString() << " opened at " << static_cast<int32_t>(rate) << " baud" << std::endl;
        if(!accels_x.empty())
        {
            logfile << std::endl << "Spatial measurements (only full packets logged):" << std::endl;
            for(size_t i = 0; i < std::min(accels_x.size(),
                                           std::min(vels_x.size(), rolls.size()) ); i++)
            {
                logfile << i + 1 << "\t"
                        << "Accelerations [X|Y|Z]:\t[ "
                        << accels_x[i] << " | "
                        << accels_y[i] << " | "
                        << accels_z[i] << " ]"
                        << std::endl;
                logfile << i + 1 << "\t"
                        << "Angular velocities [X|Y|Z]:\t[ "
                        << vels_x[i] << " | "
                        << vels_y[i] << " | "
                        << vels_z[i] << " ]"
                        << std::endl;
                logfile << "\t"
                        << "Euler angles [R|P|Y]:\t[ "
                        << rolls[i] << " | "
                        << pitches[i] << " | "
                        << yaws[i] << " ]"
                        << std::endl;
            }
        }
        logfile << std::endl;
        if(!mags_x.empty())
        {
            logfile << std::endl << "Magnetic measurements:" << std::endl;
            for(size_t i = 0; i < mags_x.size(); i++)
            {
                logfile << i + 1 << "\t"
                        << "Accelerations [X|Y|Z]:\t[ "
                        << mags_x[i] << " | "
                        << mags_y[i] << " | "
                        << mags_z[i] << " ]"
                        << std::endl;
            }
            logfile << std::endl;
            logfile << std::endl << "Temperature measurements:" << std::endl;
            for(size_t i = 0; i < mags_x.size(); i++)
                logfile << i + 1 << "\t" << temps[i] << std::endl;
        }
        logfile << std::endl
                << "Acquired "
                << std::min(accels_x.size(),
                            std::min(vels_x.size(), rolls.size()) )
                << " measurements, average reading time "
                << std::accumulate(times.begin(), times.end(), 0.f) / times.size()
                << " s"
                << std::endl;
        if(parser.isSet(CovarianceOption))
        {
            logfile << "Calculating noise covariance matrices..." << std::endl
                    << std::endl
                    << "Accelerations (total for " << accels_x.size() << " measurements): " << std::endl
                    << "[\t" << variance(accels_x) << "\t0.00000\t0.00000" << std::endl
                    << "\t0.00000\t" << variance(accels_y) << "\t0.00000" << std::endl
                    << "\t0.00000\t0.00000\t" << variance(accels_z) << "\t]" << std::endl
                    << std::endl
                    << "Angular velocities (total for " << vels_x.size() << " measurements): " << std::endl
                    << "[\t" << variance(vels_x) << "\t0.00000\t0.00000" << std::endl
                    << "\t0.00000\t" << variance(vels_y) << "\t0.00000" << std::endl
                    << "\t0.00000\t0.00000\t" << variance(vels_z) << "\t]" << std::endl
                    << std::endl
                    << "Angles (total for " << pitches.size() << " measurements): " << std::endl
                    << "[\t" << variance(rolls) << "\t0.00000\t0.00000" << std::endl
                    << "\t0.00000\t" << variance(pitches) << "\t0.00000" << std::endl
                    << "\t0.00000\t0.00000\t" << variance(yaws) << "\t]" << std::endl
                    << std::endl
                    << "Temperature (total for " << temps.size() << " measurements): " << std::endl
                    << "[\t" << variance(temps) << "\t]" << std::endl
                    << std::endl
                    << "Magnetometer (total for " << mags_x.size() << " measurements): " << std::endl
                    << "[\t" << variance(mags_x) << "\t00.00000\t00.00000" << std::endl
                    << "\t00.00000\t" << variance(mags_y) << "\t00.00000" << std::endl
                    << "\t00.00000\t00.00000\t" << variance(mags_z) << "\t]" << std::endl
                    << std::endl;
        }
        logfile << "Acquisition performed at " << std::ctime(&timestamp_start) << std::endl;
        logfile.close();
    }

    return result;
}
