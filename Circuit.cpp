#include <iomanip>
#include <utility>
#include <cctype>
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
QString Circuit::getProjectDirectory() const {
    return QCoreApplication::applicationDirPath() + QDir::separator() + "Schematics";
}

void Circuit::newProject(const std::string& projectName) {
    clearSchematic();
    currentProjectName = QString::fromStdString(projectName);
    projectDirectoryPath = getProjectDirectory() + QDir::separator() + currentProjectName;

    QDir dir(projectDirectoryPath);
    if (!dir.exists())
        dir.mkpath(".");
}

void save_qpoint(std::ofstream& file, const QPoint& p) {
    int x = p.x(), y = p.y();
    file.write(reinterpret_cast<const char*>(&x), sizeof(x));
    file.write(reinterpret_cast<const char*>(&y), sizeof(y));
}

void save_string(std::ofstream& file, const std::string& s) {
    size_t len = s.length();
    file.write(reinterpret_cast<const char*>(&len), sizeof(len));
    file.write(s.c_str(), len);
}

void Circuit::saveProject() const {
    if (currentProjectName.isEmpty()) {
        std::cerr << "Error: No project is open to save." << std::endl;
        return;
    }

    QString projectFilePath = projectDirectoryPath + QDir::separator() + currentProjectName + ".bin";
    QFile projectFile(projectFilePath);
    if (!projectFile.open(QIODevice::WriteOnly)) {
        std::cerr << "Error: Failed to open " << projectFilePath.toStdString() << " for writing." << std::endl;
        return;
    }
    QDataStream out(&projectFile);
    out.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    // Save components section
    size_t num_components = components.size();
    out.writeRawData(reinterpret_cast<const char*>(&num_components), sizeof(num_components));
    for (const auto& comp : components) {
        comp->save_binary(out);
    }

    // Save graphics section
    size_t num_graphics = componentGraphics.size();
    out.writeRawData(reinterpret_cast<const char*>(&num_graphics), sizeof(num_graphics));
    for (const auto& graphic : componentGraphics) {
        int x = graphic.startPoint.x(), y = graphic.startPoint.y();
        out.writeRawData(reinterpret_cast<const char*>(&x), sizeof(x));
        out.writeRawData(reinterpret_cast<const char*>(&y), sizeof(y));
        out.writeRawData(reinterpret_cast<const char*>(&graphic.isHorizontal), sizeof(graphic.isHorizontal));
        QByteArray nameBytes = QByteArray::fromStdString(graphic.name);
        size_t len = nameBytes.size();
        out.writeRawData(reinterpret_cast<const char*>(&len), sizeof(len));
        out.writeRawData(nameBytes.constData(), len);
    }

    // Save wires section
    size_t num_wires = wires.size();
    out.writeRawData(reinterpret_cast<const char*>(&num_wires), sizeof(num_wires));
    for (const auto& wire : wires) {
        int sx = wire.startPoint.x(), sy = wire.startPoint.y();
        int ex = wire.endPoint.x(), ey = wire.endPoint.y();
        out.writeRawData(reinterpret_cast<const char*>(&sx), sizeof(sx));
        out.writeRawData(reinterpret_cast<const char*>(&sy), sizeof(sy));
        out.writeRawData(reinterpret_cast<const char*>(&ex), sizeof(ex));
        out.writeRawData(reinterpret_cast<const char*>(&ey), sizeof(ey));
        QByteArray nodeBytes = QByteArray::fromStdString(wire.nodeName);
        size_t len = nodeBytes.size();
        out.writeRawData(reinterpret_cast<const char*>(&len), sizeof(len));
        out.writeRawData(nodeBytes.constData(), len);
    }

    // Save labels section
    size_t num_labels = labels.size();
    out.writeRawData(reinterpret_cast<const char*>(&num_labels), sizeof(num_labels));
    for (const auto& label : labels) {
        int px = label.position.x(), py = label.position.y();
        out.writeRawData(reinterpret_cast<const char*>(&px), sizeof(px));
        out.writeRawData(reinterpret_cast<const char*>(&py), sizeof(py));
        QByteArray nameBytes = QByteArray::fromStdString(label.name);
        size_t len1 = nameBytes.size();
        out.writeRawData(reinterpret_cast<const char*>(&len1), sizeof(len1));
        out.writeRawData(nameBytes.constData(), len1);
        QByteArray connBytes = QByteArray::fromStdString(label.connectedNodeName);
        size_t len2 = connBytes.size();
        out.writeRawData(reinterpret_cast<const char*>(&len2), sizeof(len2));
        out.writeRawData(connBytes.constData(), len2);
    }

    projectFile.close();
    std::cout << "Project '" << currentProjectName.toStdString() << "' saved successfully." << std::endl;
}

