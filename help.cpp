#include "help.h"
#include "log.h"

// std
#include <cstdint>
#include <iostream>
#include <vector>
#include <cstring>

// WinAPI
#include <combaseapi.h>

namespace PING
{

static_assert(sizeof(GUID) == 16);

std::vector<uint8_t> get_guid()
{
  std::vector<uint8_t> result;
  GUID guid;
  HRESULT res = CoCreateGuid(&guid);
  if (res != S_OK)
  {
    print_error("Failed to get GUID");
    return result;
  }
  result.resize(sizeof(guid));
  memcpy(result.data(), &guid, sizeof(guid));
  return result;
}

int get_guid_from_buff(GUID& guid, const uint8_t *data, int size)
{
  if (size != sizeof(guid))
    return -1;
  memcpy(&guid, data, size);
  return 0;
}

int init_winsock()
{
  WSADATA wsaData;
  return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

int winsock_cleanup()
{
  int result = WSACleanup();
  if (result == SOCKET_ERROR)
    print_error("Failed to WSACleanup");
  return result;
}

int unblock_socket(int sockfd)
{
  int res;
  uint32_t ul = 1;
  res = ioctlsocket(sockfd, FIONBIO, (unsigned long*) &ul);
  if (res == SOCKET_ERROR)
  {
    print_error("Failed to make socket unblock");
    return -1;
  }
  return 0;
}

} // namespace PING
