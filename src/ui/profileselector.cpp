#include "profileselector.h"
#include "ui_profileselector.h"
#include "../models/profile.h"
#include "../logic/profiledao.h"
#include "profilecreator.h"
#include <QSqlDatabase>
#include <QDebug>
#include <QPainter>
#include <QIcon>

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
        ui->stackedWidget->setCurrentWidget(ui->createProfilePage);
    } else {
        ui->stackedWidget->setCurrentWidget(ui->selectProfilePage);
        this->setupProfilesUI(profiles);
    }
}

void ProfileSelector::setupProfilesUI(QList<Profile> profiles) {
    ui->profilesList->setViewMode(QListView::IconMode);
    ui->profilesList->setFlow(QListView::TopToBottom);
    ui->profilesList->setMovement(QListView::Static);
    ui->profilesList->setResizeMode(QListView::Adjust);
    ui->profilesList->setIconSize(QSize(96, 96)); // tamaño del círculo
    ui->profilesList->setGridSize(QSize(160, 200)); // ancho x alto de cada celda
    ui->profilesList->setSpacing(12);
    ui->profilesList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->profilesList->setCursor(Qt::PointingHandCursor);
    ui->profilesList->setUniformItemSizes(true);
    ui->profilesList->setWordWrap(true);
    ui->profilesList->setStyleSheet(R"(
        QListWidget::item {
            border: 2px solid transparent;
            border-radius: 8px;
            padding: 4px;
        }
        QListWidget::item:hover {
            border: 2px solid #2196F3;
            background-color: rgba(33, 150, 243, 0.1);
        }
        QListWidget::item:selected {
            border: 2px solid #1976D2;
            background-color: rgba(25, 118, 210, 0.15);
        }
    )");

    ui->profilesList->clear();

    for (const Profile &prof : profiles) {
        QColor bg(180, 115, 115);
        const int D = 96;
        QPixmap pm(D, D);
        pm.fill(Qt::transparent);

        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing, true);

        // circulo de fondo
        p.setBrush(bg);
        p.setPen(Qt::NoPen);
        p.drawEllipse(pm.rect());

        // icono persona (emoji)
        QFont f; f.setPointSize(40);
        p.setFont(f);
        p.setPen(Qt::white);
        p.drawText(pm.rect(), Qt::AlignCenter, QString::fromUtf8("👤"));

        QIcon icon =  QIcon(pm);
        auto *item = new QListWidgetItem(icon, prof.name);
        item->setData(Qt::UserRole, prof.id);
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
        ui->profilesList->addItem(item);
    }

    // Boton de "+" ------------------
    QColor bg(180, 115, 115);
    const int D = 96;
    QPixmap pm(D, D);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);

    // circulo de fondo
    p.setBrush(bg);
    p.setPen(Qt::NoPen);
    p.drawEllipse(pm.rect());

    // icono persona (emoji)
    QFont f; f.setPointSize(40);
    p.setFont(f);
    p.setPen(Qt::white);
    p.drawText(pm.rect(), Qt::AlignCenter, QString::fromUtf8("➕"));

    QIcon icon =  QIcon(pm);
    auto *item = new QListWidgetItem(icon, "Nuevo perfil");
    item->setData(Qt::UserRole, "NEW");
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
    ui->profilesList->addItem(item);

    // Evento "click"

    connect(ui->profilesList, &QListWidget::itemClicked, this,
        [this](QListWidgetItem* item){
            if (!item) return;
            if(item->data(Qt::UserRole) == "NEW") { // click en nuevo perfil
                this->showCreateProfile();
                return;
            }

            const int id = item->data(Qt::UserRole).toInt();
            emit profileSelected(id);
    });
}

ProfileSelector::~ProfileSelector()
{
    delete ui;
    delete dao;
}

void ProfileSelector::showCreateProfile(){
    auto* creator = new ProfileCreator(this->dao, nullptr);

    // se ejecuta cuando en ProfileCreator se emite "creatorClosed"
    connect(creator, &ProfileCreator::creatorClosed, this, [this]() {
        this->show();
        this->getProfiles();
    });

    this->hide();
    creator->show();
}

void ProfileSelector::on_createProfileButton_clicked()
{
    this->showCreateProfile();
}

