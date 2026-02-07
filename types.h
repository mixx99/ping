#ifndef TYPES_H
#define TYPES_H

#include <chrono>

using sysclock = std::chrono::system_clock;
using sysclock_now = decltype(sysclock::now()); // std::chrono::time_point<std::chrono::system_clock>;

#endif // TYPES_H
