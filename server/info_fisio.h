#include <cstring>
#include <cstdio>
#include <iostream>
#include "db.h"
#include <sqlite3.h>
#include <arpa/inet.h>   // inet_ntop
#include <sys/socket.h>  // sendto


bool validarInfoFisio(const char* buf);
bool guardarInfoFisio(const char* buf);