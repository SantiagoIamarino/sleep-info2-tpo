#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdio>

void addChecksum(char* buf) {
    uint8_t checksum = 0;
    for (size_t i = 1; i < strlen(buf); ++i) {
        checksum += static_cast<uint8_t>(buf[i]);
    }
    strcat(buf, ";CS=");
    char checksum_str[4];
    snprintf(checksum_str, sizeof(checksum_str), "%02X", checksum);
    strcat(buf, checksum_str);
    strcat(buf, ">");
}