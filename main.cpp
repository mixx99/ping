#include "packet.h"
#include "types.h"
#include "ip.h"
#include "help.h"
#include "log.h"

// std
#include <iostream>
#include <chrono>
#include <iomanip>
#include <map>

// WinAPI
#include <winsock.h>
#include <ws2tcpip.h>

enum WinErrors
{
    WOULDBLOCK = 10035,
};

bool is_recv_error_critical()
{
  if (WSAGetLastError() == WOULDBLOCK)
    return false;
  return true;
}

int main(int argc, char *argv[])
{
  int winsock_init_result = PING::init_winsock();
  if (winsock_init_result != 0)
  {
    PING::print_error("Failed to init WinSockets: " + std::to_string(winsock_init_result), 0);
    return -1;
  }
  if(argc != 2)
  {
    PING::print_error("Wrong arguments\n", 0);
    return -2;
  }

  std::cout << "Start pinging IP: " << argv[1] << '\n';

  int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if(sockfd == INVALID_SOCKET)
  {
    PING::print_error("Failed to create socket.");
    return -3;
  }
  int unblock_result = PING::unblock_socket(sockfd);
  if(unblock_result < 0)
  {
    PING::print_error("Failed to unblock socket");
    return -4;
  }

  struct sockaddr_in local_address;
  memset(&local_address, 0, sizeof(local_address));
  local_address.sin_family = AF_INET;
  local_address.sin_addr.s_addr = INADDR_ANY;

  struct sockaddr_in dest_address;
  memset(&dest_address, 0, sizeof(dest_address));
  dest_address.sin_family = AF_INET;
  dest_address.sin_addr.s_addr = inet_addr(argv[1]);

  bind(sockfd, (struct sockaddr *)&local_address, sizeof(local_address));
  // connect(sockfd, (struct sockaddr *)&dest_address, sizeof(dest_address));

  int ttl = 1;
  setsockopt(sockfd, IPPROTO_IP, IP_TTL, (char*)&ttl, sizeof(ttl));

  fd_set writefds;
  fd_set readfds;

  struct timeval timeout;
  memset(&timeout, 0, sizeof(timeout));
  timeout.tv_sec = 4;

  int sent_packet = 0;
  int failed_to_send = 0;
  int recv_packet = 0;
  int damaged_packet = 0;
  const auto RECV_MAX_TIMEOUT = PING::SECOND * 5;

  bool is_one_second_past = true;
  const int max_packets = 4;

  auto send_timer = sysclock::now();
  auto recv_timer = sysclock::now();

  std::map<PING::Packet, PING::PacketInfo> packets;
  while (true)
  {
    FD_ZERO(&writefds);
    FD_ZERO(&readfds);
    FD_SET(sockfd, &writefds);
    FD_SET(sockfd, &readfds);
    timeout.tv_sec = 4;

    if (select(0, &readfds, &writefds, nullptr, &timeout) == SOCKET_ERROR)
    {
      PING::print_error("Error from select");
      continue;
    }

    if (FD_ISSET(sockfd, &writefds) && sent_packet < max_packets && is_one_second_past)
    {
      PING::Packet packet_to_send;
      PING::fill_icmp_packet(packet_to_send);
      int len = sizeof(packet_to_send);
      // int ret_send = send(sockfd, reinterpret_cast<const char*>(&packet_to_send), len, 0);
      int ret_send = sendto(sockfd, (const char*)&packet_to_send, len, 0, (struct sockaddr*)&dest_address, sizeof(dest_address));
      if (ret_send == SOCKET_ERROR)
      {
        PING::print_error("Failed to send");
        failed_to_send++;
      }
      send_timer = sysclock::now();
      auto success = packets.insert(std::make_pair(packet_to_send, PING::PacketInfo()));
      if (success.second == false)
        PING::print_error("Something went wrong while sending a package. Package already sent.", 0);
      sent_packet++;
      is_one_second_past = false;
    }
    if (FD_ISSET(sockfd, &readfds))
    {
      int res_recv = 0;
      int total_received = 0;
      int received_buff_size = 1024;
      std::vector<uint8_t> received(received_buff_size);
      while ((res_recv = recv(sockfd, reinterpret_cast<char*>(received.data()) + total_received, received_buff_size - total_received, 0)) > 0)
      {
        total_received += res_recv;
        if (total_received > received_buff_size)
        {
          PING::print_error("Too big packet", 0);
          break;
        }
      }
      recv_timer = sysclock::now();
      recv_packet++;
      if (is_recv_error_critical())
        PING::print_error("Error from recv");

      received.resize(total_received);
      PING::Packet received_packet;
      PING::deserialize(received_packet, received);
      if (received_packet.type != PING::REQUEST_ANSWER || received_packet.code != PING::ECHO)
      {
        std::cerr << "Something with packet went wrong. Received packet type: " << static_cast<int>(received_packet.type) << ", Code: " << static_cast<int>(received_packet.code) << std::endl;
        damaged_packet++;
        PING::Ip ip;
        ip = PING::deserialize_ipbuff(received);
        std::cout << std::hex;
        std::cout << PING::print_ip(ip.header.receiver_ip) << std::endl;
        std::cout << PING::print_ip(ip.header.sender_ip) << std::endl;
        continue;
      }
      auto it = packets.find(received_packet);
      if (it == packets.end())
      {
        PING::print_error("Something went wrong. Received packet but can't find it", 0);
        continue;
      }
      if ((it->second.received) == true)
      {
        PING::print_error("Something went wrong. Package already received.", 0);
        continue;
      }
      auto duration = recv_timer - (it->second.send_timer);
      auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
      auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
      std::cout << "Received a packet with " << received.size() << " bytes. Time elapsed: " << sec << "." << micros << std::endl;
      it->second.received = true;
    }
    auto now = sysclock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>((now - send_timer)) > PING::SECOND)
      is_one_second_past = true;
    if (recv_packet == max_packets)
      break;
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - recv_timer) > RECV_MAX_TIMEOUT)
    {
      PING::print_error("Receive timeout.", 0);
      break;
    }
  }
  std::cout << std::endl;
  std::cout << "Result:" << std::endl;
  std::cout << "Sent packets: " << sent_packet - failed_to_send << std::endl;
  std::cout << "Received packets: " << recv_packet << std::endl;
  std::cout << "Lost: " << sent_packet - recv_packet << std::endl;
  std::cout << "Damaged: " << damaged_packet << std::endl;
  std::cout << "Failed to send: " << failed_to_send << std::endl;
  closesocket(sockfd);
  PING::winsock_cleanup();
}
