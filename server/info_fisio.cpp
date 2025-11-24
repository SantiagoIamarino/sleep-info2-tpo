#include "info_fisio.h"



bool validarInfoFisio(const char* buf) {
    // formato esperado: <INFO_FISIO:PF_ID=03;PPM=56;CS=NN>
    if (!buf) return false;
    size_t len = strlen(buf);
    if (len < 20 || strncmp(buf, "<INFO_FISIO:", 12) != 0) return false;

    const char* cs_ptr = strstr(buf, ";CS=");
    if (!cs_ptr) return false;

    const char* end_ptr = strchr(cs_ptr, '>');
    if (!end_ptr || *(end_ptr+1) != '\0') return false;

   
    char cs_val[3] = {0};
    if (sscanf(cs_ptr, ";CS=%2[0-9A-Fa-f]>", cs_val) != 1) return false;
    
    // Calculo checksum
    const char* sum_start = buf + 1; // despues de '<'
    const char* sum_end = cs_ptr;
    unsigned int checksum = 0;
    for (const char* p = sum_start; p < sum_end; ++p) {
        checksum += static_cast<unsigned char>(*p);
    }
    checksum = checksum % 256;

    char computed_cs[3];
    snprintf(computed_cs, sizeof(computed_cs), "%02X", checksum);
    
    // Comparo checksums
    if (strcasecmp(cs_val, computed_cs) != 0) return false;

    //trama valida, checksum valido, obtengo valores
    int profile_id = 0, ppm = 0;
    if (sscanf(buf, "<INFO_FISIO:PF_ID=%d;PPM=%d;", &profile_id, &ppm) != 2) return false;
    
    if (strncmp(buf, "<INFO_FISIO:", 12) != 0) return false;

    if (sscanf(buf, "<INFO_FISIO:PF_ID=%d;", &profile_id) != 1) return false;
    if (profile_id < 1) return false;

    if (sscanf(buf, "<INFO_FISIO:PF_ID=%*d;PPM=%d>", &ppm) != 1) return false;
    if (!(ppm >= 30 && ppm <= 220)) return false;

    return true;
}

bool guardarInfoFisio(const char* buf) {
    int profile_id = 0;
    sscanf(buf, "<INFO_FISIO:PF_ID=%d;", &profile_id);

    int ppm = 0;
    sscanf(buf, "<INFO_FISIO:PF_ID=%*d;PPM=%d>", &ppm);

    sqlite3* db = db::Connection::instance().raw();
    if (!db) {
        return false;
    }

    printf("[INFO_FISIO] Guardando datos: PF_ID=%02d, PPM=%d\n", profile_id, ppm);

    const char* SQL =
        "INSERT INTO fisiologia_data (profile_id, ppm, timestamp) "
        "VALUES (?, ?, datetime('now'));";

    sqlite3_stmt* st = nullptr;
    int rc = sqlite3_prepare_v2(db, SQL, -1, &st, nullptr);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(st);
        return false;
    }

    sqlite3_bind_int(st, 1, profile_id);
    sqlite3_bind_int(st, 2, ppm);

    if ((rc = sqlite3_step(st)) != SQLITE_DONE) {
        sqlite3_finalize(st);
        return false;
    }

    sqlite3_finalize(st);
    return true;
}

bool enviarLiveDataCliente(int sockfd, const char* buf) {
    // crear JSON con los datos recibidos {"type":"INFO_FISIO","ppm":123}
    int ppm = 0;
    sscanf(buf, "<INFO_FISIO:PF_ID=%*d;PPM=%d>", &ppm);
    std::string json = "{\"type\":\"INFO_FISIO\",\"ppm\":" + std::to_string(ppm) + "}";
    sendJsonToUI(sockfd, json);
    return true;
}