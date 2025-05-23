#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "component.h"
#include "Eigen/Dense"
#include <vector>
#include <map>
#include <string>

class Circuit {
public:
    enum ComponentType { RESISTOR, CAPACITOR, INDUCTOR, VOLTAGE_SOURCE, CURRENT_SOURCE, DIODE };

private:
    std::vector<Component*> components;
    std::map<std::string, int> nodeNameToId;
    std::map<int, std::string> idToNodeName;
    int nextNodeId;
    int numVoltageSources;
    int numNodes;

    Eigen::MatrixXd A_mna;
    Eigen::VectorXd b_mna;

public:
    Circuit();
    ~Circuit();

    int getNodeId(const std::string& nodeName);
    void updateNumNodes();
    void addComponent(const std::string& typeStr, const std::string& name,
                      const std::string& node1Str, const std::string& node2Str,
                      double value, const std::vector<double>& params = {});

    void listNodes() const;
    void listComponents(const std::string& typeFilter = "") const;
    void renameNode(const std::string& oldName, const std::string& newName);

    void buildMNAMatrix(double time = 0.0, double h = 0.0);
    void solveMNASystem();
    void printResults(const Eigen::VectorXd& solution) const;

    void performDCAnalysis();
    void performTransientAnalysis(double timeStep, double stopTime, double startTime = 0.0);
    void handlePrintCommand(const std::string& commandLine);

    void initializeMNAMatrix();

    bool hasNode(const std::string& nodeName) const {
        return nodeNameToId.find(nodeName) != nodeNameToId.end();
    }
};

#endif // CIRCUIT_H