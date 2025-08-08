
#ifndef PROFILEDAO_H
#define PROFILEDAO_H

#include "../models/profile.h"
#include <QList>
#include "../utils/database.h"

class ProfileDAO {
public:
    explicit ProfileDAO(Database* db);
    QList<Profile> getAllProfiles();
    bool insertProfile(const QString& name);

private:
    Database* db;
    Profile currentProfile;
};

#endif // PROFILEDAO_H
