#ifndef CIRCUIT_H
#define CIRCUIT_H
#include "component.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <map>

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
    // Analysis
    void performDCAnalysis(const std::string& , double , double , double );
    void performTransientAnalysis(double, double, double);
    void printTransientResults(const std::vector<std::string>&) const;
    void printDcSweepResults(const std::string&, const std::string&) const;
private:
    int getNodeId(const std::string&, bool create = true);
    void buildMNAMatrix(double, double);
    Eigen::VectorXd solveMNASystem();
    void updateComponentStates(const Eigen::VectorXd&);
    // circuit data
    std::vector<Component*> components;
    std::map<std::string, int> nodeNameToId;
    std::map<int, std::string> idToNodeName;
    int nextNodeId;
    std::string groundNodeName;
    int groundNodeId;
    // MNA Matrix data
    Eigen::MatrixXd A_mna;
    Eigen::VectorXd b_mna;
    int numCurrentUnknowns;
    std::map<std::string, int> componentCurrentIndices; // component name -> MNA component index
    std::map<double, Eigen::VectorXd> transientSolutions;
    std::map<double, Eigen::VectorXd> dcSweepSolutions;
    // State and file management
    std::string currentFilePath;
    bool hasNonlinearComponents; // Diode
};
#endif // CIRCUIT_H