#ifndef LOG_H
#define LOG_H

// std
#include <iostream>

// WinAPI
#include <winsock.h>

namespace PING
{

inline void print_error(const std::string& error, bool print_wsa_last_error = true)
{
    if (print_wsa_last_error)
      std::cerr << error << ": "  << WSAGetLastError() << std::endl;
    else
      std::cerr << error << std::endl;
}

} // namespace PING

#endif // LOG_H
