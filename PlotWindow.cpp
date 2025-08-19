#include "PlotWindow.h"

PlotWindow::PlotWindow(QWidget* parent) : QMainWindow(parent) {
    resize(800, 600);

    QWidget *centralWidget = new QWidget(this);
    QGridLayout *mainLayout = new QGridLayout(centralWidget);

    verticalSlider = new QSlider(Qt::Vertical);
    verticalSlider->setRange(10, 400);
    verticalSlider->setValue(100);
    verticalSlider->setToolTip("Vertical Zoom");

    horizontalSlider = new QSlider(Qt::Horizontal);
    horizontalSlider->setRange(10, 400);
    horizontalSlider->setValue(100);
    horizontalSlider->setToolTip("Horizontal Zoom");

    chart = new QChart();
    chart->legend()->setVisible(true);
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    series = new QLineSeries();
    chart->addSeries(series);
    axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    mainLayout->addWidget(verticalSlider, 0, 0);
    mainLayout->addWidget(chartView, 0, 1);
    mainLayout->addWidget(horizontalSlider, 1, 1);
    mainLayout->setColumnStretch(1, 100);
    mainLayout->setRowStretch(0, 100);

    connect(verticalSlider, &QSlider::valueChanged, this, &PlotWindow::verticalScaleChanged);
    connect(horizontalSlider, &QSlider::valueChanged, this, &PlotWindow::horizontalScaleChanged);

    chartView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(chartView, &QChartView::customContextMenuRequested, this, &PlotWindow::showContextMenu);

    cursorSeries = new QScatterSeries();
    cursorSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    cursorSeries->setMarkerSize(10.0);
    cursorSeries->setColor(Qt::red);
    chart->addSeries(cursorSeries);

    auto axisX = chart->axes(Qt::Horizontal).first();
    if (axisX) {
        cursorSeries->attachAxis(axisX);
    }
    cursorSeries->attachAxis(axisY);
    connect(series, &QLineSeries::clicked, this, &PlotWindow::onSeriesClicked);
    statusBar()->show();

    setCentralWidget(centralWidget);
}

void PlotWindow::plotData(const std::map<double, double>& results, const QString& title) {
    series->clear();
    series->setName(title);
    cursorSeries->clear();
    statusBar()->clearMessage();
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

    fullXRange = {results.begin()->first, results.rbegin()->first};
    fullYRange = {yMin, yMax};

    horizontalSlider->setValue(100);
    verticalSlider->setValue(100);

    horizontalScaleChanged(100);
    verticalScaleChanged(100);

    axisY->setGridLineVisible(true);
}

void PlotWindow::verticalScaleChanged(int value) {
    if (fullYRange.first == fullYRange.second) {
        axisY->setRange(fullYRange.first - 1, fullYRange.second + 1);
        return;
    }
    double scaleFactor = value / 100.0;
    double centerY = (fullYRange.first + fullYRange.second) / 2.0;
    double newHeight = (fullYRange.second - fullYRange.first) / scaleFactor;
    axisY->setRange(centerY - newHeight / 2.0, centerY + newHeight / 2.0);
}

void PlotWindow::horizontalScaleChanged(int value) {
    if (fullXRange.first == fullXRange.second)
        return;
    double scaleFactor = value / 100.0;
    double centerX = (fullXRange.first + fullXRange.second) / 2.0;
    double newWidth = (fullXRange.second - fullXRange.first) / scaleFactor;
    auto axisX = chart->axes(Qt::Horizontal).first();
    axisX->setRange(centerX - newWidth / 2.0, centerX + newWidth / 2.0);
}

void PlotWindow::showContextMenu(const QPoint &pos) {
    QMenu contextMenu(this);

    QAction *changeColorAction = contextMenu.addAction("Change Color...");
    connect(changeColorAction, &QAction::triggered, this, &PlotWindow::changeSeriesColor);

    QAction *renameAction = contextMenu.addAction("Rename Signal...");
    connect(renameAction, &QAction::triggered, this, &PlotWindow::renameSeries);

    contextMenu.exec(chartView->mapToGlobal(pos));
}

void PlotWindow::changeSeriesColor() {
    QColor newColor = QColorDialog::getColor(series->color(), this, "Select Signal Color");
    if (newColor.isValid())
        series->setColor(newColor);
}

void PlotWindow::renameSeries() {
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Signal",
                                            "New signal name:", QLineEdit::Normal,
                                            series->name(), &ok);
    if (ok && !newName.isEmpty())
        series->setName(newName);
}

void PlotWindow::onSeriesClicked(const QPointF &point) {
    cursorSeries->clear();
    cursorSeries->append(point);

    QString xTitle = chart->axes(Qt::Horizontal).first()->titleText();
    QString yTitle = chart->axes(Qt::Vertical).first()->titleText();

    QString cursorText = QString("%1: %L2, %3: %L4")
                             .arg(xTitle)
                             .arg(point.x(), 0, 'f', 2)
                             .arg(yTitle)
                             .arg(point.y(), 0, 'f', 2);
    statusBar()->showMessage(cursorText);
}

PlotTransientData::PlotTransientData(QWidget *parent) : PlotWindow(parent) {
    setWindowTitle("Transient Analysis Plot");

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY->setTitleText("Value (V or A)");
    axisX->setGridLineVisible(true);
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
    axisX->setGridLineVisible(true);
}