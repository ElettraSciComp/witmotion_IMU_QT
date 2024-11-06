# Installation {#installation}

## Prerequisites
As the library relies on **Qt** backend, `Qt5::QSerialPort` package or the proper [Qt >= 5.2](https://qt.io) installation and [CMake](https://cmake.org) are required to build the library. In Debian-based distributions all the required software could be installed with the following command:
```sh
sudo apt install gcc g++ build-essential libqt5serialport5-dev cmake
```

## Build
The typical build script for the library looks like the following:
```sh
git clone https://github.com/ElettraSciComp/witmotion_IMU_QT.git witmotion-uart-qt
cd witmotion-uart-qt && mkdir build && cd build
cmake ..
make
```

## Install
### `noetic`
For ROS `noetic` distribution the package is available from the official buildfarm ,and it can be installed from APT:
```sh
sudo apt install witmotion_ros
```

