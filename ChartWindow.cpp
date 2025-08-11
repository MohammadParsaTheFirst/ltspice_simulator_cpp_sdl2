#include "ChartWindow.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>

ChartWindow::ChartWindow(QWidget* parent) : QWidget(parent)
{
    this->setWindowTitle("Chart window");
    this->resize(500, 350);

    setupChart();
}

void ChartWindow::setupChart() {
    QLineSeries *series = new QLineSeries();
    series->append(0, 6);
    series->append(2, 4);
    series->append(3, 8);
    series->append(7, 4);
    series->append(10, 5);
    for (double i = 11; i <= 20; i += 0.5) {
        series->append(i, qSin(i));
    }

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->setTitle("Sample chart");

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("time");
    axisX->setLabelFormat("%.1f");
    axisX->setTickCount(10);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Voltage");
    axisY->setLabelFormat("%.1f V");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chartView);
    this->setLayout(layout);
}