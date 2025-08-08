#include "src/ui/profileselector.h"
#include "src/utils/database.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Database db;
    if (!db.connect()) {
        return -1;  // salir si falla la conexión
    }

    ProfileSelector p(&db);
    p.show();
    return a.exec();
}
