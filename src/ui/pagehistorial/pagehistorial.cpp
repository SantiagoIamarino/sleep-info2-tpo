#include "pagehistorial.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDate>
#include <QRandomGenerator>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDateTime>
#include <QDebug>

const QTimeZone kLocalTz("America/Argentina/Buenos_Aires");

PageHistorial::PageHistorial(QWidget* parent, int profileId)
    : QWidget(parent), profileId_(profileId)
{
    // ---- Controles de filtro ----
    cbRango_      = new QComboBox(this);
    cbMetrica_    = new QComboBox(this);
    deReferencia_ = new QDateEdit(QDate::currentDate(), this);
    btActualizar_  = new QPushButton("Actualizar", this);

    cbRango_->addItems({"Día", "Semana"});
    cbMetrica_->addItems({"Horas de sueño", "PPM"});

    deReferencia_->setCalendarPopup(true);
    deReferencia_->setDisplayFormat("dd/MM/yyyy");

    auto* topBar = new QHBoxLayout;
    topBar->addWidget(cbRango_);
    topBar->addWidget(cbMetrica_);
    topBar->addWidget(deReferencia_);
    topBar->addStretch(1);
    topBar->addWidget(btActualizar_);

    // ---- Chart ----
    chart_     = new QChart();
    chartView_ = new QChartView(chart_);
    chartView_->setRenderHint(QPainter::Antialiasing);

    axisX_ = new QBarCategoryAxis();
    axisY_ = new QValueAxis();

    chart_->addAxis(axisX_, Qt::AlignBottom);
    chart_->addAxis(axisY_, Qt::AlignLeft);

    // layout principal
    auto* root = new QVBoxLayout(this);
    root->addLayout(topBar);
    root->addWidget(chartView_, 1);

    // señales
    connect(btActualizar_, &QPushButton::clicked, this, &PageHistorial::cargarHistorial);
    connect(cbRango_,      &QComboBox::currentIndexChanged, this, &PageHistorial::cargarHistorial);
    connect(cbMetrica_,    &QComboBox::currentIndexChanged, this, &PageHistorial::cargarHistorial);
    connect(deReferencia_, &QDateEdit::dateChanged,         this, &PageHistorial::cargarHistorial);

    // primer render
    cargarHistorial();
}

void PageHistorial::cargarHistorial()
{
    QString rango  = cbRango_->currentText();       // "Día" | "Semana"
    QString metrica= cbMetrica_->currentText();     // "Horas de sueño" | "PPM"

    QStringList xLabels;
    QVector<double> yVals;

    obtenerHistorialData(profileId_, rango, metrica, xLabels, yVals);

    chart_->removeAllSeries();
    axisX_->clear();
    // --- CASO SIN DATOS ---
    if (yVals.isEmpty()) {
        chart_->setTitle("No hay datos para mostrar.");
        axisY_->setTitleText("");
        axisY_->setRange(0, 1);          // algo neutro para que se vea el fondo
        return;                          // no seguimos armando series
    }
    axisX_->append(xLabels);

    if (metrica == "Horas de sueño") {
        refrescarChartBarras(yVals, "Horas de sueño");
        axisY_->setTitleText("hs");
        axisY_->setLabelFormat("%.1f");
    } else {
        refrescarChartLineal(yVals, "PPM promedio");
        axisY_->setTitleText("ppm");
        axisY_->setLabelFormat("%.0f");
    }

    // ajustar rangos Y
    if (!yVals.isEmpty()) {
        auto [minIt, maxIt] = std::minmax_element(yVals.begin(), yVals.end());
        double ymin = std::floor(*minIt);
        double ymax = std::ceil(*maxIt);
        if (ymin == ymax) { ymin -= 1; ymax += 1; }
        axisY_->setRange(ymin, ymax);
    }

    chart_->setTitle(QString("%1 - %2")
                         .arg(cbMetrica_->currentText(),
                              cbRango_->currentText()));
}

// --------- helpers de render ---------

void PageHistorial::refrescarChartLineal(const QVector<double>& yVals,
                                         const QString& titulo)
{
    auto* series = new QLineSeries();
    lineSeries_ = series;

    // Mapeamos X como índices (0..N-1) y luego usamos categories en el eje X.
    for (int i = 0; i < yVals.size(); ++i) {
        series->append(i, yVals[i]);
    }

    chart_->addSeries(series);
    series->attachAxis(axisX_);
    series->attachAxis(axisY_);
    chart_->setTitle(titulo);
}

void PageHistorial::refrescarChartBarras(const QVector<double>& yVals,
                                         const QString& titulo)
{
    auto* set = new QBarSet(titulo);
    for (double v : yVals) *set << v;

    auto* barSeries = new QBarSeries();
    barSeries_ = barSeries;
    barSeries->append(set);

    chart_->addSeries(barSeries);
    barSeries->attachAxis(axisY_);

    chart_->setTitle(titulo);
}

static inline QDate startOfWeekMonday(const QDate& d) {
    // 1..7 (1=lunes)
    int dow = d.dayOfWeek();
    return d.addDays(1 - dow);
}

static QString labelSemana(const QDate& weekStart) {
    QLocale loc(QLocale::Spanish, QLocale::Argentina);
    const QDate a = weekStart;
    const QDate b = weekStart.addDays(6);
    return QString("%1–%2").arg(loc.toString(a, "dd/MM"),
                                loc.toString(b, "dd/MM"));
}

