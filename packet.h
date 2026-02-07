#ifndef PACKET_H
#define PACKET_H

#include "types.h"

// std
#include <cstdint>
#include <chrono>
#include <vector>

// WinApi
#include <combaseapi.h>

namespace PING
{

constexpr const int SIZE_OF_PACKET_DATA = sizeof(GUID);

enum packet_type
{
  REQUEST_ANSWER = 0,
  REQUEST = 8,
};

enum packet_code
{
  ECHO = 0,
};

#pragma pack(push, 1)
struct Packet
{
  uint8_t type;
  uint8_t code;
  uint16_t crc;
  uint8_t data[SIZE_OF_PACKET_DATA];
};
#pragma pack(pop)

struct PacketInfo
{
  sysclock_now send_timer;
  bool received;
  PacketInfo() : send_timer(sysclock::now()), received(false) {}
};

bool operator<(const Packet& first, const Packet& second);
uint16_t checksum(const void* data, int length);
void fill_icmp_packet(Packet& packet);
void deserialize(Packet& dest, const std::vector<uint8_t>& ip_packet);

} // namespace PING

#endif // PACKET_H
