#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

class Database : public QObject {
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool connect();
    bool isOpen() const;
    QSqlDatabase getConnection() const;
    QSqlQuery executeQuery(const QString& sql) const;
    QSqlQuery prepareQuery(const QString& sql) const;

private:
    QSqlDatabase db;
};

#endif // DATABASE_H
