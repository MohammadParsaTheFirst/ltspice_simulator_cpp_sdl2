#ifndef CHARTWINDOW_H
#define CHARTWINDOW_H

#include <QWidget>
namespace QtCharts {
    class QChartView;
    class QChart;
}

class ChartWindow : public QWidget {
    Q_OBJECT

public:
    explicit ChartWindow(QWidget *parent = Q_NULLPTR);

private:
    void setupChart();
};


#endif //CHARTWINDOW_H