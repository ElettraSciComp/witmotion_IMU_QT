#include "witmotion/util.h"

#include <iostream>

namespace witmotion
{

bool id_registered(const size_t id)
{
    return (witmotion_registered_ids.find(id) != witmotion_registered_ids.end());
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

/* COMPONENT DECODERS */
float decode_acceleration(const int16_t* value)
{
    return static_cast<float>(*value) / 32768.f * 16.f * 9.81;
}

float decode_angular_velocity(const int16_t* value)
{
    return (static_cast<float>(*value) / 32768.f * 2000.f);
}

float decode_angle(const int16_t* value)
{
    return (static_cast<float>(*value) / 32768.f * 180.f);
}

float decode_temperature(const int16_t* value)
{
    return static_cast<float>(*value) / 340.f;
}

float decode_orientation(const int16_t *value)
{
    return static_cast<float>(*value) / 32768.f;
}

/* PACKET DECODERS */
void decode_accelerations(const witmotion_datapacket &packet,
                          float &x,
                          float &y,
                          float &z,
                          float &t)
{
    if(static_cast<witmotion_packet_id>(packet.id_byte) != pidAcceleration)
        return;
    x = decode_acceleration(packet.datastore.raw_cells);
    y = decode_acceleration(packet.datastore.raw_cells + 1);
    z = decode_acceleration(packet.datastore.raw_cells + 2);
    t = decode_temperature(packet.datastore.raw_cells + 3);
}

void decode_angular_velocities(const witmotion_datapacket &packet,
                               float &x,
                               float &y,
                               float &z,
                               float &t)
{
    if(static_cast<witmotion_packet_id>(packet.id_byte) != pidAngularVelocity)
        return;
    x = decode_angular_velocity(packet.datastore.raw_cells);
    y = decode_angular_velocity(packet.datastore.raw_cells + 1);
    z = decode_angular_velocity(packet.datastore.raw_cells + 2);
    t = decode_temperature(packet.datastore.raw_cells + 3);
}

void decode_angles(const witmotion_datapacket &packet,
                   float &roll,
                   float &pitch,
                   float &yaw,
                   float &t)
{
    if(static_cast<witmotion_packet_id>(packet.id_byte) != pidAngles)
        return;
    roll = decode_angle(packet.datastore.raw_cells);
    pitch = decode_angle(packet.datastore.raw_cells + 1);
    yaw = decode_angle(packet.datastore.raw_cells + 2);
    t = decode_temperature(packet.datastore.raw_cells + 3);
}

void decode_magnetometer(const witmotion_datapacket &packet,
                         float &x,
                         float &y,
                         float &z,
                         float &t)
{
    if(static_cast<witmotion_packet_id>(packet.id_byte) != pidMagnetometer)
        return;
    x = static_cast<float>(packet.datastore.raw_cells[0]);
    y = static_cast<float>(packet.datastore.raw_cells[1]);
    z = static_cast<float>(packet.datastore.raw_cells[2]);
    t = decode_temperature(packet.datastore.raw_cells + 3);
}

void decode_gps(const witmotion_datapacket &packet,
                double &longitude_deg,
                double &longitude_min,
                double &latitude_deg,
                double &latitude_min)
{
    if(static_cast<witmotion_packet_id>(packet.id_byte) != pidGPSCoordinates)
        return;
    longitude_deg = static_cast<double>(packet.datastore.raw_large[0]) / 100000000.f;
    longitude_min = static_cast<double>(packet.datastore.raw_large[0] % 10000000) / 100000.f;
    latitude_deg = static_cast<double>(packet.datastore.raw_large[1]) / 100000000.f;
    latitude_min = static_cast<double>(packet.datastore.raw_large[1] % 10000000) / 100000.f;
}

void decode_gps_ground_speed(const witmotion_datapacket &packet,
                             float altitude,
                             float angular_velocity,
                             double ground_speed)
{
    if(static_cast<witmotion_packet_id>(packet.id_byte) != pidGPSGroundSpeed)
        return;
}

}
