#ifndef PROFILECREATOR_H
#define PROFILECREATOR_H

#include <QWidget>
#include "../logic/profiledao.h"

namespace Ui {
class ProfileCreator;
}

class ProfileCreator : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileCreator(ProfileDAO* dao, QWidget *parent = nullptr);
    ~ProfileCreator();

private:
    Ui::ProfileCreator *ui;
    ProfileDAO* dao;
};

#endif // PROFILECREATOR_H
