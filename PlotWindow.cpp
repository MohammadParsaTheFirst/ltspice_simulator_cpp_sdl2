#include "PlotWindow.h"

PlotWindow::PlotWindow(QWidget* parent) : QMainWindow(parent) {
    resize(800, 600);

    chart = new QChart();
    chart->legend()->setVisible(true);
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    series = new QLineSeries();
    chart->addSeries(series);

    axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    setCentralWidget(chartView);
}

PlotWindow::~PlotWindow() {
    delete series;
    delete chart;
    delete chartView;
    delete axisY;
}

void PlotWindow::plotData(const std::map<double, double>& results, const QString& title) {
    series->clear();
    series->setName(title);
    if (results.empty()) {
        chart->setTitle(title);
        return;
    }

    double yMin = results.begin()->second;
    double yMax = results.begin()->second;
    for (const auto& pair : results) {
        series->append(pair.first, pair.second);
        if (pair.second < yMin)
            yMin = pair.second;
        if (pair.second > yMax)
            yMax = pair.second;
    }

    chart->setTitle(title);

    auto axisX = chart->axes(Qt::Horizontal).first();
    axisX->setRange(results.begin()->first, results.rbegin()->first);
    if (yMin == yMax)
        axisY->setRange(yMin - 1, yMax + 1);
    else {
        double margin = (yMax - yMin) * 0.10;
        axisY->setRange(yMin - margin, yMax + margin);
    }
}

PlotTransientData::PlotTransientData(QWidget *parent) : PlotWindow(parent) {
    setWindowTitle("Transient Analysis Plot");

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY->setTitleText("Value (V or A)");
}

PlotACData::PlotACData(QWidget *parent) : PlotWindow(parent) {
    setWindowTitle("AC Sweep Plot");

    QLogValueAxis *axisX = new QLogValueAxis();
    axisX->setTitleText("Frequency");
    axisX->setLabelFormat("%g");
    axisX->setBase(10);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY->setTitleText("Magnitude");
}