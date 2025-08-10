#include "src/ui/profileselector.h"
#include "src/ui/mainwindow.h"
#include "src/utils/database.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Database db;
    if (!db.connect()) {
        return -1;
    }

    ProfileSelector p(&db);
    p.show();

    QObject::connect(&p, &ProfileSelector::profileSelected,
        [&p, &db](int profileId){
            p.hide();
            auto mainWindow = new MainWindow(&db, profileId);
            mainWindow->show();
        });

    return a.exec();
}