QPoint load_qpoint(std::ifstream& file) {
    int x, y;
    file.read(reinterpret_cast<char*>(&x), sizeof(x));
    file.read(reinterpret_cast<char*>(&y), sizeof(y));
    return QPoint(x, y);
}

std::string load_string(std::ifstream& file) {
    size_t len;
    file.read(reinterpret_cast<char*>(&len), sizeof(len));
    if (len > 1024) {
        throw std::runtime_error("Invalid string length in save file.");
    }
    char* buf = new char[len + 1];
    file.read(buf, len);
    buf[len] = '\0';
    std::string s(buf);
    delete[] buf;
    return s;
}

void Circuit::loadProject(const std::string& projectName) {
    newProject(projectName);

    QString projectFilePath = projectDirectoryPath + QDir::separator() + currentProjectName + ".bin";
    QFile projectFile(projectFilePath);
    if (!projectFile.open(QIODevice::ReadOnly)) {
        std::cout << "Starting new empty project: " << projectName << std::endl;
        return;
    }
    QDataStream in(&projectFile);
    in.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    // Load components section
    size_t num_components;
    in.readRawData(reinterpret_cast<char*>(&num_components), sizeof(num_components));
    components.clear();
    for (size_t i = 0; i < num_components; ++i) {
        Component::Type type;
        in.readRawData(reinterpret_cast<char*>(&type), sizeof(type));

        std::shared_ptr<Component> newComp = nullptr;
        switch (type) {
            case Component::Type::RESISTOR: newComp = std::make_shared<Resistor>("", 0, 0, 0); break;
            case Component::Type::CAPACITOR: newComp = std::make_shared<Capacitor>("", 0, 0, 0); break;
            case Component::Type::INDUCTOR: newComp = std::make_shared<Inductor>("", 0, 0, 0); break;
            case Component::Type::DIODE: newComp = std::make_shared<Diode>("", 0, 0); break;
            case Component::Type::VOLTAGE_SOURCE: newComp = std::make_shared<VoltageSource>("", 0, 0, VoltageSource::SourceType::DC, 0, 0, 0); break;
            case Component::Type::CURRENT_SOURCE: newComp = std::make_shared<CurrentSource>("", 0, 0, CurrentSource::SourceType::DC, 0, 0, 0); break;
            case Component::Type::VCVS: newComp = std::make_shared<VCVS>("", 0, 0, 0, 0, 0); break;
            case Component::Type::VCCS: newComp = std::make_shared<VCCS>("", 0, 0, 0, 0, 0); break;
            case Component::Type::CCVS: newComp = std::make_shared<CCVS>("", 0, 0, "", 0); break;
            case Component::Type::CCCS: newComp = std::make_shared<CCCS>("", 0, 0, "", 0); break;
        }

        if (newComp) {
            newComp->load_binary(in);
            components.push_back(newComp);
        }
    }

    // Load graphics section
    size_t num_graphics;
    in.readRawData(reinterpret_cast<char*>(&num_graphics), sizeof(num_graphics));
    componentGraphics.clear();
    for(size_t i = 0; i < num_graphics; ++i) {
        ComponentGraphicalInfo graphic;
        int x, y;
        in.readRawData(reinterpret_cast<char*>(&x), sizeof(x));
        in.readRawData(reinterpret_cast<char*>(&y), sizeof(y));
        graphic.startPoint = QPoint(x, y);
        in.readRawData(reinterpret_cast<char*>(&graphic.isHorizontal), sizeof(graphic.isHorizontal));
        size_t len;
        in.readRawData(reinterpret_cast<char*>(&len), sizeof(len));
        if (len > 1024) throw std::runtime_error("Invalid string length in save file.");
        char* buf = new char[len];
        in.readRawData(buf, len);
        graphic.name = std::string(buf, len);
        delete[] buf;
        componentGraphics.push_back(graphic);
    }

    // Load wires section
    size_t num_wires;
    in.readRawData(reinterpret_cast<char*>(&num_wires), sizeof(num_wires));
    wires.clear();
    for(size_t i = 0; i < num_wires; ++i) {
        WireInfo wire;
        int sx, sy, ex, ey;
        in.readRawData(reinterpret_cast<char*>(&sx), sizeof(sx));
        in.readRawData(reinterpret_cast<char*>(&sy), sizeof(sy));
        in.readRawData(reinterpret_cast<char*>(&ex), sizeof(ex));
        in.readRawData(reinterpret_cast<char*>(&ey), sizeof(ey));
        wire.startPoint = QPoint(sx, sy);
        wire.endPoint = QPoint(ex, ey);
        size_t len;
        in.readRawData(reinterpret_cast<char*>(&len), sizeof(len));
        if (len > 1024) throw std::runtime_error("Invalid string length in save file.");
        char* buf = new char[len];
        in.readRawData(buf, len);
        wire.nodeName = std::string(buf, len);
        delete[] buf;
        wires.push_back(wire);
    }

    // Load labels section
    size_t num_labels;
    in.readRawData(reinterpret_cast<char*>(&num_labels), sizeof(num_labels));
    labels.clear();
    for(size_t i = 0; i < num_labels; ++i) {
        LabelInfo label;
        int px, py;
        in.readRawData(reinterpret_cast<char*>(&px), sizeof(px));
        in.readRawData(reinterpret_cast<char*>(&py), sizeof(py));
        label.position = QPoint(px, py);
        size_t len1;
        in.readRawData(reinterpret_cast<char*>(&len1), sizeof(len1));
        if (len1 > 1024) throw std::runtime_error("Invalid string length in save file.");
        char* buf1 = new char[len1];
        in.readRawData(buf1, len1);
        label.name = std::string(buf1, len1);
        delete[] buf1;
        size_t len2;
        in.readRawData(reinterpret_cast<char*>(&len2), sizeof(len2));
        if (len2 > 1024) throw std::runtime_error("Invalid string length in save file.");
        char* buf2 = new char[len2];
        in.readRawData(buf2, len2);
        label.connectedNodeName = std::string(buf2, len2);
        delete[] buf2;
        labels.push_back(label);
    }

    projectFile.close();
    std::cout << "Project '" << projectName << "' loaded successfully." << std::endl;
}