static QString labelDia(const QDate& d) {
    const QDate hoy = QDate::currentDate();
    if (d == hoy)               return QStringLiteral("Hoy");
    if (d == hoy.addDays(-1))   return QStringLiteral("Ayer");
    QLocale loc(QLocale::Spanish, QLocale::Argentina);
    return loc.toString(d, "dd/MM");
}

static QString labelHora(int h) {
    return QString("%1").arg(h, 2, 10, QChar('0')); // "00".."23"
}

void PageHistorial::obtenerHistorialData(int profileId,
                                         const QString& rango,
                                         const QString& metrica,
                                         QStringList& xLabels,
                                         QVector<double>& yVals)
{
    xLabels.clear();
    yVals.clear();

    const bool esHorasSuenio = (metrica == "Horas de sueño");
    const QDate fechaReferencia = deReferencia_->date();

    if (!esHorasSuenio) {
        const QString columna = "ppm"; // cambiar a "spo2" u otra cuando corresponda

        if (rango == "Día") {
            // Promedio por HORA
            const QDateTime fromLocal(fechaReferencia, QTime(0,0), kLocalTz);
            const QDateTime toLocal   = fromLocal.addDays(1);
            const QDateTime fromUtc   = fromLocal.toUTC();
            const QDateTime toUtc     = toLocal.toUTC();

            QSqlQuery q;
            q.prepare(QStringLiteral(
                          "SELECT \"timestamp\", %1 AS val "
                          "FROM fisiologia_data "
                          "WHERE profile_id = :pid "
                          "  AND \"timestamp\" >= :from AND \"timestamp\" < :to "
                          "ORDER BY \"timestamp\" ASC;"
                          ).arg(columna));

            q.bindValue(":pid",  profileId);
            q.bindValue(":from", fromUtc.toString("yyyy-MM-dd HH:mm:ss"));
            q.bindValue(":to",   toUtc.toString("yyyy-MM-dd HH:mm:ss"));

            if (!q.exec()) {
                qWarning() << "[Historial] Error fisiologia (día):" << q.lastError().text();
                return;
            }

            // Buckets hora 0..23
            struct Acc { double sum=0; int n=0; };
            Acc acc[24];

            while (q.next()) {
                const double v    = q.value(1).toDouble();
                yVals << v;
            }

            for (int h = 0; h < 24; ++h) {
                if (acc[h].n > 0) {
                    xLabels << labelHora(h);
                }
            }
            return;
        }

        return;
    }

    // es horas de suenio------------------------------
    if (rango == "Día") {
        QSqlQuery q;
        q.prepare(
            "SELECT fecha_dia, horas_suenio "
            "FROM suenio_historial "
            "WHERE profile_id = ? "
            "AND fecha_dia >= ? "
            "ORDER BY fecha_dia ASC "
            "LIMIT 5;");
        q.addBindValue(profileId);
        q.addBindValue(fechaReferencia.toString("yyyy-MM-dd"));

        if (!q.exec()) {
            qWarning() << "[Historial] Error suenio (día):" << q.lastError().text();
            return;
        }

        while (q.next()) {
            const QDate  fecha = q.value(0).toDate();     // columna 0 = fecha_dia
            const double horas = q.value(1).toDouble();   // columna 1 = horas_suenio

            xLabels << labelDia(fecha);
            yVals   << horas;
        }

        return;
    }

    if(rango == "Semana") {
        // traigo 3 semanas antes y 3 despues
        const QDate from = fechaReferencia.addDays(-21);
        const QDate to   = fechaReferencia.addDays(+21);

        QSqlQuery q;
        q.prepare(
            "SELECT fecha_dia, horas_suenio "
            "FROM suenio_historial "
            "WHERE profile_id = ? "
            "  AND fecha_dia BETWEEN ? AND ? "
            "ORDER BY fecha_dia ASC;" );
        q.addBindValue(profileId);
        q.addBindValue(from.toString("yyyy-MM-dd"));
        q.addBindValue(to.toString("yyyy-MM-dd"));

        if (!q.exec()) {
            qWarning() << "[Historial] Error suenio (semana):" << q.lastError().text();
            return;
        }

        // labels en X
        QMap<QDate, QVector<double>> buckets;
        while (q.next()) {
            const QDate  d = q.value(0).toDate();
            const double h = q.value(1).toDouble();
            const QDate  wk = startOfWeekMonday(d);
            buckets[wk].push_back(h);
        }

        // Orden de claves
        QList<QDate> keys = buckets.keys();
        std::sort(keys.begin(), keys.end());

        // Ventana centrada en la semana de fechaReferencia
        const QDate centerKey = startOfWeekMonday(fechaReferencia);
        int centerIdx = std::lower_bound(keys.begin(), keys.end(), centerKey) - keys.begin();
        if (centerIdx >= keys.size() || keys[centerIdx] != centerKey) {
            // si no hay registros en la semana "central", ubico ventana donde iría
            keys.append(centerKey);
            std::sort(keys.begin(), keys.end());
            centerIdx = std::lower_bound(keys.begin(), keys.end(), centerKey) - keys.begin();
        }
        const int span = 3; // mas menos 3 semanas
        const int left  = qMax(0, centerIdx - span);
        const int right = qMin(int(keys.size()) - 1, centerIdx + span);

        // construyo
        for (int i = left; i <= right; ++i) {
            const QDate& k = keys[i];
            const auto it = buckets.constFind(k);
            if (it == buckets.cend() || it->isEmpty()) continue; // semana sin datos reales

            const QVector<double>& v = it.value();
            double sum = 0.0; for (double x : v) sum += x;
            const double avg = sum / v.size();

            xLabels << labelSemana(k);
            yVals   << avg;
        }
    }
}
