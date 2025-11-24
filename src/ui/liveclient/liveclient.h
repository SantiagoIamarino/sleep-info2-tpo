#pragma once
#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

class LiveClient : public QObject {
    Q_OBJECT
public:
    explicit LiveClient(QObject* parent=nullptr)
        : QObject(parent), sock_(new QUdpSocket(this)) {}

    bool start(quint16 port = 6005, const QHostAddress& addr = QHostAddress::AnyIPv4) {
        if (!sock_->bind(addr, port, QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint))
            return false;
        connect(sock_, &QUdpSocket::readyRead, this, &LiveClient::onReadyRead);
        return true;
    }

    bool send(const QString& trama, const QHostAddress& host = QHostAddress::LocalHost, quint16 port = 5005);


signals:
    void newPPM(int ppm);
    void newLiveEvent(QString s);
    void posibleCaidaEvent();

private:
    void onReadyRead();
    QUdpSocket* sock_;
};
