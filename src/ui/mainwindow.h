#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../utils/database.h"

class QListWidget;
class QStackedWidget;
class QLabel;
class QPlainTextEdit;
class Database;

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Database* db, int profileId, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNavChanged(int row);

private:
    Database* db_ = nullptr;
    int        profileId_ = -1;

    QListWidget*    nav_   = nullptr;
    QStackedWidget* pages_ = nullptr;

    QWidget* pagePerfiles_  = nullptr;
    QWidget* pageLive_      = nullptr;
    QWidget* pageHistorial_ = nullptr;
    QWidget* pageAjustes_   = nullptr;

    QLabel*        lblPPM_ = nullptr;       // para “datos en vivo”
    QPlainTextEdit* logLive_ = nullptr;

    QWidget* buildPerfilesPage();
    QWidget* buildLivePage();
    void StartLiveClient();

signals:
    void volverAPerfiles();
};

#endif // MAINWINDOW_H
