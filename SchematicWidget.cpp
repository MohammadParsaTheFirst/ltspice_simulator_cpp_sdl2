#include "SchematicWidget.h"
#include <cmath>

SchematicWidget::SchematicWidget(Circuit *circuit, QWidget *parent) : circuit_ptr(circuit), QWidget(parent)
{
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

void SchematicWidget::paintEvent(QPaintEvent *event)
{
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

    for (int i = 0; i < components.size(); i++)
    {
        bool isHovered = (i == hoveredComponentIndex && currentMode == InteractionMode::deleteMode);
        drawComponent(painter, components[i].startPoint, components[i].isHorizontal, components[i].name, isHovered);
    }

    // Draw the preview before placing the component
    if (currentMode != InteractionMode::Normal && currentMode != InteractionMode::deleteMode) {
        QPoint startPos = stickToGrid(currentMousePos);
        drawComponent(painter, startPos, placementIsHorizontal, currentCompType);
    }
}

void SchematicWidget::startPlacingResistor()
{
    currentMode = InteractionMode::placingResistor;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    currentCompType = "R";
}

void SchematicWidget::startPlacingCapacitor()
{
    currentMode = InteractionMode::placingCapacitor;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    currentCompType = "C";
}

void SchematicWidget::startPlacingInductor()
{
    currentMode = InteractionMode::placingInductor;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    currentCompType = "L";
}

void SchematicWidget::startPlacingVoltageSource()
{
    currentMode = InteractionMode::placingVoltageSource;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    currentCompType = "V";
}

void SchematicWidget::startPlacingDiode()
{
    currentMode = InteractionMode::placingDiode;
    placementIsHorizontal = true;
    setCursor(Qt::CrossCursor);
    currentCompType = "D";
}

void SchematicWidget::startDeleteComponent()
{
    currentMode = InteractionMode::deleteMode;
    setCursor(Qt::OpenHandCursor);
}

void SchematicWidget::keyPressEvent(QKeyEvent* event)
{
    if (currentMode != InteractionMode::Normal && event->key() == Qt::Key_Control)
    {
        placementIsHorizontal = !placementIsHorizontal;
        update();
    }
    else if (currentMode != InteractionMode::Normal && event->key() == Qt::Key_Escape)
    {
        currentMode = InteractionMode::Normal;
        currentCompType = "NF";
        setCursor(Qt::ArrowCursor);
        update();
    }
    else
        QWidget::keyPressEvent(event);
}

QPoint SchematicWidget::stickToGrid(const QPoint& pos)
{
    int x = std::round(static_cast<double>(pos.x()) / gridSize) * gridSize;
    int y = std::round(static_cast<double>(pos.y()) / gridSize) * gridSize;
    return QPoint(x, y);
}

void SchematicWidget::mouseMoveEvent(QMouseEvent* event)
{
    currentMousePos = event->pos();
    if (currentMode == InteractionMode::deleteMode)
    {
        hoveredComponentIndex = -1;
        for (int i = 0;i < components.size(); i++)
        {
            QPoint start = components[i].startPoint;
            QPoint end = components[i].isHorizontal ? start + QPoint(componentLength, 0) : start + QPoint(0, componentLength);
            QRect componentRect(start,end);
            componentRect = componentRect.normalized().adjusted(-5,-5,5,5);

            if (componentRect.contains(currentMousePos))
            {
                hoveredComponentIndex = i;
                break;
            }
        }
    }
    update();
}

void SchematicWidget::mousePressEvent(QMouseEvent* event)
{
    if (currentMode == InteractionMode::Normal)
        return;

    if (event->button() == Qt::RightButton)
    {
        currentMode = InteractionMode::Normal;
        setCursor(Qt::ArrowCursor);
        update();
        return;
    }

    if (event->button() == Qt::LeftButton)
    {
        if (currentMode != InteractionMode::Normal && currentMode != InteractionMode::deleteMode)
        {
            QPoint startPoint = stickToGrid(event->pos());
            QPoint endPoint = placementIsHorizontal ? startPoint + QPoint(componentLength, 0) : startPoint + QPoint(0, componentLength);

            QString componentName = getNextComponentName(currentCompType);
            QString node1Name = getNodeNameFromPoint(startPoint);
            QString node2Name = getNodeNameFromPoint(endPoint);

            components.push_back({startPoint, placementIsHorizontal, componentName});

            circuit_ptr->addComponent(currentCompType.toStdString(), componentName.toStdString(), node1Name.toStdString(), node2Name.toStdString(), 1000.0,{}, {}, false);
            update();
        }
        else if (currentMode == InteractionMode::deleteMode)
        {
            QPoint clickPos = event->pos();

            for (auto it = components.rbegin(); it != components.rend(); it++)
            {
                QPoint start = it->startPoint;
                QPoint end;
                if (it->isHorizontal)
                    end = start + QPoint(componentLength, 0);
                else
                    end = start + QPoint(0, componentLength);

                QRect componentRect(start, end);
                componentRect = componentRect.normalized();
                componentRect.adjust(-5,-5,5,5);

                if (componentRect.contains(clickPos))
                {
                    components.erase(std::next(it).base());
                    // TODO: delete from circuit netlist

                    update();
                    return;
                }
            }
        }
    }
}

void SchematicWidget::drawComponent(QPainter& painter, const QPoint& start, bool isHorizontal, QString type, bool isHovered) const
{
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
