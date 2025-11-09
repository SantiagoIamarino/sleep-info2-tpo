#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

static const uint16_t UI_UDP_PORT  = 6005;


void sendJsonToUI(int fd, const std::string& json,
                         const std::string& ui_ip = "127.0.0.1",
                         uint16_t ui_port = UI_UDP_PORT);