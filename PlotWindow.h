#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis>
#include <QVBoxLayout>
#include <map>

class PlotWindow : public QMainWindow {
    Q_OBJECT
public:
    virtual ~PlotWindow();
    void plotData(const std::map<double, double>&, const QString& title);

protected:
    explicit PlotWindow(QWidget *parent = Q_NULLPTR);
    QChart *chart;
    QChartView *chartView;
    QLineSeries *series;
    QValueAxis *axisY;
};

class PlotTransientData : public PlotWindow {
    Q_OBJECT
public:
    explicit PlotTransientData(QWidget *parent = Q_NULLPTR);
};

class PlotACData : public PlotWindow {
    Q_OBJECT
public:
    explicit PlotACData(QWidget *parent = Q_NULLPTR);
};

#endif // PLOTWINDOW_H