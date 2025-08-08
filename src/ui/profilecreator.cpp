#include "profilecreator.h"
#include "ui_profilecreator.h"

ProfileCreator::ProfileCreator(ProfileDAO* dao, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProfileCreator)
{
    this->dao = dao;
    ui->setupUi(this);
}

ProfileCreator::~ProfileCreator()
{
    delete ui;
}
