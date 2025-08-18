#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>

//QT_CHARTS_USE_NAMESPACE

class PlotWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit PlotWindow(QWidget *parent = nullptr);
    ~PlotWindow();

    void plotData(const std::vector<double>& time, const std::vector<double>& values, const QString& title);

private:
    QChart *chart;
    QChartView *chartView;
    QLineSeries *series;
};

#endif // PLOTWINDOW_H