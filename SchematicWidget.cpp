#include <cmath>
#include "SchematicWidget.h"

SchematicWidget::SchematicWidget(Circuit* circuit, QWidget* parent) : circuit_ptr(circuit), QWidget(parent) {
    setMouseTracking(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::gray);
    setAutoFillBackground(true);
    setPalette(pal);

    setFocusPolicy(Qt::StrongFocus); // Keyboard Events

    componentCounters["R"] = 0;
    componentCounters["C"] = 0;
    componentCounters["L"] = 0;
    componentCounters["V"] = 0;
    componentCounters["D"] = 0;
    componentCounters["I"] = 0;
    componentCounters["E"] = 0;
    componentCounters["F"] = 0;
    componentCounters["G"] = 0;
    componentCounters["H"] = 0;
    componentCounters["AC"] = 0;
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
    drawGridDots(painter);
    drawComponents(painter);
    drawWires(painter);
    drawLabels(painter);
    drawGrounds(painter);
}

void SchematicWidget::drawGridDots(QPainter& painter) {
    QPen gridPen;
    gridPen.setColor(Qt::black);
    gridPen.setWidth(1);
    painter.setPen(gridPen);
    int width = this->width();
    int height = this->height();
    for (int x = 0; x < width; x += gridSize)
        for (int y = 0; y < height; y += gridSize)
            painter.drawPoint(x, y);
}

void SchematicWidget::drawComponents(QPainter& painter) {
    const auto& componentGraphics = circuit_ptr->getComponentGraphics();
    for (int i = 0; i < componentGraphics.size(); i++) {
        bool isHovered = (i == hoveredComponentIndex && currentMode == InteractionMode::deleteMode);
        drawComponent(painter, componentGraphics[i].startPoint, componentGraphics[i].isHorizontal, QString::fromStdString(componentGraphics[i].name), isHovered);
    }
    if (currentMode != InteractionMode::Normal && currentMode != InteractionMode::deleteMode && currentMode !=
        InteractionMode::placingWire && currentMode != InteractionMode::placingLabel && currentMode != InteractionMode::placingGround &&
        currentMode != InteractionMode::placingSubcircuitNodes) {
        QPoint startPos = stickToGrid(currentMousePos);
        drawComponent(painter, startPos, placementIsHorizontal, currentCompType);
    }
}

void SchematicWidget::drawWires(QPainter& painter) {
    QPen wirePen(Qt::darkBlue, 2);
    painter.setPen(wirePen);
    for (const auto& wire : circuit_ptr->getWires())
        painter.drawLine(wire.startPoint, wire.endPoint);
    if (isWiring)
        painter.drawLine(wireStartPoint, stickToGrid(currentMousePos));
}

void SchematicWidget::drawGrounds(QPainter& painter) {
    QPen groundPen(Qt::darkGreen, 2);
    painter.setPen(groundPen);

    for (const auto& ground : circuit_ptr->getGrounds()) {
        drawGroundSymbol(painter, ground.position);
    }

    if (currentMode == InteractionMode::placingGround) {
        drawGroundSymbol(painter, stickToGrid(currentMousePos));
    }
}

void SchematicWidget::drawGroundSymbol(QPainter& painter, const QPoint& pos) {
    painter.drawLine(pos, pos + QPoint(0, 15));
    painter.drawLine(pos + QPoint(-15, 15), pos + QPoint(15, 15));
    painter.drawLine(pos + QPoint(-10, 20), pos + QPoint(10, 20));
    painter.drawLine(pos + QPoint(-5, 25), pos + QPoint(5, 25));
}

void SchematicWidget::drawLabels(QPainter& painter) {
    QPen labelPen(Qt::blue, 2);
    painter.setPen(labelPen);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    for (const auto& label : circuit_ptr->getLabels()) {
        painter.drawEllipse(label.position, 2, 2);
        painter.drawText(label.position + QPoint(10, 3), QString::fromStdString(label.name));
    }
}

