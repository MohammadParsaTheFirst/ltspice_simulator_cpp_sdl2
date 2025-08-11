#ifndef SCHEMATICWIDGET_H
#define SCHEMATICWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <vector>
#include <QKeyEvent>
#include <map>
#include "Circuit.h"

enum class InteractionMode
{
    Normal,
    placingResistor,
    placingCapacitor,
    placingInductor,
    placingVoltageSource,
    placingGround,
    placingDiode,
    deleteMode
};

struct ComponentGraphicalInfo
{
    QPoint startPoint;
    bool isHorizontal;
    QString name;
};

class SchematicWidget : public QWidget
{
  Q_OBJECT
public:
    SchematicWidget(Circuit* circuit, QWidget *parent = Q_NULLPTR);

public slots:
    void startPlacingResistor();
    void startPlacingCapacitor();
    void startPlacingInductor();
    void startPlacingVoltageSource();
    void startPlacingDiode();
    void startDeleteComponent();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QPoint stickToGrid(const QPoint& pos);
    void drawComponent(QPainter& painter, const QPoint& start, bool isHorizontal, QString type, bool isHovered = false) const;
    QString getNodeNameFromPoint(const QPoint& pos) const;
    QString getNextComponentName(const QString& type);

    const int gridSize = 30; // Pixels
    const int componentLength = 3 * gridSize;
    InteractionMode currentMode = InteractionMode::Normal;

    bool placementIsHorizontal = true;
    QPoint currentMousePos;
    QString currentCompType = "NF";

    std::vector<ComponentGraphicalInfo> components;

    int hoveredComponentIndex = -1;
    Circuit *circuit_ptr;

    std::map<QString, int> componentCounters;
};

#endif //SCHEMATICWIDGET_H