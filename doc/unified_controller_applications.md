# Unified controller applications {#unified_applications}
These applications are convenience controllers allowing the user to determine the working mode and current setting of his Witmotion sensor device.

## Message enumerator {#message_enumerator}
The `message_enumerator` controller application is always built by default along with the library. It allows the user to capture and enumerate every [output data packet](\ref witmotion_protocol) produced by the sensor device connected to the given UART port. It tries to find if every incoming packet ID is registered in [witmotion_registered_ids](\ref witmotion::witmotion_registered_ids) list. If the packet ID is found, the packet will be logged and mapped with the corresponding description string from [witmotion_packet_descriptions](\ref witmotion::witmotion_packet_descriptions) table. After the program will be stopped by the user with `Ctrl+C` keystroke, all the captured packets will be enumerated with their descriptions, and the unknown IDs will be also reported and enumerated.

### Usage
```
message-enumerator [options]
```

#### Options
| Name | Default value | Description |
|------|---------------|-------------|
| `-h` `--help` | | Displays unified `QCommandLineParser` help message |
| `-b` `--baudrate` | 9600 | Baudrate, the actual values are enumerated in [`QSerialPort::BaudRate`](https://doc.qt.io/qt-5/qserialport.html#BaudRate-enum):\n- 1200\n- 2400\n- 4800\n- 9600\n- 19200\n- 38400\n- 57600\n- 115200 |
| `-d` `--device` | `ttyUSB0` | Serial device file name within the `/dev` system directory |
| `-p` `--poll` | `50` | Rate [ms] on which the application polls the sensor to retrieve packets |
| `-f` `--log-file` | | Log file name. Instructs the application to record all the retrieved packets and report them into the specified file |

#### Output
The following example is retrieved using **JY901B** sensor with 20 Hz output frequency and enabled quaternion-based orientation encoding, connected to `/dev/ttyUSB0` device endpoint on 9600 baud, and 50 ms polling rate. Measurement duration is about 10 sec.
\code{.sh}
message-enumerator --device ttyUSB0 --poll 50 --baudrate 9600
Press Ctrl+C to stop enumeration and see the report
Opening device "ttyUSB0" at 9600 baud
Instantiating timer at 50 ms
^C
Closing TTL connection

WITMOTION UART MESSAGE ENUMERATOR BY TWDRAGON

Acquired at Fri, 24 Feb 2023 г. 16:47:22 CET

ID	Qty	Description

0x50	32	Real Time Clock
0x51	31	Accelerations
0x52	32	Angular velocities
0x53	32	Spatial orientation (Euler angles)
0x54	32	Magnetometer/Magnetic orientation
0x56	32	Barometry/Altimeter
0x59	32	Spatial orientation (Quaternion)

	Unknown IDs: 0 [  ] 
Total messages: 223
\endcode

