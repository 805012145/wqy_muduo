#include "InetAddress.h"
#include <arpa/inet.h>
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <cstdio>
#include <iterator>
#include <netinet/in.h>
#include <string>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
namespace mymuduo {
    namespace base {
        InetAddress::InetAddress(uint16_t port, std::string ip) {
            bzero(&addr_, sizeof(addr_));
            addr_.sin_family = AF_INET;
            addr_.sin_port = htons(port);
            addr_.sin_addr.s_addr = inet_addr(ip.c_str());
        }
        std::string InetAddress::toIp() const {
            char buf[64] = {0};
            ::inet_ntop(AF_INET, &addr_.sin_addr.s_addr, buf, sizeof(buf));
            return buf;
        }
        std::string InetAddress::toIpPort() const {
            char buf[64] = {0};
            ::inet_ntop(AF_INET, &addr_.sin_addr.s_addr, buf, sizeof(buf));
            size_t end = strlen(buf);
            uint16_t port = ntohs(addr_.sin_port);
            sprintf(buf+end, ":%u", port);
            return buf;
        }
        uint16_t InetAddress::toPort() const {
            uint16_t port = ntohs(addr_.sin_port);
            return port;
        }
    }
}