#ifndef PAGEHISTORIAL_H
#define PAGEHISTORIAL_H

#pragma once
#include <QWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

class PageHistorial : public QWidget {
    Q_OBJECT
    public:
        explicit PageHistorial(QWidget* parent = nullptr, int profileId = 1);

    private slots:
        void cargarHistorial();

    private:
        int profileId_ = 0;
        QComboBox* cbRango_;
        QComboBox* cbMetrica_;
        QDateEdit* deReferencia_;
        QPushButton* btActualizar_;

        QChartView* chartView_;
        QChart* chart_;
        QLineSeries* lineSeries_;
        QBarSeries* barSeries_;
        QBarCategoryAxis* axisX_;
        QValueAxis* axisY_;

        void refrescarChartLineal(const QVector<double>& yVals, const QString& titulo);
        void refrescarChartBarras(const QVector<double>& yVals, const QString& titulo);
        void obtenerHistorialData(int profileId, const QString& rango, const QString& metrica, QStringList& xLabels, QVector<double>& yVals);
};

#endif // PAGEHISTORIAL_H
