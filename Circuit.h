#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <vector>
#include <QMouseEvent>
#include <QString>
#include <fstream>
#include <filesystem>
#include <set>
#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QRegularExpression>
#include "component.h"
#include "ComponentFactory.h"
#include "Serialization.h"

struct ComponentGraphicalInfo {
    QPoint startPoint;
    bool isHorizontal;
    std::string name;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(startPoint), CEREAL_NVP(isHorizontal), CEREAL_NVP(name));
    }
};

struct WireInfo {
    QPoint startPoint;
    QPoint endPoint;
    std::string nodeName;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(startPoint), CEREAL_NVP(endPoint), CEREAL_NVP(nodeName));
    }
};

struct LabelInfo {
    QPoint position;
    std::string name;
    std::string connectedNodeName;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(position), CEREAL_NVP(name), CEREAL_NVP(connectedNodeName));
    }
};

struct GroundInfo {
    QPoint position;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(position));
    }
};

struct SubcircuitDefinition {
    std::string name;
    std::vector<std::string> netlist;
    std::string port1NodeName;
    std::string port2NodeName;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(name), CEREAL_NVP(netlist), CEREAL_NVP(port1NodeName), CEREAL_NVP(port2NodeName));
    }
};

double parseSpiceValue(const std::string& valueStr);

class Circuit {
public:
    Circuit();
    ~Circuit();

    std::vector<std::string> circuitNetList;
    std::vector<std::string> allFiles;

    // File Operations
    // void saveProjectToFile(const QString& filePath) const;
    // void loadProjectFromFile(const QString& filePath);
    void newProject(const std::string& projectName);
    void saveProject() const;
    void loadProject(const std::string& projectName);
    QString getProjectDirectory() const;
    const std::vector<ComponentGraphicalInfo>& getComponentGraphics() const;
    const std::vector<WireInfo>& getWires() const;
    const std::vector<LabelInfo>& getLabels() const;
    const std::vector<GroundInfo>& getGrounds() const;

    // Component and Node Management
    void addComponent(const std::string&, const std::string&, const std::string&, const std::string&,
    const QPoint& startPoint, bool isHorizontal, double value, const std::vector<double>&, const std::vector<std::string>&, bool);
    void addComponent(const std::string& typeStr, const std::string&, const std::string&, const std::string&, double, const std::vector<double>&, const std::vector<std::string>&, bool);
    void addGround(const std::string&, const QPoint&);
    void addWire(const QPoint& start, const QPoint& end, const std::string& nodeName);
    void deleteComponent(const std::string&, char);
    void deleteGround(const std::string&);
    void listNodes() const;
    void listComponents(char typeFilter = '\0') const;
    void renameNode(const std::string&, const std::string&);
    bool hasNode(const std::string&) const;
    void clearSchematic();
    std::shared_ptr<Component> getComponent(const std::string& name) const;
    int getNodeId(const std::string&, bool create = true);
    int getNodeId(const std::string&) const;
    QString getCurrentProjectName() const;
    // const std::map<int, std::string>& getIdToNodeName() const;
    void connectNodes(const std::string&, const std::string&);
    void createSubcircuitDefinition(const std::string&, const std::string&, const std::string&);
    void addLabel(const QPoint&, const std::string&, const std::string&);
    void processLabelConnections();
    // const std::vector<std::shared_ptr<Component>>& getComponentsVector() const { return components; }

    // Analysis
    void runTransientAnalysis(double startTime, double stopTime, double stepTime);
    std::map<std::string, std::map<double, double>> getTransientResults(const std::vector<std::string>&) const;
    void runACAnalysis(double startOmega, double stopOmega, int numPoints);
    std::map<std::string, std::map<double, double>> getACSweepResults(const std::vector<std::string>&) const;

    std::map<std::string, SubcircuitDefinition> subcircuitDefinitions;
private:
    void buildMNAMatrix(double, double);
    void buildMNAMatrix_AC(double omega);
    Eigen::VectorXd solveMNASystem();
    void updateComponentStates(const Eigen::VectorXd&, const std::map<int, int>&);
    void updateNonlinearComponentStates(const Eigen::VectorXd&, const std::map<int, int>&);
    void mergeNodes(int sourceNodeI, int destNodeId);
    bool isGround(int nodeId) const;
    void makeComponentFromLine(const std::string& netListLine);
    std::vector<std::string> generateNetlistFromComponents() const;

    // circuit data
    std::vector<std::shared_ptr<Component>> components;
    std::map<std::string, int> nodeNameToId;
    std::map<int, std::string> idToNodeName;
    int nextNodeId;
    std::set<int> groundNodeIds;

    // Graphical data
    std::vector<ComponentGraphicalInfo> componentGraphics;
    std::vector<WireInfo> wires;
    std::vector<GroundInfo> grounds;
    std::vector<LabelInfo> labels;
    std::map<std::string, std::set<int>> labelToNodes;

    // MNA Matrix data
    Eigen::MatrixXd A_mna;
    Eigen::VectorXd b_mna;
    int numCurrentUnknowns;
    std::map<std::string, int> componentCurrentIndices; // component name -> MNA component index
    std::map<double, Eigen::VectorXd> transientSolutions;
    // std::map<double, Eigen::VectorXd> dcSweepSolutions;
    std::map<double, Eigen::VectorXd> acSweepSolutions;
    bool hasNonlinearComponents;

    // State and file management
    QString currentProjectName;
    QString projectDirectoryPath;
};


// QDataStream& operator<<(QDataStream& out, const ComponentGraphicalInfo& info);
// QDataStream& operator>>(QDataStream& in, ComponentGraphicalInfo& info);
//
// template<typename K, typename V>
// QDataStream& operator<<(QDataStream& out, const std::map<K, V>& map);
// template<typename K, typename V>
// QDataStream& operator>>(QDataStream& in, std::map<K, V>& map);
//
// template<typename T>
// QDataStream& operator<<(QDataStream& out, const std::set<T>& set);
// template<typename T>
// QDataStream& operator>>(QDataStream& in, std::set<T>& set);

#endif // CIRCUIT_H
