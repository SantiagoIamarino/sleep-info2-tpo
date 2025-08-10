#include "profilecreator.h"
#include "ui_profilecreator.h"
#include <QDebug>

ProfileCreator::ProfileCreator(ProfileDAO* dao, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProfileCreator)
{
    this->dao = dao;
    ui->setupUi(this);
    ui->profileNameError->hide();
}

ProfileCreator::~ProfileCreator()
{
    delete ui;
}

void ProfileCreator::on_createProfile_clicked()
{
    ui->profileNameError->hide();
    QString profileName = ui->profileName->text();

    if(profileName.length() >= 3) {
        this->dao->insertProfile(profileName);
        emit creatorClosed();
        this->close();
    } else {
        ui->profileNameError->show();
    }


}

