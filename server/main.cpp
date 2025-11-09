#include <arpa/inet.h>
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include "db.h"
#include "commands.h"

static const uint16_t UI_UDP_PORT  = 6005;

static volatile bool running = true;

void handle_sigint(int) {
    std::cout << "\n[!] Ctrl+C recibido, saliendo...\n";
    running = false;
}

int main(int argc, char** argv) {
    int port = 5005;
    if (argc >= 2) port = std::stoi(argv[1]);

    // Registrar handler de Ctrl+C ANTES de cualquier bloqueo
    std::signal(SIGINT, handle_sigint);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return 1;
    }

    std::cout << "[UDP] Escuchando en 0.0.0.0:" << port
              << "  (Ctrl+C para salir)\n";

    // Agrego timeout para que salga del recvfrom cada 1 segundo
    timeval tv{};
    tv.tv_sec = 1;  // 1 segundo
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    try {
        // Intentar abrir la base de datos
        db::Connection::instance().open("../sleep_data.db");

        // Verificar si se abrió correctamente
        if (db::Connection::instance().raw() != nullptr) {
            std::cout << "[OK] Conexión a la base de datos establecida correctamente ✅" << std::endl;
        } else {
            std::cerr << "[ERROR] La conexión a la base de datos no se pudo establecer ❌" << std::endl;
            return 1;
        }

    } catch (const db::DbError& e) {
        std::cerr << "[DB ERROR] " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }

    char buf[2048];
    while (running) {
        sockaddr_in src{};
        socklen_t srclen = sizeof(src);
        ssize_t n = recvfrom(fd, buf, sizeof(buf) - 1, 0,
                             (sockaddr*)&src, &srclen);

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;  // timeout → chequea si hay Ctrl+C
            else if (errno == EINTR)
                break;  // señal
            else {
                perror("recvfrom");
                continue;
            }
        }

        // hay mensaje
        buf[n] = '\0';
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &src.sin_addr, ip, sizeof(ip));
        std::cout << "----\nFrom " << ip << ":" << ntohs(src.sin_port)
                  << " | " << n << " bytes\n"
                  << buf << "\n";
        // Procesar mensaje...
        procesarMensaje(buf, fd, src);
    }

    std::cout << "\n[UDP] Server detenido.\n";
    close(fd);
    return 0;
}