void SchematicWidget::startOpenConfigureAnalysis() {
    ConfigureAnalysisDialog dialog(this);
    try{
        if (dialog.exec() == QDialog::Accepted) {
            if (dialog.getSelectedAnalysisType() == 0) {
                transientTStop = parseSpiceValue(dialog.getTransientTstop().toStdString());
                transientTStart = parseSpiceValue(dialog.getTransientTstart().toStdString());
                transientTStep = parseSpiceValue(dialog.getTransientTstep().toStdString());

                QString params = dialog.getTransientParameter();
                std::string paramStr = params.toStdString();
                std::stringstream ss(paramStr);
                std::string word;
                std::vector<std::string> paramsStr;
                while (ss >> word) {
                    parametersForAnalysis.push_back(QString::fromStdString(word));
                    paramsStr.push_back(word);
                }

                if (transientTStep <= 0)
                    throw std::runtime_error("Step time must be greater than zero.");
                if (transientTStop <= transientTStart)
                    throw std::runtime_error("Start time should less than stop time.");
                if (parametersForAnalysis.empty())
                    throw std::runtime_error("No parameters added for analysis.");

                QMessageBox::information(this, "Info", "Transient Analysis variables updated.");

                circuit_ptr->runTransientAnalysis(transientTStop, transientTStart, transientTStep);
                std::map<std::string, std::map<double, double>> results = circuit_ptr->getTransientResults(paramsStr);

                if (!results.empty()) {
                    PlotTransientData *plotWindow = new PlotTransientData(this);
                    for (const auto& pair : results)
                        plotWindow->addSeries(pair.second, QString::fromStdString(pair.first));
                    plotWindow->show();
                }
                else
                    QMessageBox::warning(this, "Analysis Failed", "Could not generate plot data. Please check your circuit and parameters.");
            }
            else if (dialog.getSelectedAnalysisType() == 1) {
                acSweepStartFrequency = parseSpiceValue(dialog.getACOmegaStart().toStdString());
                acSweepStopFrequency = parseSpiceValue(dialog.getACOmegaStop().toStdString());
                acSweepNPoints = parseSpiceValue(dialog.getACNPoints().toStdString());

                QString params = dialog.getTransientParameter();
                std::string paramStr = params.toStdString();
                std::stringstream ss(paramStr);
                std::string word;
                std::vector<std::string> paramsStr;
                while (ss >> word) {
                    parametersForAnalysis.push_back(QString::fromStdString(word));
                    paramsStr.push_back(word);
                }

                if (acSweepStartFrequency <= 0 || acSweepStopFrequency <=0)
                    throw std::runtime_error("Frequency must be greater than zero.");
                if (acSweepStopFrequency <= acSweepStartFrequency)
                    throw std::runtime_error("Start frequency should less than stop frequency.");
                if (parametersForAnalysis.empty())
                    throw std::runtime_error("No parameters added for analysis.");

                QMessageBox::information(this, "Info", "AC Sweep Analysis variables updated.");

                circuit_ptr->runACAnalysis(acSweepStartFrequency, acSweepStopFrequency, acSweepNPoints);
                std::map<std::string, std::map<double, double>> results = circuit_ptr->getACSweepResults(paramsStr);

                if (!results.empty()) {
                    PlotACData *plotWindow = new PlotACData(this);
                    for (const auto& pair : results)
                        plotWindow->addSeries(pair.second, QString::fromStdString(pair.first));
                    plotWindow->show();
                }
                else
                    QMessageBox::warning(this, "Analysis Failed", "Could not generate plot data. Please check your circuit and parameters.");
            }
        }
    } catch (const std::exception& e) {
        std::string errorMessage = e.what();
        QString qErrorMessage = QString::fromStdString("Error: " + errorMessage);
        QMessageBox::warning(this, "Error", qErrorMessage);
    }
}

