#include "database.h"
#include <QDebug>

#define DB_PATH "/Users/santiagoiamarino/sleep-ui/sleep_data.db"

Database::Database(QObject *parent) : QObject(parent) {
    db = QSqlDatabase::addDatabase("QSQLITE");
}

Database::~Database() {
    if (db.isOpen()) {
        db.close();
    }
}

bool Database::connect() {
    db.setDatabaseName(DB_PATH);

    if (!db.open()) {
        qCritical() << "❌ Error abriendo la base de datos:" << db.lastError().text();
        return false;
    }

    qDebug() << "✅ Base de datos conectada:" << DB_PATH;
    return true;
}

bool Database::isOpen() const {
    return db.isOpen();
}

QSqlDatabase Database::getConnection() const {
    return db;
}

QSqlQuery Database::executeQuery(const QString& sql) const {
    QSqlQuery query(db);
    if (!query.exec(sql)) {
        qCritical() << "❌ Error ejecutando query:" << sql;
        qCritical() << "     Detalle:" << query.lastError().text();
    }
    return query;
}

QSqlQuery Database::prepareQuery(const QString& sql) const {
    QSqlQuery query(db);
    if (!query.prepare(sql)) {
        qCritical() << "❌ Error preparando query:" << sql;
        qCritical() << "     Detalle:" << query.lastError().text();
    }
    return query;
}
