#include "SchematicWidget.h"
#include <cmath>

SchematicWidget::SchematicWidget(Circuit* circuit, QWidget* parent) : circuit_ptr(circuit), QWidget(parent) {
    setMouseTracking(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::gray);
    setAutoFillBackground(true);
    setPalette(pal);

    setFocusPolicy(Qt::StrongFocus); // KEyboard Events

    componentCounters["R"] = 0;
    componentCounters["C"] = 0;
    componentCounters["L"] = 0;
    componentCounters["V"] = 0;
    componentCounters["D"] = 0;
    componentCounters["I"] = 0;
}

QString SchematicWidget::getNodeNameFromPoint(const QPoint& pos) const {
    int gridX = pos.x() / gridSize;
    int gridY = pos.y() / gridSize;
    return QString("N_%1_%2").arg(gridX).arg(gridY);
}

QString SchematicWidget::getNextComponentName(const QString& type) {
    componentCounters[type]++;
    return QString("%1%2").arg(type).arg(componentCounters[type]);
}

void SchematicWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
    QPainter painter(this);

    QPen gridPen;
    gridPen.setColor(Qt::black);
    gridPen.setWidth(1);
    painter.setPen(gridPen);
    int width = this->width();
    int height = this->height();
    for (int x = 0; x < width; x += gridSize) {
        for (int y = 0; y < height; y += gridSize) {
            painter.drawPoint(x, y);
        }
    }

    for (int i = 0; i < components.size(); i++) {
        bool isHovered = (i == hoveredComponentIndex && currentMode == InteractionMode::deleteMode);
        drawComponent(painter, components[i].startPoint, components[i].isHorizontal, components[i].name, isHovered);
    }

    // Draw the preview before placing the component
    if (currentMode != InteractionMode::Normal && currentMode != InteractionMode::deleteMode && currentMode !=
        InteractionMode::placingWire) {
        QPoint startPos = stickToGrid(currentMousePos);
        drawComponent(painter, startPos, placementIsHorizontal, currentCompType);
    }

    QPen wirePen(Qt::blue, 2);
    painter.setPen(wirePen);
    for (const auto& wire : wires)
        painter.drawLine(wire.startPoint, wire.endPoint);


    // Preview before placing wires
    if (isWiring)
        painter.drawLine(wireStartPoint, stickToGrid(currentMousePos));
}

void SchematicWidget::startRunAnalysis() {
    QMessageBox::information(this, "Settings", "Buy premium!");
}

void SchematicWidget::startPlacingResistor() {
    currentMode = InteractionMode::placingResistor;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    currentCompType = "R";
    setFocus();
}

void SchematicWidget::startPlacingCapacitor() {
    currentMode = InteractionMode::placingCapacitor;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    currentCompType = "C";
    setFocus();
}

void SchematicWidget::startPlacingInductor() {
    currentMode = InteractionMode::placingInductor;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    currentCompType = "L";
    setFocus();
}

void SchematicWidget::startPlacingVoltageSource() {
    currentMode = InteractionMode::placingVoltageSource;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    currentCompType = "V";
    setFocus();
}

void SchematicWidget::startPlacingDiode() {
    currentMode = InteractionMode::placingDiode;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    currentCompType = "D";
    setFocus();
}

void SchematicWidget::startPlacingCurrentSource() {
    currentMode = InteractionMode::placingCurrentSource;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    currentCompType = "I";
    setFocus();
}

void SchematicWidget::startDeleteComponent() {
    currentMode = InteractionMode::deleteMode;
    setCursor(Qt::OpenHandCursor);
}

void SchematicWidget::startPlacingWire() {
    currentMode = InteractionMode::placingWire;
    isWiring = false;
    setCursor(Qt::CrossCursor);
}

void SchematicWidget::startOpenNodeLibrary() {
    NodeLibraryDialog dialog(this);
    connect(&dialog, &NodeLibraryDialog::componentSelected, this, &SchematicWidget::handleNodeLibraryItemSelection);
    dialog.exec();
}

