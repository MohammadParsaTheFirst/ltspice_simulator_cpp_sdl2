#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <vector>
#include <fstream>
#include <filesystem>
#include <set>
#include "component.h"
#include "ComponentFactory.h"

double parseSpiceValue(const std::string& valueStr);

class Circuit {
public:
    Circuit();
    ~Circuit();

    std::vector<std::string> circuitNetList;
    std::vector<std::string> allFiles;

    // File Operations
    void newCircuit(const std::string&);
    bool saveLineToFile(const std::string&) const;
    bool loadCircuitFromFile();
    void showExistingFiles();
    void saveCircuitToFile();
    std::string getPathRightNow() { return currentFilePath; }

    // Component and Node Management
    void addComponent(const std::string&, const std::string&, const std::string&, const std::string&, double, const std::vector<double>&, const std::vector<std::string>&, bool);
    void addGround(const std::string& );
    void deleteComponent(const std::string&, char);
    void deleteGround(const std::string&);
    void listNodes() const;
    void listComponents(char typeFilter = '\0') const;
    void renameNode(const std::string&, const std::string&);
    bool hasNode(const std::string&) const;
    void clearSchematic();
    Component* getComponent(const std::string& name) const;
    int getNodeId(const std::string&, bool create = true);
    int getNodeId(const std::string&) const;
    void connectNodes(const std::string&, const std::string&);

    // Analysis
    void performDCAnalysis(const std::string& , double , double , double );
    void performTransientAnalysis(double, double, double);
    void printTransientResults(const std::vector<std::string>&) const;
    void printDcSweepResults(const std::string&, const std::string&) const;
    void addLabel(const std::string&, const std::string&);

    std::pair<std::string, std::vector<double>> getTransientResults(const std::string& parameter);
    //std::pair<std::string, std::vector<double>>
    void runTransientAnalysis(double startTime, double stopTime, double stepTime);


private:
    void buildMNAMatrix(double, double);
    Eigen::VectorXd solveMNASystem();
    void updateComponentStates(const Eigen::VectorXd&, const std::map<int, int>&);
    void updateNonlinearComponentStates(const Eigen::VectorXd&, const std::map<int, int>&);
    void mergeNodes(int sourceNodeI, int destNodeId);
    bool isGround(int nodeId) const;

    // circuit datas
    std::vector<Component*> components;
    std::map<std::string, int> nodeNameToId;
    std::map<int, std::string> idToNodeName;
    int nextNodeId;
    std::set<int> groundNodeIds;

    // MNA Matrix data
    Eigen::MatrixXd A_mna;
    Eigen::VectorXd b_mna;
    int numCurrentUnknowns;
    std::map<std::string, int> componentCurrentIndices; // component name -> MNA component index
    std::map<double, Eigen::VectorXd> transientSolutions;//std::map<double, Eigen::VectorXd> transientSolutions;
    std::map<double, Eigen::VectorXd> dcSweepSolutions;

    // State and file management
    std::string currentFilePath;
    bool hasNonlinearComponents; // Diode

    std::map<std::string, std::set<int>> labelToNodes;
};

#endif // CIRCUIT_H