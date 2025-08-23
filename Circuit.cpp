#include <iomanip>
#include <utility>
#include <cctype>
#include <QFile>
#include "circuit.h"
namespace fs = std::filesystem;


// -------------------------------- Helper for parsing values --------------------------------
double parseSpiceValue(const std::string& valueStr) {
    if (valueStr.empty())
        throw std::runtime_error("Empty value.");

    std::string s_lower = valueStr;
    std::transform(s_lower.begin(), s_lower.end(), s_lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    std::string numPart;
    double multiplier = 1.0;
    if (s_lower.length() > 3 && s_lower.rfind("meg") == s_lower.length() - 3) {
        multiplier = 1e6;
        numPart = valueStr.substr(0, valueStr.length() - 3);
    }
    else if (!s_lower.empty() && !isdigit(s_lower.back())) {
        char suffix = s_lower.back();
        bool found_suffix = true;
        switch (suffix) {
        case 'k': multiplier = 1e3;
            break;
        case 'u': multiplier = 1e-6;
            break;
        case 'n': multiplier = 1e-9;
            break;
        case 'm': multiplier = 1e-3;
            break;
        default:
            found_suffix = false;
            break;
        }
        if (found_suffix)
            numPart = valueStr.substr(0, valueStr.length() - 1);
        else
            numPart = valueStr;
    }
    else
        numPart = valueStr;
    return std::stod(numPart) * multiplier;
}
// -------------------------------- Helper for parsing values --------------------------------


// -------------------------------- Constructors and Destructors --------------------------------
Circuit::Circuit() : nextNodeId(0), numCurrentUnknowns(0), hasNonlinearComponents(false) { }

Circuit::~Circuit() {}
// -------------------------------- Constructors and Destructors --------------------------------


// -------------------------------- File Management --------------------------------
const std::vector<ComponentGraphicalInfo>& Circuit::getComponentGraphics() const {
    return componentGraphics;
}

const std::vector<WireInfo>& Circuit::getWires() const {
    return wires;
}

const std::vector<LabelInfo>& Circuit::getLabels() const {
    return labels;
}

const std::vector<GroundInfo>& Circuit::getGrounds() const {
    return grounds;
}

QString getSubcircuitLibraryPath();
void Circuit::saveSubcircuitToFile(const SubcircuitDefinition& subDef) {
    QString libraryPath = getSubcircuitLibraryPath();
    QString filePath = libraryPath + "/" + QString::fromStdString(subDef.name) + ".sub";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("Cannot open file for writing subcircuit: " + filePath.toStdString());
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_5);
    out << subDef; // We already have the stream operator for this
    file.close();
}

void Circuit::saveToFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        throw std::runtime_error("Cannot open file for writing: " + filePath.toStdString());
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_5);

    out << componentGraphics;
    out << wires;
    out << labels;
    out << grounds;
    out << subcircuitDefinitions;
    out << (quint32)components.size()
    ;
    for (const auto& comp : components) {
        out << comp->getTypeString();
        comp->serialize(out);
    }

    out << nodeNameToId;
    out << idToNodeName;
    out << (qint32)nextNodeId;
    out << groundNodeIds;
}

void Circuit::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        throw std::runtime_error("Cannot open file for reading: " + filePath.toStdString());

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_5);

    clearSchematic();

    in >> componentGraphics;
    in >> wires;
    in >> labels;
    in >> grounds;
    in >> subcircuitDefinitions;

    quint32 componentCount;
    in >> componentCount;
    for (quint32 i = 0; i < componentCount; ++i) {
        QString typeString;
        in >> typeString;
        std::shared_ptr<Component> newComp = ComponentFactory::createComponentFromType(typeString);
        if (newComp) {
            newComp->deserialize(in);
            components.push_back(newComp);
            if (newComp->isNonlinear())
                hasNonlinearComponents = true;
        }
        else
            throw std::runtime_error("Unknown component type in file: " + typeString.toStdString());
    }

    qint32 loadedNextNodeId;
    in >> nodeNameToId;
    in >> idToNodeName;
    in >> loadedNextNodeId;
    in >> groundNodeIds;
    nextNodeId = loadedNextNodeId;
}
// -------------------------------- File Management --------------------------------


// -------------------------------- Component and Node Management --------------------------------
void Circuit::mergeNodes(int sourceNodeId, int destNodeId) {
    if (sourceNodeId == destNodeId)
        return;

    for (auto& comp : components) {
        if (comp->node1 == sourceNodeId)
            comp->node1 = destNodeId;
        if (comp->node2 == sourceNodeId)
            comp->node2 = destNodeId;
    }

    std::string sourceName = idToNodeName[sourceNodeId];
    nodeNameToId[sourceName] = destNodeId;

    for (auto& pair : labelToNodes) {
        if (pair.second.count(sourceNodeId)) {
            pair.second.erase(sourceNodeId);
            pair.second.insert(destNodeId);
        }
    }

    if (groundNodeIds.count(sourceNodeId)) {
        groundNodeIds.erase(sourceNodeId);
        groundNodeIds.insert(destNodeId);
    }

    idToNodeName.erase(sourceNodeId);
}

