#include "packet.h"
#include "log.h"
#include "ip.h"
#include "help.h"

// std
#include <cstdint>
#include <string>
#include <cstring>

namespace PING
{

bool operator<(const Packet& first, const Packet& second)
{
  GUID first_guid, second_guid;
  int res1 = get_guid_from_buff(first_guid, first.data, sizeof(first.data));
  int res2 = get_guid_from_buff(second_guid, second.data, sizeof(second.data));

  if (res1 != 0 || res2 != 0)
    print_error("Failed to get GUID from buffer. Different size of guid and packet data size.", 0);

  if (first_guid.Data1 != second_guid.Data1)
    return first_guid.Data1 < second_guid.Data1;
  if (first_guid.Data2 != second_guid.Data2)
    return first_guid.Data2 < second_guid.Data2;
  if (first_guid.Data3 != second_guid.Data3)
    return first_guid.Data3 < second_guid.Data3;

  for (size_t i = 0; i < sizeof(first_guid.Data4); i++)
  {
    if (first_guid.Data4[i] != second_guid.Data4[i])
      return first_guid.Data4[i] < second_guid.Data4[i];
  }
  return false;
}

uint16_t checksum(const void* data, int length)
{
    uint32_t sum = 0;
    const uint16_t* ptr = (uint16_t*)data;
    for (; length > 1; length -= 2)
    {
      sum += *ptr++;
      if (sum & 0x80000000)
      {
          sum = (sum & 0xFFFF) + (sum >> 16);
      }
    }
    if (length == 1)
    {
      sum += *(uint8_t*)ptr;
    }
    while (sum >> 16)
    {
      sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

void fill_icmp_packet(Packet& packet)
{
  packet.type = REQUEST;
  packet.code = ECHO;
  std::vector<uint8_t> guid = get_guid();
  for (size_t i = 0; i < SIZE_OF_PACKET_DATA && i < guid.size(); ++i)
    packet.data[i] = guid[i];
  packet.crc = 0;
  packet.crc = checksum(&packet, sizeof(packet));
}

void deserialize(Packet& dest, const std::vector<uint8_t>& ip_packet)
{
  const int ip_packet_offset = get_header_len(deserialize_ipbuff(ip_packet));
  dest.type = ip_packet[0 + ip_packet_offset];
  dest.code = ip_packet[1 + ip_packet_offset];
  dest.crc = (ip_packet[2 + ip_packet_offset] << 8) | (ip_packet[3 + ip_packet_offset] & 0xFF);
  memcpy(&dest.crc, &ip_packet[2 + ip_packet_offset], 2);
  dest.crc = 123;
  for (size_t i = 0, j = 4 + ip_packet_offset; i < SIZE_OF_PACKET_DATA && j < ip_packet.size(); ++j, ++i)
    dest.data[i] = ip_packet[j];
}

} // namespace PING
