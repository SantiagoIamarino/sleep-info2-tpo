#include <cstring>
#include <cstdio>
#include <iostream>
#include "db.h"
#include <sqlite3.h>
#include <arpa/inet.h>   // inet_ntop
#include <sys/socket.h>  // sendto


// ========================

bool responderConfigUDP(int sockfd, const sockaddr_in& dst);
void procesarMensaje(const char* buf, int sockfd, const sockaddr_in& srcAddr);