void Circuit::clearSchematic() {
    components.clear();
    nodeNameToId.clear();
    idToNodeName.clear();
    componentCurrentIndices.clear();
    nextNodeId = 0;
    numCurrentUnknowns = 0;
    hasNonlinearComponents = false;
    circuitNetList.clear();
    groundNodeIds.clear();
    labelToNodes.clear();
    wires.clear();
    labels.clear();
    grounds.clear();
    componentGraphics.clear();
}

int Circuit::getNodeId(const std::string& nodeName, bool create) {
    if (nodeNameToId.find(nodeName) == nodeNameToId.end()) {
        if (create) {
            nodeNameToId[nodeName] = nextNodeId;
            idToNodeName[nextNodeId] = nodeName;
            return nextNodeId++;
        }
        return -1;
    }
    return nodeNameToId[nodeName];
}

int Circuit::getNodeId(const std::string& nodeName) const {
    auto it = nodeNameToId.find(nodeName);
    if (it != nodeNameToId.end())
        return it->second;
    return -1;
}

bool Circuit::hasNode(const std::string& nodeName) const {
    return nodeNameToId.count(nodeName);
}

void Circuit::addComponent(const std::string& typeStr, const std::string& name, const std::string& node1Str, const std::string& node2Str, double value, const std::vector<double>& numericParams, const std::vector<std::string>& stringParams, bool isSinusoidal) {
    int n1_id = getNodeId(node1Str, true);
    int n2_id = getNodeId(node2Str, true);
    try {
        Component* newComp = ComponentFactory::createComponent(typeStr, name, n1_id, n2_id, value, numericParams,
                                                               stringParams, isSinusoidal, this);
        if (newComp) {
            components.push_back(std::shared_ptr<Component>(newComp));
            if (newComp->isNonlinear())
                hasNonlinearComponents = true;
            std::cout << "Added " << name << "." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << std::endl;
    }
}

    void Circuit::addComponent(const std::string& typeStr, const std::string& name,
                           const std::string& node1Str, const std::string& node2Str,
                           const QPoint& startPoint, bool isHorizontal,
                           double value, const std::vector<double>& numericParams,
                           const std::vector<std::string>& stringParams, bool isSinusoidal) {
    for (const auto& comp : components) {
        if (comp->name == name) {
            std::string errorMsg;
            if (comp->type == Component::Type::RESISTOR)
                errorMsg = "Resistor ";
            else if (comp->type == Component::Type::CAPACITOR)
                errorMsg = "Capacitor ";
            else if (comp->type == Component::Type::INDUCTOR)
                errorMsg = "Inductor ";
            else if (comp->type == Component::Type::DIODE)
                errorMsg = "Diode ";
            else if (comp->type == Component::Type::VOLTAGE_SOURCE)
                errorMsg = "Voltage source ";
            else if (comp->type == Component::Type::CURRENT_SOURCE)
                errorMsg = "Current source ";
            else
                errorMsg = "Component ";

            errorMsg += comp->name + " already exists in the circuit.";
            throw std::runtime_error(errorMsg);
        }
    }

    if (subcircuitDefinitions.count(typeStr)) {
        const SubcircuitDefinition& subDef = subcircuitDefinitions.at(typeStr);
        std::cout << "Unrolling subcircuit: " << name << " of type " << typeStr << std::endl;

        std::map<std::string, std::string> nodeMap;
        nodeMap[subDef.port1NodeName] = node1Str;
        nodeMap[subDef.port2NodeName] = node2Str;

        for (const std::string& line : subDef.netlist) {
            std::stringstream ss(line);
            std::string subCompTypeStr, subCompName, subNode1, subNode2, subValueStr;
            ss >> subCompTypeStr >> subCompName >> subNode1 >> subNode2 >> subValueStr;

            std::string newCompName = name + "_" + subCompName;

            if (!nodeMap.count(subNode1))
                nodeMap[subNode1] = name + "_" + subNode1;
            if (!nodeMap.count(subNode2))
                nodeMap[subNode2] = name + "_" + subNode2;

            addComponent(subCompTypeStr, newCompName, nodeMap.at(subNode1), nodeMap.at(subNode2), parseSpiceValue(subValueStr), {}, {}, false);
        }

        componentGraphics.push_back({startPoint, isHorizontal, name});
        return;
    }

    int n1_id = getNodeId(node1Str, true);
    int n2_id = getNodeId(node2Str, true);
    try {
        Component* newComp = ComponentFactory::createComponent(typeStr, name, n1_id, n2_id, value, numericParams,
                                                               stringParams, isSinusoidal, this);
        if (newComp) {
            componentGraphics.push_back({startPoint, isHorizontal, name});
            components.push_back(std::shared_ptr<Component>(newComp));
            if (newComp->isNonlinear())
                hasNonlinearComponents = true;
            std::cout << "Added " << name << "." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << std::endl;
    }
}

std::shared_ptr<Component> Circuit::getComponent(const std::string& name) const {
    for (const auto& comp : components) {
        if (comp->name == name) {
            return comp;
        }
    }
    return nullptr;
}

bool Circuit::isGround(int nodeId) const {
    return groundNodeIds.count(nodeId);
}

void Circuit::addGround(const std::string& nodeName, const QPoint& position) {
    int nodeId = getNodeId(nodeName, true);
    if (!isGround(nodeId)) {
        groundNodeIds.insert(nodeId);
        grounds.push_back({position});
        std::cout << "Ground added." << std::endl;
    }
}

void Circuit::addWire(const QPoint& start, const QPoint& end, const std::string& nodeName) {
    wires.push_back({start, end, nodeName});
}

void Circuit::deleteComponent(const std::string& componentName, char typeChar) {
components.erase(std::remove_if(components.begin(), components.end(), [&](const std::shared_ptr<Component>& comp) {
    if (comp->getName() == componentName) {
            return true;
        }
        return false;
    }), components.end());
    componentGraphics.erase(std::remove_if(componentGraphics.begin(), componentGraphics.end(), [&](const ComponentGraphicalInfo& g) {
        return g.name == componentName;
    }), componentGraphics.end());
    circuitNetList.erase(std::remove_if(circuitNetList.begin(), circuitNetList.end(), [&](const std::string& line) {
        return line.find(componentName) != std::string::npos;
    }), circuitNetList.end());
}

void Circuit::deleteGround(const std::string& nodeName) {
    if (!nodeNameToId.count(nodeName)) {
        std::cerr << "Cannot delete ground: Node '" << nodeName << "' does not exist." << std::endl;
        return;
    }

    int nodeId = nodeNameToId.at(nodeName);
    if (!groundNodeIds.count(nodeId)) {
        std::cerr << "Cannot delete ground: Node '" << nodeName << "' is not a ground node." << std::endl;
        return;
    }

    groundNodeIds.erase(nodeId);
    QPoint groundPos;
    QString qNodeName = QString::fromStdString(nodeName);
    QStringList parts = qNodeName.split('_');
    if (parts.size() == 3) {
        groundPos.setX(parts[1].toInt() * 40); // gridSize = 40
        groundPos.setY(parts[2].toInt() * 40);
    }

    grounds.erase(std::remove_if(grounds.begin(), grounds.end(), [&](const GroundInfo& g) {
        return g.position == groundPos;
    }), grounds.end());

    std::cout << "Ground at node '" << nodeName << "' deleted." << std::endl;
}

void Circuit::listNodes() const {
    std::cout << "Available nodes:" << std::endl;
    for (int i = 0; i < idToNodeName.size(); i++) {
        if (i == idToNodeName.size() - 1) {
            std::cout << idToNodeName.at(i);
            break;
        }
        std::cout << idToNodeName.at(i) << ", ";
    }
    std::cout << std::endl;
}

void Circuit::listComponents(char typeFilter) const {
    if (!typeFilter)
        for (const auto& component : components)
            std::cout << component->name << " " << idToNodeName.at(component->node1) << " " << idToNodeName.
                at(component->node2) << " " << component->value << std::endl;
    else
        for (const auto& component : components)
            if (component->name[0] == typeFilter)
                std::cout << component->name << " " << idToNodeName.at(component->node1) << " " << idToNodeName.
                    at(component->node2) << " " << component->value << std::endl;
}

void Circuit::renameNode(const std::string& oldName, const std::string& newName) {
    if (nodeNameToId.find(oldName) == nodeNameToId.end()) {
        std::cout << "ERROR: Node " << oldName << " does not exist." << std::endl;
        return;
    }
    if (nodeNameToId.count(newName)) {
        std::cout << "ERROR: Node " << newName << " already exists." << std::endl;
        return;
    }
    int nodeId = nodeNameToId[oldName];
    nodeNameToId.erase(oldName);
    nodeNameToId[newName] = nodeId;
    idToNodeName[nodeId] = newName;
    std::cout << "SUCCESS: Node renamed from " << oldName << " to " << newName << std::endl;
    for (auto it = circuitNetList.begin(); it != circuitNetList.end(); it++) {
        size_t i = 0;
        if ((i = it->find(oldName)) != std::string::npos) {
            it->erase(i, oldName.size());
            it->insert(i, newName);
        }
    }
}

std::vector<std::string> Circuit::generateNetlistFromComponents() const {
    std::vector<std::string> netlist;
    for (const auto& comp : components) {
        std::string line;
        std::string type_char = comp->name.substr(0, 1);
        std::string n1_name = idToNodeName.at(comp->node1);
        std::string n2_name = idToNodeName.at(comp->node2);

        if (dynamic_cast<Resistor*>(comp.get()) || dynamic_cast<Capacitor*>(comp.get()) || dynamic_cast<Inductor*>(comp.get()))
            line = type_char + " " + comp->name + " " + n1_name + " " + n2_name + " " + std::to_string(comp->value);
        else if (auto* vs = dynamic_cast<VoltageSource*>(comp.get())) {
            if (vs->getSourceType() == VoltageSource::SourceType::DC)
                line = type_char + " " + comp->name + " " + n1_name + " " + n2_name + " " + std::to_string(vs->getParam1());
            else
                line = type_char + " " + comp->name + " " + n1_name + " " + n2_name + " SIN(" + std::to_string(vs->getParam1()) + " " + std::to_string(vs->getParam2()) + " " + std::to_string(vs->getParam3()) + ")";
        }
        else if (auto* cs = dynamic_cast<CurrentSource*>(comp.get())) {
            if (cs->getSourceType() == CurrentSource::SourceType::DC)
                line = type_char + " " + comp->name + " " + n1_name + " " + n2_name + " " + std::to_string(cs->getParam1());
            else
                line = type_char + " " + comp->name + " " + n1_name + " " + n2_name + " SIN(" + std::to_string(cs->getParam1()) + " " + std::to_string(cs->getParam2()) + " " + std::to_string(cs->getParam3()) + ")";
        }
        else if (dynamic_cast<Diode*>(comp.get()))
            line = type_char + " " + comp->name + " " + n1_name + " " + n2_name + " D";
        else if (auto* vcvs = dynamic_cast<VCVS*>(comp.get()))
            line = type_char + " " + comp->name + " " + n1_name + " " + n2_name + " " + idToNodeName.at(vcvs->getCtrlNode1()) + " " + idToNodeName.at(vcvs->getCtrlNode2()) + " " + std::to_string(vcvs->getGain());
        else if (auto* vccs = dynamic_cast<VCCS*>(comp.get()))
            line = type_char + " " + comp->name + " " + n1_name + " " + n2_name + " " + idToNodeName.at(vccs->getCtrlNode1()) + " " + idToNodeName.at(vccs->getCtrlNode2()) + " " + std::to_string(vccs->getGain());
// TODO: Other sources
        if (!line.empty()) {
            netlist.push_back(line);
        }
    }
    return netlist;
}

void Circuit::connectNodes(const std::string& nodeAStr, const std::string& nodeBStr) {
    int nodeAInt = getNodeId(nodeAStr, true);
    int nodeBInt = getNodeId(nodeBStr, true);
    int sourceNodeId = std::max(nodeAInt, nodeBInt);
    int destNodeId = std::min(nodeAInt, nodeBInt);
    if (sourceNodeId != destNodeId) {
        mergeNodes(sourceNodeId, destNodeId);
    }
    std::cout << "Node '" << nodeAStr << "' successfully connected to '" << nodeBStr << "'." << std::endl;
}

void Circuit::addLabel(const QPoint& pos, const std::string& labelName, const std::string& nodeName) {
    int nodeId = getNodeId(nodeName, true);
    if (nodeId != -1) {
        labelToNodes[labelName].insert(nodeId);
        labels.push_back({pos, labelName, nodeName});
        std::cout << "Label '" << labelName << "' added to node " << nodeName << std::endl;
    }
}

void Circuit::processLabelConnections() {
    for (const auto& pair : labelToNodes) {
        const std::set<int>& nodes = pair.second;
        if (nodes.size() > 1) {
            int destNodeId = *nodes.begin();
            for (auto it = std::next(nodes.begin()); it != nodes.end(); ++it)
                mergeNodes(*it, destNodeId);
        }
    }
}

void Circuit::createSubcircuitDefinition(const std::string& name, const std::string& node1, const std::string& node2) {
    if (subcircuitDefinitions.count(name)) {
        std::cout << "Error: A subcircuit with this name exist." << std::endl;
        return;
    }
    SubcircuitDefinition newSubcircuit;
    newSubcircuit.name = name;
    newSubcircuit.port1NodeName = node1;
    newSubcircuit.port2NodeName = node2;
    newSubcircuit.netlist = generateNetlistFromComponents();
    subcircuitDefinitions[name] = newSubcircuit;
}
// -------------------------------- Component and Node Management --------------------------------


// -------------------------------- MNA and Solver --------------------------------
void Circuit::buildMNAMatrix(double time, double h) {
    processLabelConnections();
    std::map<int, int> nodeIdToMnaIndex;
    int currentMnaIndex = 0;
    for (int i = 0; i < nextNodeId; ++i) {
        if (!isGround(i) && idToNodeName.count(i)) {
            nodeIdToMnaIndex[i] = currentMnaIndex++;
        }
    }

    int node_count = nodeIdToMnaIndex.size();

    numCurrentUnknowns = 0;
    componentCurrentIndices.clear();
    for (const auto& comp : components) {
        if (comp->needsCurrentUnknown()) {
            componentCurrentIndices[comp->name] = node_count + numCurrentUnknowns;
            numCurrentUnknowns++;
        }
    }

    int matrix_size = node_count + numCurrentUnknowns;
    if (matrix_size <= 0) {
        A_mna.resize(0, 0);
        b_mna.resize(0);
        return;
    }
    if (A_mna.rows() != matrix_size) {
        A_mna.resize(matrix_size, matrix_size);
        b_mna.resize(matrix_size);
    }
    A_mna.setZero();
    b_mna.setZero();

    for (const auto& comp : components) {
        int idx = -1;
        if (comp->needsCurrentUnknown()) {
            idx = componentCurrentIndices.at(comp->name);
        }
        comp->stampMNA(A_mna, b_mna, componentCurrentIndices, nodeIdToMnaIndex, time, h, idx);
    }
}

void Circuit::buildMNAMatrix_AC(double omega) {
    processLabelConnections();
    std::map<int, int> nodeIdToMnaIndex;
    int currentMnaIndex = 0;
    for (int i = 0; i < nextNodeId; ++i) {
        if (!isGround(i) && idToNodeName.count(i)) {
            nodeIdToMnaIndex[i] = currentMnaIndex++;
        }
    }

    int node_count = nodeIdToMnaIndex.size();
    numCurrentUnknowns = 0;
    componentCurrentIndices.clear();
    for (const auto& comp : components) {
        if (comp->needsCurrentUnknown()) {
            componentCurrentIndices[comp->name] = node_count + numCurrentUnknowns;
            numCurrentUnknowns++;
        }
    }
    int matrix_size = node_count + numCurrentUnknowns;
    if (matrix_size <= 0)
        return;
    A_mna.resize(matrix_size, matrix_size);
    b_mna.resize(matrix_size);
    A_mna.setZero();
    b_mna.setZero();

    for (const auto& comp : components) {
        int idx = -1;
        if (comp->needsCurrentUnknown()) {
            idx = componentCurrentIndices.at(comp->name);
        }
        comp->stampMNA_AC(A_mna, b_mna, componentCurrentIndices, nodeIdToMnaIndex, omega, idx);
    }
}

Eigen::VectorXd Circuit::solveMNASystem() {
    if (A_mna.rows() == 0) {
        std::cout << "MNA matrix is empty. Cannot solve." << std::endl;
        return Eigen::VectorXd();
    }

    Eigen::FullPivLU<Eigen::MatrixXd> lu(A_mna);
    if (!lu.isInvertible()) {
        std::cout << "ERROR: Circuit matrix is singular. Check for floating nodes or invalid connections." << std::endl;
        return Eigen::VectorXd(); // Return empty vector
    }
    return lu.solve(b_mna);
}

void Circuit::updateComponentStates(const Eigen::VectorXd& solution, const std::map<int, int>& nodeIdToMnaIndex) {
    for (const auto& comp : components) {
        comp->updateState(solution, componentCurrentIndices, nodeIdToMnaIndex);
    }
}

void Circuit::updateNonlinearComponentStates(const Eigen::VectorXd& solution,
                                             const std::map<int, int>& nodeIdToMnaIndex) {
    for (const auto& comp : components) {
        if (comp->isNonlinear()) {
            comp->updateState(solution, componentCurrentIndices, nodeIdToMnaIndex);
        }
    }
}
// -------------------------------- MNA and Solver --------------------------------


// -------------------------------- Analysis Methods --------------------------------
    void Circuit::runTransientAnalysis(double stopTime, double startTime, double maxTimeStep) {
    if (maxTimeStep == 0.0)
        maxTimeStep = (stopTime - startTime) / 100;
    std::cout << "\n---------- Performing Transient Analysis ----------" << std::endl;
    std::cout << "Time Start: " << startTime << "s, Stop Time: " << stopTime << "s, Maximum Time Step: " << maxTimeStep << "s" << std::endl;

    if (groundNodeIds.empty()) {
        std::cout << "No ground node detected." << std::endl;
        return;
    }

    for (const auto& comp : components)
        comp->reset();
    transientSolutions.clear();

    std::map<int, int> nodeIdToMnaIndex;
    Eigen::VectorXd solution;

    int currentMnaIndex = 0;
    for (int i = 0; i < nextNodeId; ++i) {
        if (idToNodeName.count(i) && !isGround(i)) {
            nodeIdToMnaIndex[i] = currentMnaIndex++;
        }
    }

    double t = startTime;
    double h = maxTimeStep;
    const double h_min = 1e-12;

    while (t < stopTime) {
        if (t + h > stopTime)
            h = stopTime - t;

        Eigen::VectorXd solution;
        bool stepSucceeded = false;

        while (!stepSucceeded) {
            if (h < h_min) {
                 std::cout << "ERROR at t = " << t << "s: Timestep fell below minimum. Simulation stopped." << std::endl;
                 return;
            }

            bool nr_converged = false;
            if (!hasNonlinearComponents) {
                buildMNAMatrix(t + h, h);
                solution = solveMNASystem();
                if (solution.size() > 0)
                    nr_converged = true;
            }
            else {
                const int MAX_ITERATIONS = 100;
                const double TOLERANCE = 1e-6;
                Eigen::VectorXd lastSolution;
                for (int i = 0; i < MAX_ITERATIONS; ++i) {
                    buildMNAMatrix(t + h, h);
                    solution = solveMNASystem();
                    if (solution.size() == 0) {
                        nr_converged = false;
                        break;
                    }
                    if (i > 0 && (solution - lastSolution).norm() < TOLERANCE) {
                        nr_converged = true;
                        break;
                    }
                    lastSolution = solution;
                    updateNonlinearComponentStates(solution, nodeIdToMnaIndex);
                }
            }

            if (nr_converged)
                stepSucceeded = true;
            else
                h = h / 2.0;
        }
        t += h;
        updateComponentStates(solution, nodeIdToMnaIndex);
        transientSolutions[t] = solution;

        h = maxTimeStep;
    }

    std::cout << "Transient analysis complete. " << transientSolutions.size() << " time points stored." << std::endl;
}

void Circuit::runACAnalysis(double startOmega, double stopOmega, int numPoints) {
    if (groundNodeIds.empty())
        throw std::runtime_error("No ground node detected.");

    bool acSourceFound = false;
    for (const auto& comp : components) {
        if (comp->type == Component::Type::AC_VOLTAGE_SOURCE) {
            acSourceFound = true;
            break;
        }
    }
    if (!acSourceFound)
        throw std::runtime_error("AC Sweep failed. No AC source found.");

    acSweepSolutions.clear();
    double omegaStep = (numPoints > 1) ? (stopOmega - startOmega) / (numPoints - 1) : 0;

    for (double w = omegaStep; w <= stopOmega; w += omegaStep) {
        buildMNAMatrix_AC(w);
        Eigen::VectorXd solution = solveMNASystem();
        if (solution.size() > 0)
            acSweepSolutions[w] = solution;
        else
            throw std::runtime_error("AC Analysis failed.");
    }
    std::cout << "AC Sweep complete. " << acSweepSolutions.size() << " frequency points stored." << std::endl;
}
// -------------------------------- Analysis Methods --------------------------------


// -------------------------------- Output Results --------------------------------
std::map<std::string, std::map<double, double>> Circuit::getTransientResults(const std::vector<std::string>& variablesToPrint) const {
    std::map<std::string, std::map<double, double>> results;

    if (transientSolutions.empty()) {
        std::cout << "No analysis results found. Run .TRAN or .DC first." << std::endl;
        return {};
    }

    std::map<int, int> nodeIdToMnaIndex;
    int currentMnaIndex = 0;
    for (int i = 0; i < nextNodeId; ++i) {
        if (idToNodeName.count(i) && !isGround(i)) {
            nodeIdToMnaIndex[i] = currentMnaIndex++;
        }
    }

    struct PrintJob {
        std::string header;
        enum class Type { VOLTAGE, MNA_CURRENT, RESISTOR_CURRENT, CAPACITOR_CURRENT } type;
        int index = -1;
        std::shared_ptr<Component> component_ptr = nullptr;
    };
    std::vector<PrintJob> printJobs;

    for (const auto& var : variablesToPrint) {
        if (var.length() < 4)
            continue;
        std::string type = var.substr(0, 1);
        std::string name = var.substr(2, var.length() - 3);

        if (type == "V") {
            if (!hasNode(name)) {
                std::cout << "Node " << name << " not found." << std::endl;
                return {};
            }
            int nodeID = nodeNameToId.at(name);
            int solutionIndex = isGround(nodeID) ? -1 : nodeIdToMnaIndex.at(nodeID);
            printJobs.push_back({var, PrintJob::Type::VOLTAGE, solutionIndex, nullptr});
        }
        else if (type == "I") {
            if (componentCurrentIndices.count(name))
                printJobs.push_back({var, PrintJob::Type::MNA_CURRENT, componentCurrentIndices.at(name), nullptr});
            else {
                if (componentCurrentIndices.count(name))
                    printJobs.push_back({var, PrintJob::Type::MNA_CURRENT, componentCurrentIndices.at(name), nullptr});
                else {
                    auto comp = getComponent(name);
                    if (!comp) {
                        std::cout << "Component " << name << " not found." << std::endl;
                        return {};
                    }
                    if (dynamic_cast<Resistor*>(comp.get()))
                        printJobs.push_back({var, PrintJob::Type::RESISTOR_CURRENT, -1, comp});
                    else if (dynamic_cast<Capacitor*>(comp.get()))
                        printJobs.push_back({var, PrintJob::Type::CAPACITOR_CURRENT, -1, comp});
                    else
                        std::cout << "Warning: Current for component type of '" << name << "' cannot be calculated." << std::endl;
                }
            }
        }
    }
    if (printJobs.empty())
        return {};

    for (const auto& job : printJobs)
        results[job.header];

    auto itPrev = transientSolutions.begin();
    for (auto it = transientSolutions.begin(); it != transientSolutions.end(); ++it) {
        double t = it->first;
        const Eigen::VectorXd& solution = it->second;

        for (const auto& job : printJobs) {
            double result = 0.0;
            if (job.type == PrintJob::Type::VOLTAGE || job.type == PrintJob::Type::MNA_CURRENT)
                result = (job.index == -1) ? 0.0 : solution(job.index);
            else {
                int node1 = job.component_ptr->node1;
                int node2 = job.component_ptr->node2;
                double v1 = isGround(node1) ? 0.0 : solution(nodeIdToMnaIndex.at(node1));
                double v2 = isGround(node2) ? 0.0 : solution(nodeIdToMnaIndex.at(node2));

                if (job.type == PrintJob::Type::RESISTOR_CURRENT)
                    result = (v1 - v2) / job.component_ptr->value;
                else if (job.type == PrintJob::Type::CAPACITOR_CURRENT) {
                    if (it == transientSolutions.begin())
                        result = 0.0;
                    else {
                        const Eigen::VectorXd& prevSolution = itPrev->second;
                        double v1_prev = isGround(node1) ? 0.0 : prevSolution(nodeIdToMnaIndex.at(node1));
                        double v2_prev = isGround(node2) ? 0.0 : prevSolution(nodeIdToMnaIndex.at(node2));
                        double vCap_prev = v1_prev - v2_prev;
                        double vCap_now = v1 - v2;
                        double h = t - itPrev->first;
                        if (h > 0)
                            result = job.component_ptr->value * (vCap_now - vCap_prev) / h;
                    }
                }
            }
            results.at(job.header)[t] = result;
        }
        itPrev = it;
    }
    return results;
}

std::map<std::string, std::map<double, double>> Circuit::getACSweepResults(const std::vector<std::string>& variables) const {
    std::map<std::string, std::map<double, double>> results;

    if (acSweepSolutions.empty())
        throw std::runtime_error("No AC analysis results found. Run .AC analysis first.");

    std::map<int, int> nodeIdToMnaIndex;
    int currentMnaIndex = 0;
    for (int i = 0; i < nextNodeId; ++i) {
        if (idToNodeName.count(i) && !isGround(i)) {
            nodeIdToMnaIndex[i] = currentMnaIndex++;
        }
    }

    for (const auto& var : variables)
        results[var];

    for (const auto& pair : acSweepSolutions) {
        double omega = pair.first;
        const Eigen::VectorXd& solution = pair.second;
        double resultValue = 0.0;

        for (const auto& variable : variables) {
            if (variable.length() < 4)
                continue;

            char varType = variable.front();
            std::string varName = variable.substr(2, variable.length() - 3);
            double resultValue = 0.0;

            if (varType == 'V') {
                int nodeId = getNodeId(varName);
                if (nodeId != -1)
                    resultValue = isGround(nodeId) ? 0.0 : solution(nodeIdToMnaIndex.at(nodeId));
            }
            else if (varType == 'I') {
                auto comp = getComponent(varName);
                if (!comp) continue;

                if (comp->needsCurrentUnknown() && componentCurrentIndices.count(varName))
                    resultValue = solution(componentCurrentIndices.at(varName));
                else {
                    double v1 = isGround(comp->node1) ? 0.0 : solution(nodeIdToMnaIndex.at(comp->node1));
                    double v2 = isGround(comp->node2) ? 0.0 : solution(nodeIdToMnaIndex.at(comp->node2));
                    double voltage_diff = v1 - v2;

                    if (auto* resistor = dynamic_cast<Resistor*>(comp.get()))
                        resultValue = voltage_diff / resistor->value;
                    else if (auto* capacitor = dynamic_cast<Capacitor*>(comp.get()))
                        resultValue = voltage_diff * omega * capacitor->value;
                }
            }
            results.at(variable)[omega] = resultValue;
        }
    }

    return results;
}
// -------------------------------- Output Results --------------------------------

template<typename T>
QDataStream& operator<<(QDataStream& out, const std::vector<T>& vec) {
    out << (quint32)vec.size();
    for (const T& item : vec) { out << item; }
    return out;
}
template<typename T>
QDataStream& operator>>(QDataStream& in, std::vector<T>& vec) {
    quint32 size;
    in >> size;
    vec.clear();
    vec.reserve(size);
    for (quint32 i = 0; i < size; ++i) {
        T item;
        in >> item;
        vec.push_back(item);
    }
    return in;
}

QDataStream& operator<<(QDataStream& out, const std::map<std::string, int>& map) {
    out << (quint32)map.size();
    for(const auto& pair : map) { out << QString::fromStdString(pair.first) << (qint32)pair.second; }
    return out;
}
QDataStream& operator>>(QDataStream& in, std::map<std::string, int>& map) {
    map.clear();
    quint32 size;
    in >> size;
    for(quint32 i=0; i<size; ++i) {
        QString key;
        qint32 val;
        in >> key >> val;
        map[key.toStdString()] = val;
    }
    return in;
}

QDataStream& operator<<(QDataStream& out, const std::map<int, std::string>& map) {
    out << (quint32)map.size();
    for(const auto& pair : map) { out << (qint32)pair.first << QString::fromStdString(pair.second); }
    return out;
}
QDataStream& operator>>(QDataStream& in, std::map<int, std::string>& map) {
    map.clear();
    quint32 size;
    in >> size;
    for(quint32 i=0; i<size; ++i) {
        qint32 key;
        QString val;
        in >> key >> val;
        map[key] = val.toStdString();
    }
    return in;
}

QDataStream& operator<<(QDataStream& out, const std::set<int>& set) {
    out << (quint32)set.size();
    for(int item : set) { out << (qint32)item; }
    return out;
}
QDataStream& operator>>(QDataStream& in, std::set<int>& set) {
    set.clear();
    quint32 size;
    in >> size;
    for(quint32 i=0; i<size; ++i) {
        qint32 item;
        in >> item;
        set.insert(item);
    }
    return in;
}

QDataStream& operator<<(QDataStream& out, const std::map<std::string, SubcircuitDefinition>& map) {
    out << (quint32)map.size();
    for (const auto& pair : map) { out << QString::fromStdString(pair.first) << pair.second; }
    return out;
}
QDataStream& operator>>(QDataStream& in, std::map<std::string, SubcircuitDefinition>& map) {
    map.clear();
    quint32 size;
    in >> size;
    for (quint32 i = 0; i < size; ++i) {
        QString key;
        SubcircuitDefinition val;
        in >> key >> val;
        map[key.toStdString()] = val;
    }
    return in;
}

QDataStream& operator<<(QDataStream& out, const ComponentGraphicalInfo& info) {
    out << info.startPoint << info.isHorizontal << QString::fromStdString(info.name);
    return out;
}
QDataStream& operator>>(QDataStream& in, ComponentGraphicalInfo& info) {
    QString name;
    in >> info.startPoint >> info.isHorizontal >> name;
    info.name = name.toStdString();
    return in;
}

QDataStream& operator<<(QDataStream& out, const WireInfo& info) {
    out << info.startPoint << info.endPoint << QString::fromStdString(info.nodeName);
    return out;
}
QDataStream& operator>>(QDataStream& in, WireInfo& info) {
    QString nodeName;
    in >> info.startPoint >> info.endPoint >> nodeName;
    info.nodeName = nodeName.toStdString();
    return in;
}

QDataStream& operator<<(QDataStream& out, const LabelInfo& info) {
    out << info.position << QString::fromStdString(info.name) << QString::fromStdString(info.connectedNodeName);
    return out;
}
QDataStream& operator>>(QDataStream& in, LabelInfo& info) {
    QString name, connectedNodeName;
    in >> info.position >> name >> connectedNodeName;
    info.name = name.toStdString();
    info.connectedNodeName = connectedNodeName.toStdString();
    return in;
}

QDataStream& operator<<(QDataStream& out, const GroundInfo& info) {
    out << info.position;
    return out;
}
QDataStream& operator>>(QDataStream& in, GroundInfo& info) {
    in >> info.position;
    return in;
}

QDataStream& operator<<(QDataStream& out, const SubcircuitDefinition& def) {
    out << QString::fromStdString(def.name);
    out << (quint32)def.netlist.size();
    for(const auto& s : def.netlist) { out << QString::fromStdString(s); }
    out << QString::fromStdString(def.port1NodeName) << QString::fromStdString(def.port2NodeName);
    return out;
}
QDataStream& operator>>(QDataStream& in, SubcircuitDefinition& def) {
    QString name, port1, port2;
    in >> name;
    def.name = name.toStdString();
    quint32 netlistSize;
    in >> netlistSize;
    def.netlist.clear();
    def.netlist.reserve(netlistSize);
    for(quint32 i=0; i<netlistSize; ++i) {
        QString s;
        in >> s;
        def.netlist.push_back(s.toStdString());
    }
    in >> port1 >> port2;
    def.port1NodeName = port1.toStdString();
    def.port2NodeName = port2.toStdString();
    return in;
}