void SchematicWidget::startRunAnalysis() {
    startOpenConfigureAnalysis();
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

void SchematicWidget::startPlacingACVoltageSource() {
    currentMode = InteractionMode::placingACVoltageSource;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    currentCompType = "AC";
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

void SchematicWidget::startPlacingGround() {
    currentMode = InteractionMode::placingGround;
    setCursor(Qt::PointingHandCursor);
    setFocus();
}

void SchematicWidget::startOpenNodeLibrary() {
    NodeLibraryDialog dialog(circuit_ptr, this);
    connect(&dialog, &NodeLibraryDialog::componentSelected, this, &SchematicWidget::handleNodeLibraryItemSelection);
    dialog.exec();
}

void SchematicWidget::startPlacingLabel() {
    currentMode = InteractionMode::placingLabel;
    setCursor(Qt::IBeamCursor);
    setFocus();
}

void SchematicWidget::startCreateSubcircuit() {
    currentMode = InteractionMode::placingSubcircuitNodes;
    subcircuitNodes.clear();
    setCursor(Qt::PointingHandCursor);
    QMessageBox::information(this, "Create Subcircuit", "Please select the first node.");
    update();
}

void SchematicWidget::startPlacingSubcircuit() {
    currentMode = InteractionMode::placingSubcircuit;
    currentCompType = currentSubcircuitName;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    setFocus();
}

void SchematicWidget::startOpeningSubcircuitLibrary() {
    SubcircuitLibarary dialog(circuit_ptr, this);
    connect(&dialog, &SubcircuitLibarary::componentSelected, this, &SchematicWidget::handleNodeLibraryItemSelection);
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
            // subcircuitNodes.clear();
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
        const auto& componentGraphics = circuit_ptr->getComponentGraphics();
        for (int i = 0; i < componentGraphics.size(); i++) {
            QPoint start = componentGraphics[i].startPoint;
            QPoint end = componentGraphics[i].isHorizontal
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

    if (!isWiring) {
        isWiring = true;
        wireStartPoint = currentPoint;
    }
    else {
        QString startNodeName = findOrCreateNodeAtPoint(wireStartPoint);
        QString endNodeName = findOrCreateNodeAtPoint(currentPoint);

        circuit_ptr->connectNodes(startNodeName.toStdString(), endNodeName.toStdString());
        circuit_ptr->addWire(wireStartPoint, currentPoint, startNodeName.toStdString());
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

        circuit_ptr->addComponent(currentCompType.toStdString(), componentName.toStdString(),
                                  node1Name.toStdString(), node2Name.toStdString(), startPoint, placementIsHorizontal, value, {}, {}, false);
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
                                      node1Name.toStdString(), node2Name.toStdString(), startPoint, placementIsHorizontal, 0.0, sinParams, {}, true);
        }
        else {
            QString dcValue = dialog.getDCValue();
            if (dcValue.isEmpty())
                return;
            double value = parseSpiceValue(dcValue.toStdString());
            circuit_ptr->addComponent(currentCompType.toStdString(), componentName.toStdString(),
                                      node1Name.toStdString(), node2Name.toStdString(), startPoint, placementIsHorizontal, value, {}, {}, false);
        }
    }
}

void SchematicWidget::placingOtherComp(QMouseEvent* event) {
    QPoint startPoint = stickToGrid(event->pos());
    QPoint endPoint = placementIsHorizontal
                          ? startPoint + QPoint(componentLength, 0)
                          : startPoint + QPoint(0, componentLength);

    QString componentName = getNextComponentName(currentCompType);
    QString node1Name = getNodeNameFromPoint(startPoint);
    QString node2Name = getNodeNameFromPoint(endPoint);

    circuit_ptr->addComponent(currentCompType.toStdString(), componentName.toStdString(),
                              node1Name.toStdString(), node2Name.toStdString(), startPoint, placementIsHorizontal, 0.0, {}, {}, false);
}

void SchematicWidget::placingComponentMouseEvent(QMouseEvent* event) {
    if (currentCompType == "R" || currentCompType == "C" || currentCompType == "L")
        showSimpleValueDialog(event);
    else if (currentCompType == "V" || currentCompType == "I")
        showSourceValueDialog(event);
    else if (currentCompType == "AC" || currentCompType == "D")
        placingOtherComp(event);
}

bool SchematicWidget::deletingComponentMouseEvent(QMouseEvent* event) {
    QPoint clickPos = event->pos();

    const auto& componentGraphics = circuit_ptr->getComponentGraphics();
    for (auto it = componentGraphics.begin(); it != componentGraphics.end(); it++) {
        QPoint start = it->startPoint;
        QPoint end = placementIsHorizontal ? start + QPoint(componentLength, 0) : start + QPoint(0, componentLength);

        QRect componentRect(start, end);
        componentRect = componentRect.normalized();
        componentRect.adjust(-5, -5, 5, 5);

        if (componentRect.contains(clickPos)) {
            circuit_ptr->deleteComponent(it->name, it->name[0]);
            return true;
        }
    }
    return false;
}

void SchematicWidget::deletingGroundMouseEvent(QMouseEvent* event) {
    QPoint clickPos = event->pos();

    for (const auto& groundInfo : circuit_ptr->getGrounds()) {
        QRect groundRect(groundInfo.position - QPoint(15, 0), QSize(30, 30));
        if (groundRect.contains(clickPos)) {
            QString nodeName = getNodeNameFromPoint(groundInfo.position);
            circuit_ptr->deleteGround(nodeName.toStdString());
            return;
        }
    }
}

void SchematicWidget::placingLabelMouseEvent(QMouseEvent* event) {
    QPoint clickPos = stickToGrid(event->pos());
    QString nodeName = findNodeAt(clickPos);

    LabelDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString valueString = dialog.getLabel();
        if (valueString.isEmpty())
            return;

        circuit_ptr->addLabel(clickPos, valueString.toStdString(), nodeName.toStdString());
    }
}

