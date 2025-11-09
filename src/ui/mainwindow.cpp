#include "mainwindow.h"
#include "pagehistorial/pagehistorial.h"
#include "pageajustes/pageajustes.h"
#include "liveclient/liveclient.h"
#include <QDebug>
#include <QListWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QFrame>

// Helper para separadores
static QFrame* hline() {
    auto* f = new QFrame;
    f->setFrameShape(QFrame::HLine);
    f->setFrameShadow(QFrame::Sunken);
    return f;
}

MainWindow::MainWindow(Database* db, int profileId, QWidget *parent)
    : QMainWindow(parent)
    , db_(db)
    , profileId_(profileId)
{
    qDebug() << "ProfileId: " << profileId_;

    auto* central = new QWidget(this);
    auto* root    = new QHBoxLayout(central);
    root->setContentsMargins(12,12,12,12);
    root->setSpacing(12);

    // Nav lateral
    nav_ = new QListWidget(central);
    nav_->setFixedWidth(200);
    nav_->addItem("Perfiles");
    nav_->addItem("Datos en vivo");
    nav_->addItem("Historial");
    nav_->addItem("Ajustes");
    nav_->setCurrentRow(1); // default: Datos en vivo
    connect(nav_, &QListWidget::currentRowChanged,
            this, &MainWindow::onNavChanged);

    // Páginas
    pages_ = new QStackedWidget(central);
    pagePerfiles_  = buildPerfilesPage();
    pageLive_      = buildLivePage();
    pageHistorial_ = new PageHistorial(this, profileId_);
    pageAjustes_   = new PageAjustes(this, profileId_);

    pages_->addWidget(pagePerfiles_);
    pages_->addWidget(pageLive_);
    pages_->addWidget(pageHistorial_);
    pages_->addWidget(pageAjustes_);
    pages_->setCurrentIndex(1);

    root->addWidget(nav_);
    root->addWidget(pages_, 1);
    setCentralWidget(central);
    setWindowTitle("Control de Sueño - TPO Info2");

    resize(1200, 800);
    setMinimumSize(1000, 700);

    this->StartLiveClient();
}

void MainWindow::StartLiveClient() {
    auto live = new LiveClient(this);
    if (live->start(6005)) {
        connect(live, &LiveClient::newPPM, this, [this](int ppm){ // PPM actualizacion en vivo
            lblPPM_->setText(QString("PPM: %1").arg(ppm));
        });
        connect(live, &LiveClient::newLiveEvent, this, [this](const QString& s){ // log en vivo
            logLive_->appendPlainText(s);
        });
    } else {
        logLive_->appendPlainText("ERROR: no se pudo iniciar LiveClient.");
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::onNavChanged(int row) {
    if (row == 0) {
        emit volverAPerfiles();
        this->close();
        return;
    }

    if (row >= 0 && row < pages_->count())
        pages_->setCurrentIndex(row);
}

QWidget* MainWindow::buildPerfilesPage() {
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    v->addWidget(new QLabel("Perfiles (UI a completar)"));
    v->addWidget(hline());
    v->addStretch(1);
    return page;
}

QWidget* MainWindow::buildLivePage() {
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);

    auto* title = new QLabel("Datos en vivo");
    title->setStyleSheet("font-size: 18px; font-weight: 600;");

    lblPPM_ = new QLabel("PPM: --");
    lblPPM_->setStyleSheet("font-size: 24px;");

    logLive_ = new QPlainTextEdit;
    logLive_->setReadOnly(true);
    logLive_->setPlaceholderText("Log de eventos...");

    v->addWidget(title);
    v->addWidget(hline());
    v->addWidget(lblPPM_);
    v->addWidget(new QLabel("Eventos / Mensajes:"));
    v->addWidget(logLive_, 1);
    return page;
}