void SchematicWidget::keyPressEvent(QKeyEvent* event) {
    if (currentMode != InteractionMode::Normal) {
        if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_R) {
            placementIsHorizontal = !placementIsHorizontal;
            update();
            return;
        }
        if (event->key() == Qt::Key_Escape) {
            currentMode = InteractionMode::Normal;
            currentCompType = "NF";
            setCursor(Qt::ArrowCursor);
            isWiring = false;
            update();
            return;
        }
    }
    else
        QWidget::keyPressEvent(event);
}

QPoint SchematicWidget::stickToGrid(const QPoint& pos) {
    int x = std::round(static_cast<double>(pos.x()) / gridSize) * gridSize;
    int y = std::round(static_cast<double>(pos.y()) / gridSize) * gridSize;
    return QPoint(x, y);
}

void SchematicWidget::mouseMoveEvent(QMouseEvent* event) {
    currentMousePos = event->pos();
    if (currentMode == InteractionMode::deleteMode) {
        hoveredComponentIndex = -1;
        for (int i = 0; i < components.size(); i++) {
            QPoint start = components[i].startPoint;
            QPoint end = components[i].isHorizontal
                             ? start + QPoint(componentLength, 0)
                             : start + QPoint(0, componentLength);
            QRect componentRect(start, end);
            componentRect = componentRect.normalized().adjusted(-5, -5, 5, 5);

            if (componentRect.contains(currentMousePos)) {
                hoveredComponentIndex = i;
                break;
            }
        }
    }
    update();
}

void SchematicWidget::placingWireMouseEvent(QMouseEvent* event) {
    QPoint currentPoint = stickToGrid(event->pos());
    QString currentNodeName = findNodeAt(currentPoint);

    if (!isWiring) {
        isWiring = true;
        wireStartPoint = currentPoint;
        wires.push_back({currentPoint, currentPoint, currentNodeName});
    }
    else {
        QString prevNodeName = findNodeAt(wireStartPoint);
        if (currentNodeName != prevNodeName)
            circuit_ptr->connectNodes(currentNodeName.toStdString(), prevNodeName.toStdString());
        wires.push_back({wireStartPoint, currentPoint, prevNodeName});
        wireStartPoint = currentPoint;
    }
}

void SchematicWidget::showSimpleValueDialog(QMouseEvent* event) {
    ValueDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString valueStr = dialog.getValue();
        if (valueStr.isEmpty())
            return;

        double value = parseSpiceValue(valueStr.toStdString());

        QPoint startPoint = stickToGrid(event->pos());
        QPoint endPoint = placementIsHorizontal
                              ? startPoint + QPoint(componentLength, 0)
                              : startPoint + QPoint(0, componentLength);

        QString componentName = getNextComponentName(currentCompType);
        QString node1Name = getNodeNameFromPoint(startPoint);
        QString node2Name = getNodeNameFromPoint(endPoint);

        components.push_back({startPoint, placementIsHorizontal, componentName});

        circuit_ptr->addComponent(currentCompType.toStdString(), componentName.toStdString(),
                                  node1Name.toStdString(), node2Name.toStdString(), value, {}, {}, false);
    }
}

void SchematicWidget::showSourceValueDialog(QMouseEvent* event) {
    SourceValueDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QPoint startPoint = stickToGrid(event->pos());
        QPoint endPoint = placementIsHorizontal
                              ? startPoint + QPoint(componentLength, 0)
                              : startPoint + QPoint(0, componentLength);
        QString componentName = getNextComponentName(currentCompType);
        QString node1Name = getNodeNameFromPoint(startPoint);
        QString node2Name = getNodeNameFromPoint(endPoint);

        components.push_back({startPoint, placementIsHorizontal, componentName});

        if (dialog.isSinusoidal()) {
            QString offsetStr = dialog.getSinOffset();
            QString amplitudeStr = dialog.getSinAmplitude();
            QString frequencyStr = dialog.getSinFrequency();

            if (offsetStr.isEmpty() || amplitudeStr.isEmpty() || frequencyStr.isEmpty())
                return;

            std::vector<double> sinParams = {
                parseSpiceValue(offsetStr.toStdString()), parseSpiceValue(amplitudeStr.toStdString()),
                parseSpiceValue(frequencyStr.toStdString())
            };

            circuit_ptr->addComponent(currentCompType.toStdString(), componentName.toStdString(),
                                      node1Name.toStdString(), node2Name.toStdString(), 0.0, sinParams, {}, true);
        }
        else {
            QString dcValue = dialog.getDCValue();
            if (dcValue.isEmpty())
                return;
            double value = parseSpiceValue(dcValue.toStdString());
            circuit_ptr->addComponent(currentCompType.toStdString(), componentName.toStdString(),
                                      node1Name.toStdString(), node2Name.toStdString(), value, {}, {}, false);
        }
    }
}

