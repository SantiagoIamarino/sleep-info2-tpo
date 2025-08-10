#ifndef PROFILESELECTOR_H
#define PROFILESELECTOR_H

#include "../utils/database.h"
#include "../logic/profiledao.h"
#include <QWidget>
#include <QIcon>

namespace Ui {
class ProfileSelector;
}

class ProfileSelector : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileSelector(Database* db, QWidget *parent = nullptr);
    void getProfiles();
    void setupProfilesUI(QList<Profile> profiles);
    void showCreateProfile();
    ~ProfileSelector();

private:
    Ui::ProfileSelector *ui;
    ProfileDAO* dao;

private slots:
    void on_createProfileButton_clicked();

signals:
    void profileSelected(int profileId);
};

#endif // PROFILESELECTOR_H
