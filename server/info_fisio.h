#include <cstring>
#include <cstdio>
#include <iostream>
#include "db.h"
#include <sqlite3.h>
#include <arpa/inet.h>   // inet_ntop
#include <sys/socket.h>  // sendto
#include "client_con.h"


bool validarInfoFisio(const char* buf);
bool guardarInfoFisio(const char* buf);
bool enviarLiveDataCliente(int sockfd, const char* buf);