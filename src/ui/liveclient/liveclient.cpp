#include "liveclient.h"
#include <QDateTime>

void LiveClient::onReadyRead() {
    while (sock_->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(sock_->pendingDatagramSize()));
        sock_->readDatagram(datagram.data(), datagram.size());

        QJsonParseError perr{};
        QJsonDocument doc = QJsonDocument::fromJson(datagram, &perr);
        if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
            emit newLiveEvent(QStringLiteral("JSON inválido: %1").arg(QString::fromUtf8(datagram)));
            continue;
        }

        emit newLiveEvent(
            QStringLiteral("[%1] Live data recibida: %2")
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
                .arg(QString::fromUtf8(datagram))
            );

        // Formatos JSON {"type":"INFO_FISIO","ppm":123}
        QJsonObject obj = doc.object();
        const QString type = obj.value("type").toString();
        if(type == "INFO_FISIO") {
            int ppm = obj.value("ppm").toInt(-1);
            if (ppm >= 0) emit newPPM(ppm);
        }

        if(type == "POSIBLE_CAIDA") {
            emit posibleCaidaEvent();
        }

    }
}

bool LiveClient::send(const QString& trama,
                           const QHostAddress& host,
                           quint16 port)
{
    QByteArray data = trama.toUtf8();
    qint64 sent = sock_->writeDatagram(data, host, port);

    if (sent == -1) {
        qWarning() << "[LiveSender] Error al enviar:" << sock_->errorString();
        return false;
    }

    qDebug() << "[LiveSender] Trama enviada (" << sent << " bytes) →"
             << host.toString() << ":" << port << ":" << trama;
    return true;
}
