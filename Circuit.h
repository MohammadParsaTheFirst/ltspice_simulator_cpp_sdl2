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

struct ComponentGraphicalInfo {
    QPoint startPoint;
    bool isHorizontal;
    std::string name;
};

struct WireInfo {
    QPoint startPoint;
    QPoint endPoint;
    std::string nodeName;
};

struct LabelInfo {
    QPoint position;
    std::string name;
    std::string connectedNodeName;
};

struct GroundInfo {
    QPoint position;
};

struct SubcircuitDefinition {
    std::string name;
    std::vector<std::string> netlist;
    std::string port1NodeName;
    std::string port2NodeName;
};

double parseSpiceValue(const std::string& valueStr);

class Circuit {
public:
    Circuit();
    ~Circuit();

    std::vector<std::string> circuitNetList;
    std::vector<std::string> allFiles;

    // File Operations
    void newProject(const std::string& projectName);
    void saveProject() const;
    void loadProject(const std::string& projectName);
    QString getProjectDirectory() const;
    // void saveSubcircuit(const SubcircuitDefinition& subDef) const;
    // void loadSubcircuits();
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
    void connectNodes(const std::string&, const std::string&);
    void createSubcircuitDefinition(const std::string&, const std::string&, const std::string&);

    // Analysis
    void performDCAnalysis(const std::string&, double, double, double);
    // void performTransientAnalysis(double, double, double);
    std::vector<double> getTransientResults(const std::vector<std::string>&) const;
    void printDcSweepResults(const std::string&, const std::string&) const;
    void addLabel(const std::string&, const std::string&);
    // std::pair<std::string, std::vector<double>> getTransientResults(const std::string& parameter);
    void runTransientAnalysis(double startTime, double stopTime, double stepTime);

    std::map<std::string, SubcircuitDefinition> subcircuitDefinitions;
private:
    void buildMNAMatrix(double, double);
    Eigen::VectorXd solveMNASystem();
    void updateComponentStates(const Eigen::VectorXd&, const std::map<int, int>&);
    void updateNonlinearComponentStates(const Eigen::VectorXd&, const std::map<int, int>&);
    void mergeNodes(int sourceNodeI, int destNodeId);
    bool isGround(int nodeId) const;
    void makeComponentFromLine(const std::string& netListLine);

    // circuit datas
    std::vector<std::shared_ptr<Component>> components;
    std::vector<ComponentGraphicalInfo> componentGraphics;
    std::map<std::string, int> nodeNameToId;
    std::map<int, std::string> idToNodeName;
    int nextNodeId;
    std::set<int> groundNodeIds;
    std::vector<WireInfo> wires;
    std::vector<GroundInfo> grounds;
    std::vector<LabelInfo> labels;

    // MNA Matrix data
    Eigen::MatrixXd A_mna;
    Eigen::VectorXd b_mna;
    int numCurrentUnknowns;
    std::map<std::string, int> componentCurrentIndices; // component name -> MNA component index
    std::map<double, Eigen::VectorXd> transientSolutions;
    std::map<double, Eigen::VectorXd> dcSweepSolutions;

    // State and file management
    QString currentProjectName;
    QString projectDirectoryPath;
    bool hasNonlinearComponents; // Diode

    std::map<std::string, std::set<int>> labelToNodes;
};

#endif // CIRCUIT_H
