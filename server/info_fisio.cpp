#include "info_fisio.h"

bool validarInfoFisio(const char* buf) {
    // formato esperado: <INFO_FISIO:PF_ID=03;PPM=56>
    if (strncmp(buf, "<INFO_FISIO:", 12) != 0) return false;

    int profile_id = 0;
    if (sscanf(buf, "<INFO_FISIO:PF_ID=%d;", &profile_id) != 1) return false;
    if (profile_id < 1) return false;

    int ppm = 0;
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