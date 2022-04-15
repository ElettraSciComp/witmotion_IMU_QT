#include "witmotion/util.h"

#include <iostream>

namespace witmotion
{

bool id_registered(const size_t id)
{
    return (witmotion_registered_ids.find(id) != witmotion_registered_ids.end());
}


inline float decode_acceleration(const int16_t* value)
{
    return static_cast<float>(*value) / 32768.f * 16.f * 9.81;
}

inline float decode_angular_velocity(const int16_t* value, bool to_rad)
{
    return (static_cast<float>(*value) / 32768.f * 2000.f) * (to_rad ? (M_PI / 180.f) : 1.f);
}

inline float decode_angle(const int16_t* value, bool to_rad)
{
    return (static_cast<float>(*value) / 32768.f * 180.f) * (to_rad ? (M_PI / 180.f) : 1.f);
}

inline float decode_temperature(const int16_t* value)
{
    return static_cast<float>(*value) / 340.f;
}

inline float decode(const witmotion_packet_id type, const int16_t* value, bool to_rad)
{
    switch(type)
    {
    case pidAcceleration:
        return decode_acceleration(value);
    case pidAngularVelocity:
        return decode_angular_velocity(value, to_rad);
    case pidAngles:
        return decode_angle(value, to_rad);
    }
}

inline float decode(const uint8_t type, const int16_t* value, bool to_rad)
{
    return decode(static_cast<witmotion_packet_id>(type), value, to_rad);
}

bool decode(const witmotion_datapacket* packet, float& X_val, float& Y_val, float& Z_val, float& T_val, bool to_rad)
{
    if(packet->header_byte != WITMOTION_HEADER_BYTE)
        return false;
    uint8_t crc = packet->header_byte + packet->id_byte;
    bool valid;
    for(uint8_t i = 0; i < 8; i++)
        crc += packet->datastore.raw[i];
    valid = (crc == packet->crc);
    X_val = decode(packet->id_byte, packet->datastore.raw_cells, to_rad);
    Y_val = decode(packet->id_byte, packet->datastore.raw_cells + 1, to_rad);
    Z_val = decode(packet->id_byte, packet->datastore.raw_cells + 2, to_rad);
    T_val = decode_temperature(packet->datastore.raw_cells + 3);
    return valid;
}

inline void decode(const int16_t* data, const uint8_t type, float& X_val, float& Y_val, float& Z_val, float& T_val, bool to_rad)
{
    X_val = decode(type, data, to_rad);
    Y_val = decode(type, data + 1, to_rad);
    Z_val = decode(type, data + 2, to_rad);
    T_val = decode_temperature(data + 3);
}

witmotion_datapacket& witmotion_typed_packets::operator[](const witmotion_packet_id id)
{
    size_t int_id = static_cast<size_t>(id) - 0x50;
    return array[int_id];
}

size_t& witmotion_typed_bytecounts::operator[](const witmotion_packet_id id)
{
    size_t int_id = static_cast<size_t>(id) - 0x50;
    return array[int_id];
}

}
