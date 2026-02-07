#ifndef IP_H
#define IP_H

#include <cstdint>
#include <vector>
#include <cstring>
#include <iostream>

namespace PING{

const constexpr int NUMBER_OF_32_BITS_BYTE = 4;

struct Ip
{
  uint8_t version_and_header_len;
  uint8_t type;
  uint16_t len;
  uint16_t identificator;
  uint8_t flags;
  uint8_t offset[13];
  uint8_t ttl;
  uint8_t proto_type;
  uint16_t crc_header;
  uint32_t sender_ip;
  uint32_t receiver_ip;
  // Parameters?
  std::vector<uint8_t> data;
};

inline Ip deserialize_ipbuff(const std::vector<uint8_t>& buff)
{
  if(buff.size() < 4)
      return {};
  Ip result;
  int header_len = (buff[0] & 0xF) * NUMBER_OF_32_BITS_BYTE;
  int full_len = (buff[2] << 8) | buff[3];
  memcpy(&result, buff.data(), header_len);
  result.data.resize(full_len - header_len);
  memcpy(result.data.data(), buff.data() + header_len, full_len - header_len);
  return result;
}

inline int get_header_len(const Ip& ip)
{
  return (ip.version_and_header_len & 0xF) * NUMBER_OF_32_BITS_BYTE;
}

} // namespace PING

#endif // IP_H

