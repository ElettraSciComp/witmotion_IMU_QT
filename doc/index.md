# Witmotion Sensors UART Library {#index}
Core library anc C++ API for the inertial pose estimation units (IMUs) with TTL/UART hardware interface, manufactured by [WitMotion Shenzhen Co.,Ltd](https://www.wit-motion.com/).

The library implements the C++ API to program applications accessing Witmotion sensor devices supporting unified communication protocol. The class structure is defined to stay able to implement also any other protocol for the different hardware communication interface. In the current state, the library uses [Qt](https://qt.io) backend for UART/TTL serial interface through [Qt5 Serial Port](https://doc.qt.io/qt-5/qtserialport-index.html) internal API. Thus the **Qt library** installation of version 5.1 or newer is the only system requirement for the library.

## Contribution
The library is the project under active development, so any user contribution is encouraged. Please do not hesitate to create pull requests [here](https://github.com/ElettraSciComp/witmotion_IMU_QT/pulls). Here below is the list of currently incompleted tasks presented with the corresponding Github issues.

### List of incompleted development tasks
- Create installation/packaging scripts for system-wide installation of the library (https://github.com/ElettraSciComp/witmotion_IMU_QT/issues/5)
- Implementation of the subdirectory-based or submodule-based link for the existing projects (https://github.com/ElettraSciComp/witmotion_IMU_QT/issues/6)
