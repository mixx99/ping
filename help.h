#ifndef HELP_H
#define HELP_H

// std
#include <vector>
#include <cstdint>
#include <chrono>

// WinAPI
#include <combaseapi.h>

namespace PING
{

constexpr const auto SECOND = std::chrono::seconds(1);

std::vector<uint8_t> get_guid();
int init_winsock();
int winsock_cleanup();
int unblock_socket(int sockfd);
int get_guid_from_buff(GUID& guid, const uint8_t *data, int size);

} // namespace PING

#endif // HELP_H
