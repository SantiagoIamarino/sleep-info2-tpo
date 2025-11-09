#include "client_con.h"

void sendJsonToUI(int fd, const std::string& json,
                         const std::string& ui_ip,
                         uint16_t ui_port) {
                            printf("Sending JSON to UI: %s\n", json.c_str());
    sockaddr_in dst{};
    dst.sin_family = AF_INET;
    dst.sin_port   = htons(ui_port);
    inet_pton(AF_INET, ui_ip.c_str(), &dst.sin_addr);

    sendto(fd, json.data(), json.size(), 0,
           reinterpret_cast<sockaddr*>(&dst), sizeof(dst));
}