#ifndef SCHEMATICWIDGET_H
#define SCHEMATICWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QInputDialog>
#include <QString>
#include "Circuit.h"
#include "Dialogs.h"
#include "PlotWindow.h"

enum class InteractionMode {
    Normal,
    placingResistor,
    placingCapacitor,
    placingInductor,
    placingVoltageSource,
    placingACVoltageSource,
    placingGround,
    placingDiode,
    deleteMode,
    placingWire,
    placingCurrentSource,
    placingLabel,
    placingSubcircuitNodes,
    placingSubcircuit
};

class SchematicWidget : public QWidget {
    Q_OBJECT
public:
    SchematicWidget(Circuit* circuit, QWidget* parent = Q_NULLPTR);
    void reloadFromCircuit();

public slots:
    void startOpenConfigureAnalysis();
    void startRunAnalysis();
    void startPlacingGround();
    void startPlacingResistor();
    void startPlacingCapacitor();
    void startPlacingInductor();
    void startPlacingVoltageSource();
    void startPlacingACVoltageSource();
    void startPlacingCurrentSource();
    void startPlacingDiode();
    void startDeleteComponent();
    void startPlacingWire();
    void startOpenNodeLibrary();
    void startPlacingLabel();
    void startCreateSubcircuit();
    void startPlacingSubcircuit();
    void startOpeningSubcircuitLibrary();

private slots:
    void handleNodeLibraryItemSelection(const QString& compType);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QPoint stickToGrid(const QPoint& pos);
    void drawComponent(QPainter& painter, const QPoint& start, bool isHorizontal, QString type,
                       bool isHovered = false) const;
    QString getNodeNameFromPoint(const QPoint& pos) const;
    QString getNextComponentName(const QString& type);
    QString findNodeAt(const QPoint& nodePos);
    QString findOrCreateNodeAtPoint(const QPoint& point);

    void placingWireMouseEvent(QMouseEvent* event);
    void placingComponentMouseEvent(QMouseEvent* event);
    bool deletingComponentMouseEvent(QMouseEvent* event);
    void deletingGroundMouseEvent(QMouseEvent* event);
    void placingLabelMouseEvent(QMouseEvent* event);
    void placingGroundMouseEvent(QMouseEvent* event);
    void showSimpleValueDialog(QMouseEvent* event);
    void showSourceValueDialog(QMouseEvent* event);
    void placingOtherComp(QMouseEvent* event);
    void placingSubcircuitMouseEvent(QMouseEvent* event);
    void selectingSubcircuitNodesMouseEvent(QMouseEvent* event);

    void drawGridDots(QPainter& painter);
    void drawComponents(QPainter& painter);
    void drawLabels(QPainter& painter);
    void drawWires(QPainter& painter);
    void drawGrounds(QPainter& painter);
    void drawGroundSymbol(QPainter& painer, const QPoint& pos);

    const int gridSize = 40;
    InteractionMode currentMode = InteractionMode::Normal;

    const int componentLength = 3 * gridSize;
    bool placementIsHorizontal = true;
    QPoint currentMousePos;
    QString currentCompType = "NF";
    int hoveredComponentIndex = -1;
    Circuit* circuit_ptr;
    std::map<QString, int> componentCounters;

    bool isWiring = false;
    QPoint wireStartPoint;

    // Analysis
    std::vector<QString> parametersForAnalysis;
    // Transient analysis
    double transientTStop = 0.0;
    double transientTStart = 0.0;
    double transientTStep = 0.0;
    // AC sweep
    double acSweepStartFrequency = 0.0;
    double acSweepStopFrequency = 0.0;
    double acSweepNPoints = 0.0;
    
    QString currentSubcircuitName;
    std::vector<QString> subcircuitNodes;
};

#endif //SCHEMATICWIDGET_H