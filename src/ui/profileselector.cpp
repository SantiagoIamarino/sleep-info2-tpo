#include "profileselector.h"
#include "ui_profileselector.h"
#include "../models/profile.h"
#include "../logic/profiledao.h"
#include "profilecreator.h"
#include <QSqlDatabase>
#include <QDebug>

ProfileSelector::ProfileSelector(Database* db, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProfileSelector)
{
    dao = new ProfileDAO(db);
    ui->setupUi(this);
    this->getProfiles();
}

void ProfileSelector::getProfiles() {
    QList<Profile> profiles = dao->getAllProfiles();

    if (profiles.isEmpty()) {
        qDebug() << "No Hay perfiles creados";
        ui->messageLabel->setText("No hay perfiles creados, por favor crea uno a continuación.");
        ui->createProfileButton->setVisible(true);
    } else {
        ui->messageLabel->setVisible(false);
        ui->createProfileButton->setVisible(false);
        /*for (const Profile& p : profiles) {
            ui->comboBox->addItem(p.name, p.id);
        }
        ui->stackedWidget->setCurrentWidget(ui->profileSelectionPage);*/
    }
}

ProfileSelector::~ProfileSelector()
{
    delete ui;
    delete dao;
}

void ProfileSelector::on_createProfileButton_clicked()
{
    qDebug() << "Click";
    auto* creator = new ProfileCreator(this->dao, this);
    creator->show();
    //this->hide();
}

