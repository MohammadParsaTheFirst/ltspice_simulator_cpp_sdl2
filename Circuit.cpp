#include "circuit.h"
#include <iomanip>
#include <utility>
#include <cctype>
#include <cereal/archives/binary.hpp>
namespace fs = std::filesystem;


// -------------------------------- Helper for parsing values --------------------------------
double parseSpiceValue(const std::string& valueStr) {
    if (valueStr.empty()) {
        throw std::runtime_error("Empty value");
    }

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
Circuit::Circuit() : nextNodeId(0), numCurrentUnknowns(0),
                     currentFilePath("C:\\Users\\parsa\\Documents\\university\\Programming and linux\\403101518-403101683.0\\Schematics\\draft.txt"),
                     hasNonlinearComponents(false) { loadSubcircuits(); }

Circuit::~Circuit() {
    for (Component* comp : components) {
        delete comp;
    }
}
// -------------------------------- Constructors and Destructors --------------------------------


// -------------------------------- File Management --------------------------------
void Circuit::newCircuit(const std::string& path) {
    for (Component* comp : components) {
        delete comp;
    }
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
    currentFilePath = path;
}

void Circuit::saveCircuitToFile() {
    std::ofstream outFile(currentFilePath);
    if (!outFile.is_open()) {
        std::cout << "Error: Could not open file '" << currentFilePath << std::endl;
        return;
    }
    outFile.close();
    for (std::string& lineToAddToFile : circuitNetList) {
        outFile << lineToAddToFile << std::endl;
    }
    outFile.close();
}

bool Circuit::loadCircuitFromFile() {
    clearSchematic();
    std::ifstream inFile(currentFilePath);
    if (!inFile.is_open())
        return false;
    std::string line;
    while (std::getline(inFile, line)) {
        makeComponentFromLine(line);
        circuitNetList.push_back(line);
    }
    inFile.close();
    return true;
}

void Circuit::makeComponentFromLine(const std::string& line) {
    if (line.empty() || line[0] == '*' || line[0] == ';')
        return;

    std::stringstream ss(line);
    std::string component_model, comp_name, node1_str, node2_str;
    if (!(ss >> component_model >> comp_name >> node1_str >> node2_str))
        throw std::runtime_error("Invalid 'add' format. Expected: add <type><name> <node1> <node2> ...");
    if (node1_str == node2_str)
        throw std::runtime_error("Nodes cannot be the same.");

    char type_char = component_model[0];

    double value = 0.0;
    std::string value_str;

    std::vector<double> numericParams;
    std::vector<std::string> stringParams;

    bool isSinusoidal = false;
    std::string model;

    if (type_char == 'R' || type_char == 'C' || type_char == 'L') {
        if (!(ss >> value_str))
            throw std::runtime_error("Missing value.");
        value = parseSpiceValue(value_str);
    }
    else if (type_char == 'V' || type_char == 'I') {
        std::string next_token;
        if (!(ss >> next_token))
            throw std::runtime_error("Missing source parameters.");
        if (next_token.find("SIN(") != std::string::npos) {
            isSinusoidal = true;
            std::string offset_str, amplitude_str, freq_str;
            offset_str = next_token.substr(4);
            ss >> amplitude_str;
            ss >> next_token;
            if (next_token.back() == ')')
                freq_str = next_token.substr(0, next_token.size() - 1);
            numericParams = {parseSpiceValue(offset_str), parseSpiceValue(amplitude_str), parseSpiceValue(freq_str)};
        }
        else
            value = parseSpiceValue(next_token);
    }
    else if (type_char == 'D') {
        if (!(ss >> model))
            throw std::runtime_error("Missing value.");
        if (model != "D" && model != "Z")
            throw std::runtime_error("Model " + model + " not found in library.");
    }
    else if (type_char == 'E' || type_char == 'G') {
        std::string c_n1, c_n2;
        if (!(ss >> c_n1 >> c_n2 >> value_str))
            throw std::runtime_error("Missing parameters for dependent source.");
        value = parseSpiceValue(value_str);
        stringParams = {c_n1, c_n2};
    }
    else if (type_char == 'H' || type_char == 'F') {
        std::string c_name;
        if (!(ss >> c_name >> value_str))
            throw std::runtime_error("Missing parameters for dependent source.");
        value = parseSpiceValue(value_str);
        stringParams = {c_name};
    }

    addComponent(std::string(1, type_char), comp_name, node1_str, node2_str, value, numericParams, stringParams, isSinusoidal);
}

void Circuit::saveSubcircuit(const SubcircuitDefinition& subDef) const {
    QString libDirPath = QCoreApplication::applicationDirPath() + QDir::separator() + "lib";
    QDir libDir(libDirPath);
    if (!libDir.exists())
        libDir.mkpath(".");
    QString filePath = libDirPath + QDir::separator() + QString::fromStdString(subDef.name) + ".sub";
    std::ofstream os(filePath.toStdString(), std::ios::binary);
    if (!os.is_open()) {
        std::cerr << "Error: Could not open file for saving subcircuit." << std::endl;
        return;
    }
    cereal::BinaryOutputArchive archive(os);
    archive(subDef);
}

void Circuit::loadSubcircuits() {
    QString libDirPath = QCoreApplication::applicationDirPath() + QDir::separator() + "lib";
    QDir libDir(libDirPath);
    if (!libDir.exists())
        return;
    for (const QString& fileName: libDir.entryList(QStringList() << "*.sub", QDir::Files)) {
        std::ifstream is(libDir.filePath(fileName).toStdString(), std::ios::binary);
        cereal::BinaryInputArchive archive(is);
        SubcircuitDefinition subDef;
        archive(subDef);
        subcircuitDefinitions[subDef.name] = subDef;
    }
    std::cout << subcircuitDefinitions.size() << " subcircuits loaded from directory 'lib'." << std::endl;
}
// -------------------------------- File Management --------------------------------


// -------------------------------- Component and Node Management --------------------------------
void Circuit::mergeNodes(int sourceNodeId, int destNodeId) {
    if (sourceNodeId == destNodeId)
        return;

    for (auto* comp : components) {
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

    idToNodeName.erase(sourceNodeId);
}

void Circuit::clearSchematic() {
    newCircuit(currentFilePath);
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

void Circuit::addComponent(const std::string& typeStr, const std::string& name,
                           const std::string& node1Str, const std::string& node2Str,
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
            std::string subCompType, subCompName, subNode1, subNode2;
            ss >> subCompType >> subCompName >> subNode1 >> subNode2;
            std::string newCompName = name + "_" + subCompName;
            if (!nodeMap.count(subNode1))
                nodeMap[subNode1] = name + "_" + subNode1;
            if (!nodeMap.count(subNode2))
                nodeMap[subNode2] = name + "_" + subNode2;
            std::string newLine = subCompType + " " + newCompName + " " + nodeMap[subNode1] + " " + nodeMap[subNode2];
            std::string remaining_params;
            std::getline(ss, remaining_params);
            newLine += remaining_params;
            circuitNetList.push_back(newLine);
        }
        for (const auto& line: subDef.netlist)
            makeComponentFromLine(line);
        return;
    }

    int n1_id = getNodeId(node1Str, true);
    int n2_id = getNodeId(node2Str, true);
    try {
        Component* newComp = ComponentFactory::createComponent(typeStr, name, n1_id, n2_id, value, numericParams,
                                                               stringParams, isSinusoidal, this);
        if (newComp) {
            components.push_back(newComp);
            if (newComp->isNonlinear())
                hasNonlinearComponents = true;
            std::cout << "Added " << name << "." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << std::endl;
    }
}

Component* Circuit::getComponent(const std::string& name) const {
    for (Component* comp : components) {
        if (comp->name == name) {
            return comp;
        }
    }
    return nullptr;
}

bool Circuit::isGround(int nodeId) const {
    return groundNodeIds.count(nodeId);
}

void Circuit::addGround(const std::string& nodeName) {
    int nodeId = getNodeId(nodeName, true);
    if (!isGround(nodeId)) {
        groundNodeIds.insert(nodeId);
        std::cout << "Ground added." << std::endl;
    }
}

void Circuit::deleteComponent(const std::string& componentName, char typeChar) {
    for (auto it = components.begin(); it != components.end(); it++) {
        if ((*it)->getName() == componentName) {
            delete *it;
            components.erase(it);
            for (auto it = circuitNetList.begin(); it != circuitNetList.end(); it++) {
                if (it->find(componentName) != std::string::npos) {
                    circuitNetList.erase(it);
                    break;
                }
            }
            std::cout << "Component " << componentName << " deleted." << std::endl;
            return;
        }
    }

    if (typeChar == 'R')
        throw std::runtime_error("Cannot delete resistor; component not found.");
    else if (typeChar == 'C')
        throw std::runtime_error("Cannot delete capacitor; component not found.");
    else if (typeChar == 'L')
        throw std::runtime_error("Cannot delete inductor; component not found.");
    else if (typeChar == 'D')
        throw std::runtime_error("Cannot delete diode; component not found.");
    else if (typeChar == 'V')
        throw std::runtime_error("Cannot delete voltage source; component not found.");
    else if (typeChar == 'I')
        throw std::runtime_error("Cannot delete current source; component not found.");
}

void Circuit::deleteGround(const std::string& ground_node_name) {
    if (!hasNode(ground_node_name))
        std::cout << "Node does not exist." << std::endl;
    else if (!isGround(nodeNameToId[ground_node_name]))
        std::cout << "This node isn't ground!" << std::endl;
    else {
        groundNodeIds.erase(nodeNameToId[ground_node_name]);
        std::cout << "Ground deleted." << std::endl;
    }
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
        for (Component* component : components)
            std::cout << component->name << " " << idToNodeName.at(component->node1) << " " << idToNodeName.
                at(component->node2) << " " << component->value << std::endl;

    else
        for (Component* component : components)
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

void Circuit::addLabel(const std::string& labelName, const std::string& nodeName) {
    int nodeId = getNodeId(nodeName, true);
    if (nodeId != -1) {
        labelToNodes[labelName].insert(nodeId);
        std::cout << "Label '" << labelName << "' added to node " << nodeName << std::endl;
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
    newSubcircuit.netlist = circuitNetList;
    subcircuitDefinitions[name] = newSubcircuit;
    saveSubcircuit(newSubcircuit);
}

// -------------------------------- Component and Node Management --------------------------------


// -------------------------------- MNA and Solver --------------------------------
void Circuit::buildMNAMatrix(double time, double h) {
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
    for (Component* comp : components) {
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

    for (Component* comp : components) {
        int idx = -1;
        if (comp->needsCurrentUnknown()) {
            idx = componentCurrentIndices.at(comp->name);
        }
        comp->stampMNA(A_mna, b_mna, componentCurrentIndices, nodeIdToMnaIndex, time, h, idx);
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
    for (Component* comp : components) {
        comp->updateState(solution, componentCurrentIndices, nodeIdToMnaIndex);
    }
}

void Circuit::updateNonlinearComponentStates(const Eigen::VectorXd& solution,
                                             const std::map<int, int>& nodeIdToMnaIndex) {
    for (Component* comp : components) {
        if (comp->isNonlinear()) {
            comp->updateState(solution, componentCurrentIndices, nodeIdToMnaIndex);
        }
    }
}
// -------------------------------- MNA and Solver --------------------------------


// -------------------------------- Analysis Methods --------------------------------
void Circuit::performDCAnalysis(const std::string& sourceName, double startValue, double endValue, double increment) {
    Component* sweepSource = getComponent(sourceName);

    if (!sweepSource)
        throw std::runtime_error("Source '" + sourceName + "' for DC sweep not found.");
    if (groundNodeIds.empty())
        throw std::runtime_error("No ground node detected.");

    std::cout << "\n--- Performing DC Sweep Analysis on " << sourceName << " ---" << std::endl;
    std::cout << "Start: " << startValue << ", Stop: " << endValue << ", Increment: " << increment << std::endl;

    dcSweepSolutions.clear();
    for (Component* component : components)
        component->reset();

    std::map<int, int> nodeIdToMnaIndex;

    for (double sweepValue = startValue; sweepValue <= endValue; sweepValue += increment) {
        if (auto vs = dynamic_cast<VoltageSource*>(sweepSource))
            vs->setValue(sweepValue);
        else if (auto cs = dynamic_cast<CurrentSource*>(sweepSource))
            cs->setValue(sweepValue);
        else
            throw std::runtime_error("Component '" + sourceName + "' is not a sweepable source.");

        Eigen::VectorXd solution;
        buildMNAMatrix(0.0, 0.0);
        nodeIdToMnaIndex.clear();
        int currentMnaIndex = 0;
        for (int i = 0; i < nextNodeId; ++i) {
            if (idToNodeName.count(i) && !isGround(i)) {
                nodeIdToMnaIndex[i] = currentMnaIndex++;
            }
        }

        if (!hasNonlinearComponents) {
            solution = solveMNASystem();
        }
        else {
            const int MAX_ITERATIONS = 100;
            const double TOLERANCE = 1e-6;
            bool converged = false;

            Eigen::VectorXd lastSolution;

            for (auto* comp : components) {
                if (auto* diode = dynamic_cast<Diode*>(comp)) {
                    diode->reset();
                }
            }

            for (int i = 0; i < MAX_ITERATIONS; ++i) {
                buildMNAMatrix(0.0, 0.0);
                solution = solveMNASystem();
                if (solution.size() == 0) {
                    std::cout << "DC sweep failed to solve at " << sourceName << " = " << sweepValue << std::endl;
                    break;
                }
                if (i > 0 && (solution - lastSolution).norm() < TOLERANCE) {
                    converged = true;
                    break;
                }
                lastSolution = solution;
                updateNonlinearComponentStates(solution, nodeIdToMnaIndex);
            }

            if (!converged)
                std::cout << "Warning: DC analysis did not converge at sweep value " << sweepValue << std::endl;
        }
        if (solution.size() > 0) {
            dcSweepSolutions[sweepValue] = solution;
        }
        dcSweepSolutions[sweepValue] = solution;
    }
    std::cout << "DC Sweep complete. " << dcSweepSolutions.size() << " points calculated." << std::endl;
}

void Circuit::performTransientAnalysis(double stopTime, double startTime, double maxTimeStep) {
    if (maxTimeStep == 0.0)
        maxTimeStep = (stopTime - startTime) / 100;
    std::cout << "\n\t---------- Performing Transient Analysis ----------" << std::endl;
    std::cout << "Time Start: " << startTime << "s, Stop Time: " << stopTime << "s, Maximum Time Step: " << maxTimeStep
        << "s" << std::endl;

    if (groundNodeIds.empty())
        throw std::runtime_error("No ground node detected.");

    for (Component* comp : components)
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

    for (double t = startTime; t <= stopTime; t += maxTimeStep) {
        if (!hasNonlinearComponents) {
            buildMNAMatrix(t, maxTimeStep);
            solution = solveMNASystem();
        }
        else {
            const int MAX_ITERATIONS = 100;
            const double TOLERANCE = 1e-6;
            bool converged = false;
            Eigen::VectorXd lastSolution;

            for (int i = 0; i < MAX_ITERATIONS; ++i) {
                buildMNAMatrix(t, maxTimeStep);
                solution = solveMNASystem();
                if (solution.size() == 0) break;

                if (i > 0 && (solution - lastSolution).norm() < TOLERANCE) {
                    converged = true;
                    break;
                }
                lastSolution = solution;
                updateNonlinearComponentStates(solution, nodeIdToMnaIndex);
            }
            if (!converged)
                std::cout << "Warning: Transient analysis did not converge at t = " << t << "s" << std::endl;
        }
        if (solution.size() == 0)
            throw std::runtime_error("ERROR at t = " + std::to_string(t) + "s: Simulation stopped.");
        updateComponentStates(solution, nodeIdToMnaIndex);
        transientSolutions[t] = solution;
    }
    std::cout << "Transient analysis complete. " << transientSolutions.size() << " time points stored." << std::endl;
    std::cout << "Use .print to view results." << std::endl;
}
// -------------------------------- Analysis Methods --------------------------------


// -------------------------------- Output Results --------------------------------
void Circuit::printTransientResults(const std::vector<std::string>& variablesToPrint) const {
    if (transientSolutions.empty())
        throw std::runtime_error("No analysis results found. Run .TRAN or .DC first.");
    if (groundNodeIds.empty())
        throw std::runtime_error("No ground node detected.");

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
        Component* component_ptr = nullptr;
    };
    std::vector<PrintJob> printJobs;

    for (const auto& var : variablesToPrint) {
        if (var.length() < 4)
            continue;
        std::string type = var.substr(0, 1);
        std::string name = var.substr(2, var.length() - 3);

        if (type == "V") {
            if (!hasNode(name)) throw std::runtime_error("Node " + name + " not found.");
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
                    Component* comp = getComponent(name);
                    if (!comp)
                        throw std::runtime_error("Component " + name + " not found.");
                    if (dynamic_cast<Resistor*>(comp))
                        printJobs.push_back({var, PrintJob::Type::RESISTOR_CURRENT, -1, comp});
                    else if (dynamic_cast<Capacitor*>(comp))
                        printJobs.push_back({var, PrintJob::Type::CAPACITOR_CURRENT, -1, comp});
                    else
                        std::cout << "Warning: Current for component type of '" << name << "' cannot be calculated." <<
                            std::endl;
                }
            }
        }
    }
    if (printJobs.empty())
        throw std::runtime_error("No valid variables to print.");

    std::cout << std::left << std::setw(14) << "Time";
    for (const auto& job : printJobs)
        std::cout << std::setw(14) << job.header;
    std::cout << std::endl;

    auto itPrev = transientSolutions.begin();
    for (auto it = transientSolutions.begin(); it != transientSolutions.end(); ++it) {
        double t = it->first;
        const Eigen::VectorXd& solution = it->second;

        std::cout << std::left << std::fixed << std::setprecision(6) << std::setw(14) << t;

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
            std::cout << std::setw(14) << result;
        }
        std::cout << std::endl;
        itPrev = it;
    }
}

void Circuit::printDcSweepResults(const std::string& sourceName, const std::string& variable) const {
    if (dcSweepSolutions.empty())
        throw std::runtime_error("No DC sweep results found. Run a .DC analysis first via the .print command.");

    char varType = variable.front();
    std::string varName = variable.substr(2, variable.length() - 3);

    std::map<int, int> nodeIdToMnaIndex;
    int currentMnaIndex = 0;
    for (int i = 0; i < nextNodeId; ++i) {
        if (idToNodeName.count(i) && !isGround(i)) {
            nodeIdToMnaIndex[i] = currentMnaIndex++;
        }
    }

    std::cout << "\n---- DC Sweep Results ----" << std::endl;
    std::cout << std::left << std::setw(14) << sourceName;
    std::cout << std::setw(14) << variable << std::endl;
    std::cout << "-----------------------------" << std::endl;

    for (const auto& pair : dcSweepSolutions) {
        double sweepValue = pair.first;
        const Eigen::VectorXd& solution = pair.second;
        double result = 0.0;

        if (varType == 'V') {
            int nodeID = getNodeId(varName);
            if (nodeID == -1) throw std::runtime_error("Node not found.");
            result = isGround(nodeID) ? 0.0 : solution(nodeIdToMnaIndex.at(nodeID));
        }
        else {
            Component* comp = getComponent(varName);
            if (!comp) {
                std::string errorMsg = "Component " + varName + " not found in circuit.";
                throw std::runtime_error(errorMsg);
            }
            if (comp->needsCurrentUnknown()) {
                if (componentCurrentIndices.count(varName))
                    result = solution(componentCurrentIndices.at(varName));
                else {
                    std::cout << "Warning: Could not find current index for '" << varName << "'. Skipping." <<
                        std::endl;
                    continue;
                }
            }
            else {
                if (dynamic_cast<Resistor*>(comp)) {
                    int node1 = comp->node1;
                    int node2 = comp->node2;

                    double v1 = 0.0;
                    if (nodeIdToMnaIndex.count(node1)) {
                        int index1 = nodeIdToMnaIndex.at(node1);
                        v1 = solution(index1);
                    }
                    double v2 = 0.0;
                    if (nodeIdToMnaIndex.count(node2)) {
                        int index2 = nodeIdToMnaIndex.at(node2);
                        v2 = solution(index2);
                    }
                    result = (v1 - v2) / comp->value;
                }
                else if (dynamic_cast<Capacitor*>(comp))
                    result = 0.0;
                else {
                    std::cout << "Warning: Current printing for this component type ('" << varName <<
                        "') is not supported. Skipping." << std::endl;
                    continue;
                }
            }
        }

        std::cout << std::left << std::fixed << std::setprecision(6);
        std::cout << std::setw(14) << sweepValue;
        std::cout << std::setw(14) << result << std::endl;
    }
}
// -------------------------------- Output Results --------------------------------