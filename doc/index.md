# Witmotion Sensors UART Library {#index}
Core library allowing the external programs to read the data and control the parameters from the TTL/UART-compatible Inertial Measurement Units (IMU), manufactured by [WitMotion Shenzhen Co.,Ltd](https://www.wit-motion.com/).

The library implements the C++ API to program applications accessing Witmotion sensor devices supporting unified communication protocol. The class structure is defined to stay able to implement also any other protocol for the different hardware communication interface. In the current state, the library uses [Qt](https://qt.io) backend for UART/TTL serial interface through [Qt5 Serial Port](https://doc.qt.io/qt-5/qtserialport-index.html) internal API. Thus the **Qt library** installation of version 5.1 or newer is the only system requirement for the library. 

## Contents
- [Overview](\ref index)
- [List of official documentation and mirrors](\ref witmotion_docs)
- [Communication protocol description](\ref witmotion_protocol)
- [C++ API structure](\ref api_structure)
- [Installation](\ref installation)
- [Unified controller applications](\ref unified_applications)
- [Support policy for new devices](\ref support_policy)
- [Currently supported devices](\ref devices)
- [Contribution](\ref contrib)

## Device support policy {#support_policy}
The project strives to provide the following tools for every supported Witmotion device:
- Single-class based C++ API with dedicated dynamic library and set of header files;
- Dedicated controller tool allowing the user to set the device parameters and view the produced data through convenient command-line interface.

### Supported devices {#devices}
In its current state the library supports any Witmotion device using the [unified Witmotion communication protocol](\ref witmotion_protocol). The following table enumerates devices which are currently tested and **fully** supported.

| Device/Part ID | Description | Testing status |
|----------------|-------------|----------------|
|**WT31N**       | 3-Axis Accelerometer/Gyroscope (linear accelerations + 2-axis Euler angles gravity tracking) | Tested on baud rates 9600 and 115200 baud, polling frequencies up to 100 Hz |
|**JY901B**      | 9-Axis Combined IMU/Magnetometer/Altimeter (linear accelerations, angular velocities, Euler angles, magnetic field, barometry, altitude) | Tested on baud rates from 2400 to 115200 baud, polling frequencies up to 150 Hz |
|**WT901**      | 9-Axis Combined Open Circuit IMU/Magnetometer (linear accelerations, angular velocities, Euler angles, magnetic field) | Tested on baud rates from 2400 to 115200 baud, polling frequencies up to ~180 Hz |

## Contribution {#contrib}
The library is still a project under active development, so any user contribution is encouraged. Please do not hesitate to create pull requests [here](https://github.com/ElettraSciComp/witmotion_IMU_QT/pulls). Here below is the list of currently incompleted tasks presented with the corresponding Github issues.

### List of incompleted development tasks
- Writing documentation (ANY help is encouraged)
- [Create installation/packaging scripts for system-wide installation of the library](https://github.com/ElettraSciComp/witmotion_IMU_QT/issues/5)
- [Implementation of the subdirectory-based or submodule-based link for the existing projects](https://github.com/ElettraSciComp/witmotion_IMU_QT/issues/6)

