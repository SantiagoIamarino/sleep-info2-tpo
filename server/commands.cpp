// ========================
// Procesamiento de mensajes
// ========================

#include "commands.h"
#include "info_fisio.h"

static int ESP32_fd = -1;
static sockaddr_in ESP32_addr{};

enum class TipoMensaje {
    INFO_FISIO,
    ACK_CONFIG,
    PING,
    REQ_CONFIG,
    CFG_UPDATE,
    UNKNOWN
};

TipoMensaje detectarTipo(const char* buf) {
    if (strncmp(buf, "<REQ_CONFIG>", 12) == 0)      return TipoMensaje::REQ_CONFIG;
    if (strncmp(buf, "<CFG_UPDATE:", 12) == 0)      return TipoMensaje::CFG_UPDATE;
    if (strncmp(buf, "<INFO_FISIO:", 12) == 0) return TipoMensaje::INFO_FISIO;
    if (strncmp(buf, "<ACK_REQ_CONFIG>", 16) == 0) return TipoMensaje::ACK_CONFIG;
    if (strncmp(buf, "<PING>", 6) == 0) return TipoMensaje::PING;
    return TipoMensaje::UNKNOWN;
}

static inline const char* b2s(bool b) { return b ? "0TRUE" : "FALSE"; }

bool responderConfigUDP(int sockfd, const sockaddr_in& dst, bool es_update) {
    sqlite3* db = db::Connection::instance().raw();
    if (!db) {
        const char* err = "<ERR:DB_NOT_OPEN>";
        sendto(sockfd, err, strlen(err), 0, (const sockaddr*)&dst, sizeof(dst));
        return false;
    }

    // Trae la config del perfil seleccionado
    const char* SQL =
        "SELECT ps.profile_id, sc.horas_suenio, sc.alarma_on, sc.luz_on "
        "FROM suenio_config sc "
        "JOIN profile_selected ps ON sc.profile_id = ps.profile_id "
        "LIMIT 1;";

    sqlite3_stmt* st = nullptr;
    int rc = sqlite3_prepare_v2(db, SQL, -1, &st, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = "<ERR:DB_PREPARE>";
        sendto(sockfd, err, strlen(err), 0, (const sockaddr*)&dst, sizeof(dst));
        return false;
    }

    bool ok = false;
    if ((rc = sqlite3_step(st)) == SQLITE_ROW) {
        int profile_id = sqlite3_column_int(st, 0);
        int horas   = sqlite3_column_int(st, 1);
        int alarma  = sqlite3_column_int(st, 2); // 0/1
        int luz     = sqlite3_column_int(st, 3); // 0/1

        // parsea profile_id a 2 digitos ej: 1 = 01
        char profile_id_str[3];
        profile_id_str[0] = '0' + (profile_id / 10);
        profile_id_str[1] = '0' + (profile_id % 10);
        profile_id_str[2] = '\0';

        // Normaliza rango
        if (horas < 0) horas = 0;
        if (horas > 20) horas = 20;

        char hs[3];
        hs[0] = '0' + (horas / 10);
        hs[1] = '0' + (horas % 10);
        hs[2] = '\0';

        // formato: <CFG:PF_ID=01;HORAS_SUENIO=08;ALARMA_ON=TRUE;LUZ_ON=FALSE>
        char payload[96];
        if(!es_update) {
            //  mandar un ACK primero
            const char* ack = "<ACK_REQ_CONFIG>";
            sendto(sockfd, ack, strlen(ack), 0, (const sockaddr*)&dst, sizeof(dst));
            snprintf(payload, sizeof(payload),
                 "<CFG:PF_ID=%s;HORAS_SUENIO=%s;ALARMA_ON=%s;LUZ_ON=%s>",
                 profile_id_str, hs, b2s(alarma != 0), b2s(luz != 0));
        } else {
            snprintf(payload, sizeof(payload),
                 "<CFG_UPDATE:PF_ID=%s;HORAS_SUENIO=%s;ALARMA_ON=%s;LUZ_ON=%s>",
                 profile_id_str, hs, b2s(alarma != 0), b2s(luz != 0));
        }

        ssize_t sent = sendto(sockfd, payload, strlen(payload), 0,
                              (const sockaddr*)&dst, sizeof(dst));
        ok = (sent > 0);
        printf("[UDP] Enviado config: %s\n", payload);
    } else {
        const char* err = "<ERR:NO_CONFIG>";
        sendto(sockfd, err, strlen(err), 0, (const sockaddr*)&dst, sizeof(dst));
    }

    sqlite3_finalize(st);
    return ok;
}

void procesarMensaje(const char* buf, int sockfd, const sockaddr_in& srcAddr) {
    switch (detectarTipo(buf)) {

        case TipoMensaje::REQ_CONFIG:
            std::cout << "[REQ_CONFIG] Configuracion solicitada por el dispositivo.\n";
            responderConfigUDP(sockfd, srcAddr);
            break;

        case TipoMensaje::CFG_UPDATE:
            std::cout << "[CFG_UPDATE] Actualizacion de configuracion recibida: " << buf << "\n";
            // Responder con la nueva config
            if(ESP32_fd != -1) {
                responderConfigUDP(ESP32_fd, ESP32_addr, true);
            }
            break;

        case TipoMensaje::INFO_FISIO: {
            if (validarInfoFisio(buf)) {
                guardarInfoFisio(buf);
                enviarLiveDataCliente(sockfd, buf);
            } else {
                std::cerr << "[WARN] Formato invalido en INFO_FISIO: " << buf << "\n";
            }
            break;
        }

        case TipoMensaje::ACK_CONFIG:
            std::cout << "[ACK] Configuracion confirmada.\n";
            break;

        case TipoMensaje::PING:
            std::cout << "[PING] Keepalive recibido.\n";
            // responder con un PONG
            {
                const char* pong = "<PONG>";
                sendto(sockfd, pong, strlen(pong), 0,
                       (const sockaddr*)&srcAddr, sizeof(srcAddr));
                ESP32_fd = sockfd;
                ESP32_addr = srcAddr;
            }
            break;

        case TipoMensaje::UNKNOWN:
        default:
            std::cerr << "[WARN] Comando desconocido: " << buf << "\n";
            break;
    }
}