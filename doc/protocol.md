# Unified Witmotion communication protocol {#witmotion_protocol}

The communication protocol for all known Witmotion IMU sensor devices is unified. It bases on TTL UART communication circuits. Please refer to [official documentation](\ref witmotion_docs) for complete description. UART initiates connection on the baudrate which should be already set in the sensor's memory. 

## Connection mode
| Parameter | Value | `QSerialPort` config constant |
|-----------|-------|-------------------------|
|**Port mode** | Full Duplex | `QSerialPort::Direction::AllDirections` |
|**Start Bit** | 0 | `0x00` |
|**Stop Bit**  | 1 | `QSerialPort::OneStop` |
|**Hardware Flow Control**| Off | `QSerialPort::FlowControl::NoFlowControl` |

### Supported baud rates
The known sensors support the following baud rates (the non-standard rates unsupported in `QSerialPort` are noticed with *italic*):
- 2400
- 4800
- 9600 (default for open-circuit devices),
- 19200
- 38400
- 57600
- 115200 (default for enclosed devices with built-in USB/TTL transceiver)
- *230400*
- *256000*
- *460800*
- *921600*
\note The non-standard baudrated are not supported officially by `QSerialPort` backend. However, the [official Windows controller application](https://github.com/ElettraSciComp/witmotion_IMU_ros/issues/20#issuecomment-1369174406) by Witmotion allows to set up the device with these values. If this would be occasionally done, the Qt backend cannot connect to the device, and the user is strongly advised to reduce baud rate via the same Windows controller application to 115200 baud manually before proceed.

## Packet structures
The device throws out the *data packets* containing the measured values and accepts *configuration packets* to set up the parameters on-the-fly. The internal data representations for both types of packets are unified. For actual declarations refer to \ref types.h header file.

### Output data packet
The output data are organized in 11-byte sequential packets by the following principle:
| Offset | Length | Description |
|--------|--------|-------------|
|`0x00`| 1 | Magic header key, [WITMOTION_HEADER_BYTE](\ref witmotion::WITMOTION_HEADER_BYTE) |
|`0x01`| 1 | Data type ID, as defined in [witmotion_packet_id](\ref witmotion::witmotion_packet_id). All supported packet IDs should be enumerated in [witmotion_registered_ids](\ref witmotion::witmotion_registered_ids) list and described in [witmotion_packet_descriptions](\ref witmotion::witmotion_packet_descriptions) map in \ref types.h header file, otherwise the library will not consider it as available to support. |
|`0x02`| 8 | Payload. The payload is organized as sequential byte array wrapped into C-style union. It can store: \n - 8 8-bit *signed* integers \f$ \left[ -127 ... 128 \right] \f$ \n - 8 8-bit *unsigned* integers \f$ \left[ 0 ... 255 \right] \f$ \n - 4 16-bit *signed* integers \n - 2 32-bit *signed* integers |
|`0x0A`| 1 | Validation CRC. Calculated as a result of summation over all the **bytes**, not elements, as *unsigned* integers: \n \f$ crc = \sum_{i=0}^{i < 10}\times\f$`reinterpret_cast<uint8_t*>(payload) + i` |

The data packet structure is internally represented by [witmotion_datapacket](\ref witmotion::witmotion_datapacket) structure.

### Configuration data packet
Configuration packets are 5-bytes sequential structures organized as it is shown in the following table. CRC check is not implemented on the device.
| Offset | Length | Description |
|--------|--------|-------------|
|`0x00`| 1 | Magic header key, [WITMOTION_CONFIG_HEADER](\ref witmotion::WITMOTION_CONFIG_HEADER) |
|`0x01`| 1 | Magic configuration packet ID, [WITMOTION_CONFIG_KEY](\ref witmotion::WITMOTION_CONFIG_KEY) |
|`0x02`| 1 | Register ID, as defined in [witmotion_config_register_id](\ref witmotion::witmotion_config_register_id) |
|`0x03`| 2 | Payload. The set of 1 or 2 values that should be set on the device, represented as two 8-bit or one 16-bit unsigned integer |

The configuration packet has an internal representation in [witmotion_config_packet](\ref witmotion::witmotion_config_packet) structure.

## Data decoding algorithms and decoder functions
The type-specific descriptions for payload components encapsulated in output data packet, are placed in [witmotion_packet_id](\ref witmotion::witmotion_packet_id) enumeration documentation. The component decoder functions and packet parsers are located in \ref util.h header file. In the following table the actual measurements are enumerated with corresponding decoding rules and output types. The rules are defined here only for the cases when the special decoding is needed. Otherwise the values should be interpreted exactly as they are defined in [witmotion_packet_id](\ref witmotion::witmotion_packet_id) via direct copy.

Here and below, \f$ V \f$ is the **measured** value obtained from the device, \f$ D \f$ - **decoded** value, de-normalized from the sensor output. 
| Value | Unit | Decoding rule | Output type |
|-------|------|---------------|-------------|
| Acceleration | \f$ m/s^2 \f$ | \f$ D = 16\frac{V}{32768} \times 9.81 \f$ | float |
| Angular velocity | \f$ rad/s \f$ | \f$ D = \frac{V}{32768} \times 2000 \f$ | float |
| Euler angle | \f$ deg \f$ | \f$ D = \frac{V}{32768} \times 180 \f$ | float |
| Temperature | \f$ ^{\circ}C \f$ | \f$ D = \frac{V}{100} \f$ | float |
| Quaternion component |  | \f$ D = \frac{V}{32768} \f$ | float |
| Altimetry | \f$ m \f$ | \f$ D = \frac{V}{100} \f$ | float |
| GPS coordinate, rough/degrees part | \f$ deg \f$ | \f$ D = \frac{V}{10^7} \f$ | double-precision float |
| GPS coordinate, fine/minute part | \f$ min \f$ | \f$ D = \frac{V \mod 10^7}{10^5} \f$ | double-precision float |
| GPS altimetry | \f$ m \f$ | \f$ D = \frac{V}{10} \f$ | float |
| GPS angular velocity | \f$ rad/s \f$ | \f$ D = \frac{V}{10} \f$ | float |
| GPS ground speed | \f$ m/s \f$ | \f$ D = \frac{V}{10^3} \f$ | double-precision float |

\note Some values like temperature, require different coefficients on different sensors. These values require linear calibration. If you encounter this situation, please do not hesitate to open an [issue](https://github.com/ElettraSciComp/witmotion_IMU_QT/issues).

