#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(Database* db, int profileId, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "ProfileId: " << profileId;
    // obtener data de la db aca como en profileselector con profiledao
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
