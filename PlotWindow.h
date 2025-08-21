#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis>
#include <QtCharts/QScatterSeries>
#include <QSlider>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QColorDialog>
#include <QInputDialog>
#include <QtCharts/QLegend>
#include <QtCharts/QLegendMarker>
#include <QStatusBar>

#include <map>
#include <limits>

class PlotWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit PlotWindow(QWidget *parent = Q_NULLPTR);
    void plotData(const std::map<double, double>&, const QString& title);

    void addSeries(const std::map<double, double>& data, const QString& seriesName);
    void clearAllSeries();

protected:
    void finalAxisSetup();
    QChart *chart;
    QChartView *chartView;
    QValueAxis *axisY;
    QScatterSeries *cursorSeries;
    QList<QLineSeries*> m_seriesList;

private slots:
    void verticalScaleChanged(int value);
    void horizontalScaleChanged(int value);
    void showContextMenu(const QPoint &pos);
    void changeSeriesColor();
    void renameSeries();
    void onSeriesClicked(QLineSeries* series, const QPointF& point);
    void clearCursor();
    void handleLegendClick();

private:
    void updateFullRange();

    QSlider *verticalSlider;
    QSlider *horizontalSlider;

    QPair<double, double> fullXRange;
    QPair<double, double> fullYRange;
    QLineSeries* m_activeSeries; 
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