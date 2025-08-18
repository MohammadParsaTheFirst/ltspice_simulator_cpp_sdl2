#include "PlotWindow.h"

PlotWindow::PlotWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Transient Analysis Plot");
    resize(800, 600);

    chart = new QChart();
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    series = new QLineSeries();
    chart->addSeries(series);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Value");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    setCentralWidget(chartView);
}

PlotWindow::~PlotWindow() {
    delete series;
    delete chart;
}

void PlotWindow::plotData(const std::vector<double>& time, const std::vector<double>& values, const QString& title) {
    if (time.size() != values.size()) {
        // Handle error: sizes don't match
        return;
    }

    series->clear();
    for (size_t i = 0; i < time.size(); ++i) {
        series->append(time[i], values[i]);
    }

    chart->setTitle(title);
}
