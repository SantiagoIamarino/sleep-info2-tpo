#ifndef PAGEAJUSTES_H
#define PAGEAJUSTES_H

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QSqlQuery>
#include <QSqlError>

class PageAjustes : public QWidget {
    Q_OBJECT

public:
    explicit PageAjustes(QWidget* parent = nullptr, int profileId = 1);

signals:
    void configuracionGuardada(int horasSuenio, bool alarmaOn, bool luzOn);

private slots:
    bool obtenerConfigDB();
    void guardarCambios();
    void notificarLPC();

private:
    int profileId_;
    QSpinBox*  spHorasSuenio_ = nullptr;
    QCheckBox* chkAlarmaOn_   = nullptr;
    QCheckBox* chkLuzOn_      = nullptr;
};

#endif // PAGEAJUSTES_H
