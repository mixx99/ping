#ifndef IP_H
#define IP_H

#include "log.h"

#include <ws2tcpip.h>

#include <cstdint>
#include <vector>
#include <cstring>
#include <string>

namespace PING{

const constexpr int NUMBER_OF_32_BITS_BYTE = 4;

#pragma pack(push, 1)
struct IpHeader
{
  uint8_t version_and_header_len;
  uint8_t type;
  uint16_t len;
  uint16_t identificator;
  uint16_t flags_and_offset;
  uint8_t ttl;
  uint8_t proto_type;
  uint16_t crc_header;
  uint32_t sender_ip;
  uint32_t receiver_ip;
  // Parameters?
};
#pragma pack(pop)

struct Ip{
  IpHeader header;
  std::vector<uint8_t> data;
};

inline Ip deserialize_ipbuff(const std::vector<uint8_t>& buff)
{
  if(buff.size() < 20)
  {
    print_error("Wrong buffer size to deserialize ip.", 0);
    return {};
  }
  Ip result;
  IpHeader header;
  int header_len = (buff[0] & 0xF) * NUMBER_OF_32_BITS_BYTE;
  int full_len = (buff[2] << 8) | buff[3];
  if(header_len <= sizeof(header))
    memcpy(&header, buff.data(), header_len);
  else
  {
    PING::print_error("Failed to copy ip buffer. Wrong size.", 0);
    return {};
  }
  if(buff.size() < full_len)
  {
    print_error("Wrong buffer size.");
    return {};
  }
  result.data.resize(full_len - header_len);
  memcpy(result.data.data(), buff.data() + header_len, full_len - header_len);
  result.header = header;
  return result;
}

inline std::string print_ip(uint32_t ip) {
    in_addr addr;
    addr.S_un.S_addr = ip;
    char buf[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    return buf;
}

inline int get_header_len(const Ip& ip)
{
  return (ip.header.version_and_header_len & 0xF) * NUMBER_OF_32_BITS_BYTE;
}

} // namespace PING

#endif // IP_H

