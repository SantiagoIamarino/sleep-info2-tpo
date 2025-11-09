#include "pageajustes.h"
#include "../liveclient/liveclient.h"
#include <QMessageBox>

PageAjustes::PageAjustes(QWidget* parent, int profileId)
    : QWidget(parent), profileId_(profileId)
{
    auto* mainLayout = new QVBoxLayout(this);

    auto* title = new QLabel("Ajustes del Sistema");
    title->setStyleSheet("font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(title);

    // --- config ---
    auto* grp = new QGroupBox("Configuración de sueño");
    auto* form = new QFormLayout(grp);

    spHorasSuenio_ = new QSpinBox;
    spHorasSuenio_->setRange(3, 12);

    chkAlarmaOn_ = new QCheckBox("Activar alarma");
    chkLuzOn_ = new QCheckBox("Encender luz al despertar");

    form->addRow("Horas de sueño:", spHorasSuenio_);
    form->addRow("", chkAlarmaOn_);
    form->addRow("", chkLuzOn_);

    mainLayout->addWidget(grp);

    // --- guardar ---
    auto* btnGuardar = new QPushButton("Guardar cambios");
    connect(btnGuardar, &QPushButton::clicked, this, &PageAjustes::guardarCambios);
    mainLayout->addWidget(btnGuardar);

    mainLayout->addStretch(1);

    if(!obtenerConfigDB()) { // uso valores default
        spHorasSuenio_->setValue(8);
        chkAlarmaOn_->setChecked(true);
        chkLuzOn_->setChecked(true);
    }
}

bool PageAjustes::obtenerConfigDB()
{
    QSqlQuery q;
    q.prepare("SELECT horas_suenio, alarma_on, luz_on FROM suenio_config WHERE profile_id = ?");
    q.addBindValue(profileId_);

    if (!q.exec()) {
        qWarning() << "[DB] Error al obtener configuración:" << q.lastError().text();
        return false;
    }

    if (q.next()) {
        // Extrae los valores y los aplica a los widgets
        const int horas   = q.value(0).toInt();
        const bool alarma = q.value(1).toInt();
        const bool luz    = q.value(2).toInt();

        spHorasSuenio_->setValue(horas);
        chkAlarmaOn_->setChecked(alarma);
        chkLuzOn_->setChecked(luz);

        qDebug() << "[DB] Configuración cargada para perfil" << profileId_
                 << "→ horas:" << horas << "alarma:" << alarma << "luz:" << luz;
        return true;
    } else {
        qDebug() << "[DB] No hay configuración previa para profile_id =" << profileId_
                 << "(se usarán valores por defecto)";
    }

    return false;
}

void PageAjustes::notificarLPC() { // ej trama: <CFG_UPDATE:PF_ID=NN;HORAS_SUENIO=NN;ALARMA_ON=TRUE;LUZ_ON=FALSE>
    int horas = spHorasSuenio_->value();
    bool alarma = chkAlarmaOn_->isChecked();
    bool luz = chkLuzOn_->isChecked();

    QString pfIdStr = QString("%1").arg(profileId_, 2, 10, QChar('0'));
    QString horasStr = QString("%1").arg(horas, 2, 10, QChar('0'));

    // Construir la trama real
    QString trama = QString("<CFG_UPDATE:PF_ID=%1;HORAS_SUENIO=%2;ALARMA_ON=%3;LUZ_ON=%4>")
                        .arg(pfIdStr)
                        .arg(horasStr)
                        .arg(alarma ? "TRUE" : "FALSE")
                        .arg(luz ? "TRUE" : "FALSE");

    auto live = new LiveClient(this);
    live->start(5005);
    live->send(trama);
}

void PageAjustes::guardarCambios()
{
    int horas = spHorasSuenio_->value();
    bool alarma = chkAlarmaOn_->isChecked();
    bool luz = chkLuzOn_->isChecked();

    // guardar en DB
    QSqlQuery q;
    q.prepare(
        "INSERT OR REPLACE INTO suenio_config (profile_id, horas_suenio, alarma_on, luz_on) "
        "VALUES (?, ?, ?, ?);"
    );
    q.addBindValue(profileId_);
    q.addBindValue(horas);
    q.addBindValue(alarma);
    q.addBindValue(luz);

    if (!q.exec()) {
        qWarning() << "[DB] Error al guardar ajustes:" << q.lastError().text();
        return;
    }

    // emitir al server local
    notificarLPC(); // notifica a el LPC en caso de que este corriendo

    QMessageBox::information(this, "Configuración",
                             "Cambios guardados correctamente.\n"
                             "Horas de sueño: " + QString::number(horas) +
                                 "\nAlarma: " + (alarma ? "Sí" : "No") +
                                 "\nLuz: " + (luz ? "Sí" : "No"));
}