void SchematicWidget::placingGroundMouseEvent(QMouseEvent* event) {
    QPoint clickPos = stickToGrid(event->pos());
    QString nodeName = findNodeAt(clickPos);
    circuit_ptr->addGround(nodeName.toStdString(), clickPos);
}

void SchematicWidget::mousePressEvent(QMouseEvent* event) {
    try{
        if (currentMode == InteractionMode::Normal)
            return;
        if (event->button() == Qt::RightButton) {
            currentMode = InteractionMode::Normal;
            currentCompType = "NF";
            setCursor(Qt::ArrowCursor);
            isWiring = false;
            // subcircuitNodes.clear();
            update();
            return;
        }
        if (event->button() == Qt::LeftButton) {
            if (currentMode == InteractionMode::placingWire) {
                placingWireMouseEvent(event);
            }
            else if (currentMode == InteractionMode::placingLabel) {
                placingLabelMouseEvent(event);
            }
            else if (currentMode == InteractionMode::deleteMode) {
                bool itemDeleted = deletingComponentMouseEvent(event);
                if (!itemDeleted)
                    deletingGroundMouseEvent(event);
                update();
            }
            else if (currentMode == InteractionMode::placingGround) {
                placingGroundMouseEvent(event);
            }
            else if (currentMode == InteractionMode::placingSubcircuitNodes) {
                selectingSubcircuitNodesMouseEvent(event);
            }
            else if (currentMode == InteractionMode::placingSubcircuit) {
                placingSubcircuitMouseEvent(event);
            }
            else {
                placingComponentMouseEvent(event);
            }
            update();
        }
    } catch(const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void SchematicWidget::placingSubcircuitMouseEvent(QMouseEvent* event) {
    QPoint startPoint = stickToGrid(event->pos());
    QPoint endPoint = placementIsHorizontal ? startPoint + QPoint(componentLength, 0) : startPoint + QPoint(0, componentLength);
    QString componentName = getNextComponentName(currentSubcircuitName);
    QString node1Name = getNodeNameFromPoint(startPoint);
    QString node2Name = getNodeNameFromPoint(endPoint);
    circuit_ptr->addComponent(currentSubcircuitName.toStdString(), componentName.toStdString(), node1Name.toStdString(), node2Name.toStdString(), startPoint, placementIsHorizontal, 0.0, {}, {}, false);
}

void SchematicWidget::drawComponent(QPainter& painter, const QPoint& start, bool isHorizontal, QString type,
                                    bool isHovered) const {
    QPoint end = isHorizontal ? start + QPoint(componentLength, 0) : start + QPoint(0, componentLength);

    QPen componentPen(Qt::black, 2);
    componentPen.setColor(isHovered ? Qt::darkRed : Qt::black);
    painter.setPen(componentPen);
    // painter.drawLine(start, end);

    painter.drawLine(start, isHorizontal ? start + QPoint(gridSize, 0) : start + QPoint(0, gridSize));
    painter.drawLine(end, isHorizontal ? end - QPoint(gridSize, 0) : end - QPoint(0, gridSize));
    QPoint center = (start + end) / 2;

    painter.setBrush(Qt::white);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    if (type.startsWith("V")) {
        painter.drawEllipse(center, gridSize/2, gridSize/2);
        painter.drawText(center + QPoint(-4,4), type);
        QPen polarityPen(Qt::black, 2);
        painter.setPen(polarityPen);
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        if (isHorizontal) {
            painter.drawText(start - QPoint(20,-7), "+");
            painter.drawText(end + QPoint(5, 7), "-");
        }
        else {
            painter.drawText(start - QPoint(5,10), "+");
            painter.drawText(end + QPoint(-5, 25), "-");
        }
    }
    else if (type.startsWith("I")) {
        painter.drawEllipse(center, gridSize/2, gridSize/2);
        painter.drawText(center + QPoint(-4, 4), type);
        QPen arrowPen(Qt::darkBlue, 2);
        painter.setPen(arrowPen);
        QPoint arrowStart, arrowEnd;
        if (isHorizontal) {
            arrowStart = center - QPoint(15, 0);
            arrowEnd = center + QPoint(15, 0);
            painter.drawLine(arrowStart, arrowEnd);
            painter.drawLine(arrowEnd, arrowEnd - QPoint(10, 5));
            painter.drawLine(arrowEnd, arrowEnd - QPoint(10, -5));
        }
        else {
            arrowStart = center - QPoint(0, 15);
            arrowEnd = center + QPoint(0, 15);
            painter.drawLine(arrowStart, arrowEnd);
            painter.drawLine(arrowEnd, arrowEnd - QPoint(5, 10));
            painter.drawLine(arrowEnd, arrowEnd - QPoint(-5, 10));
        }
    }
    else {
        QRectF componentBody(center - QPoint(gridSize, 10), QSize(2 * gridSize, 20));
        painter.drawRect(componentBody);
        painter.drawText(componentBody, Qt::AlignCenter, type);
    }
}

QString SchematicWidget::findNodeAt(const QPoint& nodePos) {
    const auto& components = circuit_ptr->getComponentGraphics();
    for (const auto& component : components) {
        QPoint start = component.startPoint;
        QPoint end = component.isHorizontal ? start + QPoint(componentLength, 0) : start + QPoint(0, componentLength);
        if (nodePos == start || nodePos == end)
            return nodePos == start ? getNodeNameFromPoint(start) : getNodeNameFromPoint(end);
    }

    for (const auto& wire : circuit_ptr->getWires()) {
        QRect wireRect(wire.startPoint, wire.endPoint);
        wireRect = wireRect.normalized().adjusted(-5, -5, 5, 5);
        if (wireRect.contains(nodePos))
            return QString::fromStdString(wire.nodeName);
    }

    return getNodeNameFromPoint(nodePos);
}

void SchematicWidget::handleNodeLibraryItemSelection(const QString& compType) {
    if (compType.startsWith("U:")) {
        currentSubcircuitName = compType.mid(2);
        startPlacingSubcircuit();
    }
    else if (compType == "R")
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
    else if (compType == "AC")
        startPlacingACVoltageSource();
    else
        QMessageBox::information(this, "Dependent source", "Buy premium to access this element!");
}

QString SchematicWidget::findOrCreateNodeAtPoint(const QPoint& point) {
    const auto& components = circuit_ptr->getComponentGraphics();
    for (const auto& compInfo : components) {
        QPoint start = compInfo.startPoint;
        QPoint end = compInfo.isHorizontal
                         ? start + QPoint(componentLength, 0)
                         : start + QPoint(0, componentLength);
        if (point == start)
            return getNodeNameFromPoint(start);
        if (point == end)
            return getNodeNameFromPoint(end);
    }

    for (const auto& wire : circuit_ptr->getWires()) {
        QRect wireRect(wire.startPoint, wire.endPoint);
        wireRect = wireRect.normalized().adjusted(-5, -5, 5, 5);
        if (wireRect.contains(point))
            return QString::fromStdString(wire.nodeName);
    }

    return getNodeNameFromPoint(point);
}

void SchematicWidget::selectingSubcircuitNodesMouseEvent(QMouseEvent* event) {
    QPoint clickPos = stickToGrid(event->pos());
    QString nodeName = findNodeAt(clickPos);

    if (nodeName.isEmpty()) {
        QMessageBox::warning(this, "Node Selection Error", "No node found at this position. Please click on a valid node.");
        return;
    }
    subcircuitNodes.push_back(nodeName);
    if (subcircuitNodes.size() == 1) {
        QMessageBox::information(this, "Create Subcircuit", QString("First node '%1' selected. Please select the second node.").arg(nodeName));
    }
    else if (subcircuitNodes.size() == 2) {
        bool ok;
        QString subcircuitName = QInputDialog::getText(this, "Subcircuit Name", "Enter a name for the new subcircuit:", QLineEdit::Normal, "", &ok);
        if (ok && !subcircuitName.isEmpty()) {
            circuit_ptr->createSubcircuitDefinition(subcircuitName.toStdString(), subcircuitNodes[0].toStdString(), subcircuitNodes[1].toStdString());
            try {
                const auto& newSubDef = circuit_ptr->subcircuitDefinitions.at(subcircuitName.toStdString());
                circuit_ptr->saveSubcircuitToFile(newSubDef);
                QMessageBox::information(this, "Success", QString("Subcircuit '%1' created and saved to library.").arg(subcircuitName));
            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error", QString("Failed to save subcircuit to file: %1").arg(e.what()));
            }
        }
        currentMode = InteractionMode::Normal;
        setCursor(Qt::ArrowCursor);
    }
}