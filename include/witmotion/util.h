#ifndef WITMOTION_UTIL
#define WITMOTION_UTIL
#include "witmotion/types.h"

namespace witmotion
{

class witmotion_typed_packets
{
private:
    witmotion_datapacket array[32];
public:
    witmotion_datapacket& operator[](const witmotion_packet_id id);
};

class witmotion_typed_bytecounts
{
private:
    size_t array[32];
public:
    size_t& operator[](const witmotion_packet_id id);
};

bool id_registered(const size_t id);

inline float decode_acceleration(const int16_t* value);
inline float decode_angular_velocity(const int16_t* value, bool to_rad = true);
inline float decode_angle(const int16_t* value, bool to_rad = true);
inline float decode_temperature(const int16_t* value);
inline float decode(const witmotion_packet_id type, const int16_t* value, bool to_rad = true);
inline float decode(const uint8_t type, const int16_t* value, bool to_rad = true);
bool decode(const witmotion_datapacket* packet, float& X_val, float& Y_val, float& Z_val, float& T_val, bool to_rad = true);
inline void decode(const int16_t* data, const uint8_t type, float& X_val, float& Y_val, float& Z_val, float& T_val, bool to_rad = true);

}
#endif
