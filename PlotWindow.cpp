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

    axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);

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

    cursorSeries->attachAxis(axisY);

    statusBar()->show();
    setCentralWidget(centralWidget);
}

void PlotWindow::clearAllSeries() {
    chart->removeAllSeries();
    m_seriesList.clear();

    cursorSeries = new QScatterSeries();
    cursorSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    cursorSeries->setMarkerSize(10.0);
    cursorSeries->setColor(Qt::red);
    chart->addSeries(cursorSeries);
    cursorSeries->attachAxis(axisY);
    auto axisX = chart->axes(Qt::Horizontal).first();
    if (axisX) {
        cursorSeries->attachAxis(axisX);
    }

    updateFullRange();
}

void PlotWindow::addSeries(const std::map<double, double>& data, const QString& seriesName) {
    QLineSeries* newSeries = new QLineSeries();
    newSeries->setName(seriesName);

    for (const auto& pair : data)
        newSeries->append(pair.first, pair.second);
    chart->addSeries(newSeries);
    m_seriesList.append(newSeries);
    auto axisX = chart->axes(Qt::Horizontal).first();
    if (axisX)
        newSeries->attachAxis(axisX);
    newSeries->attachAxis(axisY);

    connect(newSeries, &QLineSeries::clicked, this, [this, newSeries](const QPointF& point) {
        onSeriesClicked(newSeries, point);
    });

    const auto markers = chart->legend()->markers();
    for (const auto& marker : markers) {
        if (marker->series() == newSeries) {
            connect(marker, &QLegendMarker::clicked, this, &PlotWindow::handleLegendClick);
            break;
        }
    }

    updateFullRange();
}

void PlotWindow::handleLegendClick() {
    QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
    if (marker) {
        m_activeSeries = qobject_cast<QLineSeries*>(marker->series());
        showContextMenu(QCursor::pos());
    }
}

void PlotWindow::finalAxisSetup() {
    auto axisX = chart->axes(Qt::Horizontal).first();
    if (axisX) {
        cursorSeries->attachAxis(axisX);
    }
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
    if (!m_activeSeries)
        return;

    QMenu contextMenu(this);
    QAction *changeColorAction = contextMenu.addAction("Change Color...");
    connect(changeColorAction, &QAction::triggered, this, &PlotWindow::changeSeriesColor);

    QAction *renameAction = contextMenu.addAction("Rename Signal...");
    connect(renameAction, &QAction::triggered, this, &PlotWindow::renameSeries);

    contextMenu.exec(pos);
}

void PlotWindow::changeSeriesColor() {
    if (!m_activeSeries)
        return;
    QColor newColor = QColorDialog::getColor(m_activeSeries->color(), this, "Select Signal Color");
    if (newColor.isValid())
        m_activeSeries->setColor(newColor);
}

void PlotWindow::renameSeries() {
    if (!m_activeSeries) return;
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Signal", "New signal name:", QLineEdit::Normal, m_activeSeries->name(), &ok);
    if (ok && !newName.isEmpty())
        m_activeSeries->setName(newName);
}

void PlotWindow::onSeriesClicked(QLineSeries* series, const QPointF &point) {
    cursorSeries->clear();
    cursorSeries->append(point);
    cursorSeries->setColor(series->color());

    QString xTitle = chart->axes(Qt::Horizontal).first()->titleText();
    QString yTitle = chart->axes(Qt::Vertical).first()->titleText();

    QString cursorText = QString("%1: %L2, %3: %L4").arg(xTitle).arg(point.x(), 0, 'g', 3).arg(yTitle).arg(point.y(), 0, 'g', 3);
    statusBar()->showMessage(cursorText);
}

void PlotWindow::clearCursor() {
    cursorSeries->clear();
}

void PlotWindow::updateFullRange() {
    if (m_seriesList.isEmpty()) {
        fullXRange = {0, 0};
        fullYRange = {0, 0};
        return;
    }

    double overallXMin = std::numeric_limits<double>::max();
    double overallXMax = std::numeric_limits<double>::lowest();
    double overallYMin = std::numeric_limits<double>::max();
    double overallYMax = std::numeric_limits<double>::lowest();

    for (QLineSeries* s : m_seriesList) {
        for (const QPointF& p : s->points()) {
            if (p.x() < overallXMin)
                overallXMin = p.x();
            if (p.x() > overallXMax)
                overallXMax = p.x();
            if (std::isfinite(p.y())) {
                if (p.y() < overallYMin)
                    overallYMin = p.y();
                if (p.y() > overallYMax)
                    overallYMax = p.y();
            }
        }
    }
    fullXRange = {overallXMin, overallXMax};
    fullYRange = {overallYMin, overallYMax};

    horizontalScaleChanged(horizontalSlider->value());
    verticalScaleChanged(verticalSlider->value());
}


PlotTransientData::PlotTransientData(QWidget *parent) : PlotWindow(parent) {
    setWindowTitle("Transient Analysis Plot");

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Time");
    chart->addAxis(axisX, Qt::AlignBottom);

    axisY->setTitleText("Value");
    axisX->setGridLineVisible(true);
    axisX->setLabelFormat("%.2e");
    axisY->setLabelFormat("%.2e");

    finalAxisSetup();
}

PlotACData::PlotACData(QWidget *parent) : PlotWindow(parent) {
    setWindowTitle("AC Sweep Plot");

    QLogValueAxis *axisX = new QLogValueAxis();
    axisX->setTitleText("Frequency");
    axisX->setLabelFormat("%g");
    axisX->setBase(10);
    chart->addAxis(axisX, Qt::AlignBottom);

    axisY->setTitleText("Magnitude");
    axisX->setGridLineVisible(true);
    axisY->setLabelFormat("%.2e");

    finalAxisSetup();
}