QString Circuit::getCurrentProjectName() const {
    return currentProjectName;
}

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

const std::map<int, std::string>& Circuit::getIdToNodeName() const {
    return idToNodeName;
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

void Circuit::saveProjectToFile(const QString& filePath) const {
    if (currentProjectName.isEmpty()) {
        std::cerr << "Error: No project is open to save." << std::endl;
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        std::cerr << "Error: Could not open file for writing: " << filePath.toStdString() << std::endl;
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_DefaultCompiledVersion); //Qt_6_0

    // Save basic project info
    out << currentProjectName;
    out << nextNodeId;
    out << numCurrentUnknowns;
    out << hasNonlinearComponents;

    // Save node mappings
    out << static_cast<qint32>(nodeNameToId.size());
    for (const auto& pair : nodeNameToId) {
        out << QString::fromStdString(pair.first) << pair.second;
    }

    out << static_cast<qint32>(idToNodeName.size());
    for (const auto& pair : idToNodeName) {
        out << pair.first << QString::fromStdString(pair.second);
    }

    // Save ground nodes
    out << static_cast<qint32>(groundNodeIds.size());
    for (int nodeId : groundNodeIds) {
        out << nodeId;
    }

    // Save components
    out << static_cast<qint32>(components.size());
    for (const auto& comp : components) {
        comp->save_binary(out);
    }

    // Save graphical data
    out << static_cast<qint32>(componentGraphics.size());
    for (const auto& graphic : componentGraphics) {
        out << graphic.startPoint << graphic.isHorizontal << QString::fromStdString(graphic.name);
    }

    out << static_cast<qint32>(wires.size());
    for (const auto& wire : wires) {
        out << wire.startPoint << wire.endPoint << QString::fromStdString(wire.nodeName);
    }

    out << static_cast<qint32>(labels.size());
    for (const auto& label : labels) {
        out << label.position << QString::fromStdString(label.name) << QString::fromStdString(label.connectedNodeName);
    }

    out << static_cast<qint32>(grounds.size());
    for (const auto& ground : grounds) {
        out << ground.position;
    }

    file.close();
    std::cout << "Project '" << currentProjectName.toStdString() << "' saved successfully to: " << filePath.toStdString() << std::endl;
}

void Circuit::loadProjectFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cerr << "Error: Could not open file for reading: " << filePath.toStdString() << std::endl;
        return;
    }

    std::cout << "Starting to read the file ..." << std::endl;
    clearSchematic();
    std::cout << "Cleared Schematic ..." <<std::endl;

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_DefaultCompiledVersion); //Qt_6_0
    std::cout << "Qt version: " << in.version() << std::endl;


    // Add status checking throughout
    if (in.status() != QDataStream::Ok) {
        std::cerr << "Error: QDataStream status error before reading" << std::endl;
        file.close();
        return;
    }


    // Load basic project info
    in >> currentProjectName;
    in >> nextNodeId;
    in >> numCurrentUnknowns;
    in >> hasNonlinearComponents;

    // CHECK May THIS WORK!
    if (in.status() != QDataStream::Ok) {
        std::cerr << "Error: Failed to read project info from: " << filePath.toStdString() << std::endl;
        file.close();
        return;
    }

    // Load node mappings
    std::cout << "Loading node mappings ..." << std::endl;
    qint32 nodeNameToIdSize;
    in >> nodeNameToIdSize;
    nodeNameToId.clear();
    for (int i = 0; i < nodeNameToIdSize; ++i) {
        QString nodeName;
        int nodeId;
        in >> nodeName >> nodeId;
        nodeNameToId[nodeName.toStdString()] = nodeId;
    }

    qint32 idToNodeNameSize;
    in >> idToNodeNameSize;
    idToNodeName.clear();
    for (int i = 0; i < idToNodeNameSize; ++i) {
        int nodeId;
        QString nodeName;
        in >> nodeId >> nodeName;
        idToNodeName[nodeId] = nodeName.toStdString();
    }
    std::cout << "Loaded node mappings" << std::endl;

    // Load ground nodes
    std::cout << "Loading ground mappings ..." << std::endl;
    qint32 groundNodeIdsSize;
    in >> groundNodeIdsSize;
    groundNodeIds.clear();
    for (int i = 0; i < groundNodeIdsSize; ++i) {
        int nodeId;
        in >> nodeId;
        groundNodeIds.insert(nodeId);
    }
    std::cout << "Loaded ground mappings" << std::endl;

    // Load components
    std::cout << "Loading components ..." << std::endl;
    qint32 componentsSize;
    in >> componentsSize;
    components.clear();
    for (int i = 0; i < componentsSize; ++i) {
        Component::Type type;
        in.readRawData(reinterpret_cast<char*>(&type), sizeof(type));

        std::shared_ptr<Component> newComp = nullptr;
        switch (type) {
            case Component::Type::RESISTOR: newComp = std::make_shared<Resistor>("", 0, 0, 0); break;
            case Component::Type::CAPACITOR: newComp = std::make_shared<Capacitor>("", 0, 0, 0); break;
            case Component::Type::INDUCTOR: newComp = std::make_shared<Inductor>("", 0, 0, 0); break;
            case Component::Type::DIODE: newComp = std::make_shared<Diode>("", 0, 0); break;
            case Component::Type::VOLTAGE_SOURCE: newComp = std::make_shared<VoltageSource>("", 0, 0, VoltageSource::SourceType::DC, 0, 0, 0); break;
            case Component::Type::CURRENT_SOURCE: newComp = std::make_shared<CurrentSource>("", 0, 0, CurrentSource::SourceType::DC, 0, 0, 0); break;
            case Component::Type::VCVS: newComp = std::make_shared<VCVS>("", 0, 0, 0, 0, 0); break;
            case Component::Type::VCCS: newComp = std::make_shared<VCCS>("", 0, 0, 0, 0, 0); break;
            case Component::Type::CCVS: newComp = std::make_shared<CCVS>("", 0, 0, "", 0); break;
            case Component::Type::CCCS: newComp = std::make_shared<CCCS>("", 0, 0, "", 0); break;
            case Component::Type::AC_VOLTAGE_SOURCE: newComp = std::make_shared<ACVoltageSource>("", 0, 0); break;
        }

        if (newComp) {
            newComp->load_binary(in);
            components.push_back(newComp);
        }
    }
    std::cout << "Loaded components" << std::endl;


    // // After components loop
    // nodeNameToId.clear();
    // idToNodeName.clear();
    // nextNodeId = 0;
    // for (const auto& comp : components) {
    //     if (comp->node1 >= 0) {
    //         QString nodeName = QString("node%1").arg(comp->node1);
    //         nodeNameToId[nodeName.toStdString()] = comp->node1;
    //         idToNodeName[comp->node1] = nodeName.toStdString();
    //         nextNodeId = std::max(nextNodeId, comp->node1 + 1);
    //     }
    //     if (comp->node2 >= 0) {
    //         QString nodeName = QString("node%1").arg(comp->node2);
    //         nodeNameToId[nodeName.toStdString()] = comp->node2;
    //         idToNodeName[comp->node2] = nodeName.toStdString();
    //         nextNodeId = std::max(nextNodeId, comp->node2 + 1);
    //     }
    // }

    // Load graphical data
    qint32 graphicsSize;
    in >> graphicsSize;
    componentGraphics.clear();
    for (int i = 0; i < graphicsSize; ++i) {
        ComponentGraphicalInfo graphic;
        QString name;
        in >> graphic.startPoint >> graphic.isHorizontal >> name;
        graphic.name = name.toStdString();
        componentGraphics.push_back(graphic);
    }

    qint32 wiresSize;
    in >> wiresSize;
    wires.clear();
    for (int i = 0; i < wiresSize; ++i) {
        WireInfo wire;
        QString nodeName;
        in >> wire.startPoint >> wire.endPoint >> nodeName;
        wire.nodeName = nodeName.toStdString();
        wires.push_back(wire);
    }

    qint32 labelsSize;
    in >> labelsSize;
    labels.clear();
    for (int i = 0; i < labelsSize; ++i) {
        LabelInfo label;
        QString name, connectedNode;
        in >> label.position >> name >> connectedNode;
        label.name = name.toStdString();
        label.connectedNodeName = connectedNode.toStdString();
        labels.push_back(label);
    }

    qint32 groundsSize;
    in >> groundsSize;
    grounds.clear();
    for (int i = 0; i < groundsSize; ++i) {
        GroundInfo ground;
        in >> ground.position;
        grounds.push_back(ground);
    }
    std::cout << "Loaded wires, labels, grounds ..." << std::endl;

    file.close();
    std::cout << "Project loaded successfully from: " << filePath.toStdString() << std::endl;
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
        if (solution.size() == 0) {
            std::cout << "ERROR at t = " << t << "s: Simulation stopped." << std::endl;
            return;
        }
        updateComponentStates(solution, nodeIdToMnaIndex);
        transientSolutions[t] = solution;
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