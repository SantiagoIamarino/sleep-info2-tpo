// profiledao.cpp
#include "profiledao.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ProfileDAO::ProfileDAO(Database* db) : db(db) {}

QList<Profile> ProfileDAO::getAllProfiles() {
    QList<Profile> profiles;
    QSqlQuery query = db->executeQuery("SELECT id, name FROM profiles");

    while (query.next()) {
        profiles.append({ query.value(0).toInt(), query.value(1).toString() });
    }
    return profiles;
}

bool ProfileDAO::insertProfile(const QString& name) {
    QSqlQuery query = db->prepareQuery("INSERT INTO profiles (name) VALUES (?)");
    query.addBindValue(name);
    return query.exec();
}