void SchematicWidget::placingComponentMouseEvent(QMouseEvent* event) {
    if (currentCompType == "R" || currentCompType == "C" || currentCompType == "L")
        showSimpleValueDialog(event);
    else if (currentCompType == "V" || currentCompType == "I")
        showSourceValueDialog(event);
}

void SchematicWidget::deletingComponentMouseEvent(QMouseEvent* event) {
    QPoint clickPos = event->pos();

    for (auto it = components.rbegin(); it != components.rend(); it++) {
        QPoint start = it->startPoint;
        QPoint end;
        if (it->isHorizontal)
            end = start + QPoint(componentLength, 0);
        else
            end = start + QPoint(0, componentLength);

        QRect componentRect(start, end);
        componentRect = componentRect.normalized();
        componentRect.adjust(-7, -7, 7, 7);

        if (componentRect.contains(clickPos)) {
            components.erase(std::next(it).base());
            // TODO: delete from circuit netlist
            return;
        }
    }
}

void SchematicWidget::mousePressEvent(QMouseEvent* event) {
    if (currentMode == InteractionMode::Normal)
        return;

    if (event->button() == Qt::RightButton) {
        currentMode = InteractionMode::Normal;
        setCursor(Qt::ArrowCursor);
        isWiring = false;
        update();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (currentMode == InteractionMode::placingWire) {
            placingWireMouseEvent(event);
        }
        else if (currentMode != InteractionMode::Normal && currentMode != InteractionMode::deleteMode) {
            placingComponentMouseEvent(event);
        }
        else if (currentMode == InteractionMode::deleteMode) {
            deletingComponentMouseEvent(event);
        }
        update();
    }
}

void SchematicWidget::drawComponent(QPainter& painter, const QPoint& start, bool isHorizontal, QString type,
                                    bool isHovered) const {
    QPoint end;
    if (isHorizontal)
        end = start + QPoint(componentLength, 0);
    else
        end = start + QPoint(0, componentLength);

    QPen componentPen(Qt::darkBlue, 2);
    componentPen.setColor(isHovered ? Qt::yellow : Qt::darkBlue);
    painter.setPen(componentPen);
    painter.drawLine(start, end);

    painter.setBrush(Qt::white);
    QRectF resistorBody((start + end) / 2 - QPoint(15, 8), QSize(30, 16));
    painter.drawRect(resistorBody);

    painter.drawText(resistorBody, Qt::AlignCenter, type);
}

QString SchematicWidget::findNodeAt(const QPoint& nodePos) {
    for (const auto& component : components) {
        QPoint start = component.startPoint;
        QPoint end = component.isHorizontal ? start + QPoint(componentLength, 0) : start + QPoint(0, componentLength);
        if (nodePos == start || nodePos == end)
            return nodePos == start ? getNodeNameFromPoint(start) : getNodeNameFromPoint(end);
    }

    for (const auto& wire : wires) {
        QRect wireRect(wire.startPoint, wire.endPoint);
        wireRect = wireRect.normalized().adjusted(-5, -5, 5, 5);
        if (wireRect.contains(nodePos))
            return wire.nodeName;
    }

    return getNodeNameFromPoint(nodePos);
}

void SchematicWidget::handleNodeLibraryItemSelection(const QString& compType) {
    if (compType == "R")
        startPlacingResistor();
    else if (compType == "C")
        startPlacingCapacitor();
    else if (compType == "L")
        startPlacingInductor();
    else if (compType == "V")
        startPlacingVoltageSource();
    else if (compType == "D")
        startPlacingDiode();
    else if (compType == "I")
        startPlacingCurrentSource();
}