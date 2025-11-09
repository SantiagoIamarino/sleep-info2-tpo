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

    // Cuando selecciona un perfil, paso a main
    QObject::connect(&p, &ProfileSelector::profileSelected,
    [&p, &db](int profileId){
        p.hide();
        auto mainWindow = new MainWindow(&db, profileId);
        mainWindow->setAttribute(Qt::WA_DeleteOnClose);
        mainWindow->show();

        // Cuando se clickea en "Perfiles", paso a el selector nuevamente
        QObject::connect(mainWindow, &MainWindow::volverAPerfiles,
             [mainWindow, &p]() {
                 mainWindow->close();
                 p.show();
        });
    });

    return a.exec();
}
