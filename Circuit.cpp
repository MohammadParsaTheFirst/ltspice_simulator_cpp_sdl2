#include "circuit.h"
#include "component.h"
#include <iostream>
#include <sstream>
#include <limits>

Circuit::Circuit() : nextNodeId(0), numVoltageSources(0), numNodes(0) {
    nodeNameToId["GND"] = 0;
    idToNodeName[0] = "GND";
    nextNodeId = 1;
}

Circuit::~Circuit() {
    for (Component* comp : components) {
        delete comp;
    }
}

int Circuit::getNodeId(const std::string& nodeName) {
    if (nodeNameToId.find(nodeName) == nodeNameToId.end()) {
        nodeNameToId[nodeName] = nextNodeId;
        idToNodeName[nextNodeId] = nodeName;
        nextNodeId++;
    }
    return nodeNameToId[nodeName];
}

void Circuit::updateNumNodes() {
    numNodes = nextNodeId;
}

void Circuit::initializeMNAMatrix() {
    updateNumNodes();// Ensure numNodes is up-to-date
    // MNA matrix size: (numNodes-1 + numVoltageSources) x (numNodes-1 + numVoltageSources)
    // If node 0 (ground) is implicitly handled (its equations removed), then it's (numNodes-1)
    // Here, we'll assume numNodes includes ground, and the MNA matrix will be of size `numNodes + numVoltageSources`
    // and then we'll adjust for ground if needed (e.g., by making A(0,x) and A(x,0) zero).
    // Or, more commonly, the ground node is explicitly handled as node 0 but its row/column is excluded
    // from the final matrix, meaning the matrix size is `(max_node_id) + numVoltageSources`.

    // Let's go with the convention that the MNA matrix will have 'max_node_id' rows/cols for voltages
    // and 'numVoltageSources' for currents.
    // Total unknowns = (max_node_ID, assuming 0-indexed nodes, so numNodes actually holds max_node_id + 1)
    // + numVoltageSources.
    // However, ground node (node 0) is usually excluded from the MNA matrix equations,
    // so we only consider nodes 1 to (max_node_id).
    // Let's assume we have `numNodes` total *unique* nodes (including GND).
    // The number of *unknown* node voltages is `numNodes - 1` (excluding ground).
    // Total matrix size will be `(numNodes - 1) + numVoltageSources`.
    int total_unknowns = (nextNodeId - 1) + numVoltageSources;
    if (total_unknowns <= 0) {
        A_mna.resize(0, 0);
        b_mna.resize(0);
        return;
    }

    A_mna.setZero(total_unknowns, total_unknowns);
    b_mna.setZero(total_unknowns);
}

void Circuit::buildMNAMatrix(double time, double h) {
     // 1. Determine MNA matrix size
     numVoltageSources = 0;
     for (const auto& comp : components) {
         if (comp->type == Component::Type::VOLTAGE_SOURCE) {
             numVoltageSources++;
         }
     }

     // Initialize the combined A_mna and b_mna matrices with zeros
     // The node indices in MNA will be 0, 1, ..., numNodes-1. Node 0 is ground.
     // The MNA matrix equations are for nodes 1 to (numNodes-1) and then for voltage source currents.
     // So, matrix size = (max_node_id) + numVoltageSources, where max_node_id is highest node ID.
     // The 0th row/col for node 0 (ground) is typically omitted.

     int total_nodes_for_mna = nextNodeId; // This includes node 0 (GND)
     int matrix_size = (total_nodes_for_mna - 1) + numVoltageSources; // Nodes 1 to (max_node_id) + V_source currents

     if (matrix_size <= 0) {
         std::cerr << "Error: Cannot build MNA matrix, no active nodes or sources." << std::endl;
         A_mna.resize(0, 0);
         b_mna.resize(0);
         return;
     }

     A_mna.setZero(matrix_size, matrix_size);
     b_mna.setZero(matrix_size);

     // 2. Iterate through components and "stamp" their contributions
     int voltageSourceIdxCounter = 0; // To assign unique indices to voltage sources in the MNA matrix
     for (Component* comp : components) {
         int mna_idx = -1; // Default, for components that don't add new unknowns
         if (comp->type == Component::Type::VOLTAGE_SOURCE) {
             // Voltage sources add a new unknown (current through them)
             // This unknown will be at index (numNodes - 1) + voltageSourceIdxCounter
             mna_idx = (total_nodes_for_mna - 1) + voltageSourceIdxCounter;
             voltageSourceIdxCounter++;
         } else if (comp->type == Component::Type::INDUCTOR) {
             // In transient analysis, inductors can also add unknowns (their current),
             // or be handled by the Backward Euler approximation that converts them to equivalent R and I source.
             // For now, let's assume Inductor's stampMNA handles its own new unknown index if it needs one.
             // If the Inductor needs to add its current as an unknown, its 'stampMNA'
             // would need to manage its own 'idx' similar to VoltageSource.
             // For simplicity here, let's assume the Inductor's stampMNA is called without a new 'idx'
             // and it uses its R/I equivalence. If it needs a new unknown, you'd track its index here.
         }

         // For stamping, the component's nodes need to be adjusted if node 0 (ground) is implicit.
         // If a node is 0 (ground), then the corresponding row/column in A and b is ignored.
         // E.g., if node1 = 0, then A(0, node2) and A(node2, 0) are not touched.
         // Also, A(node1, node1) will be excluded from the stamping.

         // Call the component's stampMNA method
         // Node 0 (ground) will map to -1 for Eigen matrix indexing, as its row/col is removed.
         // This means, actual node IDs should be shifted. Node 1 becomes index 0, Node 2 becomes index 1 etc.
         // Or, more simply, implement stampMNA such that it checks for node 0 and doesn't stamp those terms.
         // The current stampMNA implementations already do this (if node1 != 0, if node2 != 0).

         // Pass the actual current time step for components that are time-dependent
         comp->stampMNA(A_mna, b_mna, mna_idx, h);
     }

     // Error check for disconnected circuit (could be done here or in analyze methods)
     // This is more complex and typically involves graph traversal (BFS/DFS) on the G matrix.
     // If the MNA matrix is singular, it might indicate a disconnected circuit, or other issues.
 }


void Circuit::solveMNASystem() {
    if (A_mna.rows() == 0) {
        std::cerr << "No matrix to solve. Please add components and build the circuit first." << std::endl;
        return;
    }

    // Solve Ax = b using Eigen's robust solvers
    // Eigen::FullPivLU is a good general-purpose solver.
    // For large sparse systems, you might consider Eigen::SparseLU or Eigen::ConjugateGradient.
    Eigen::VectorXd x;
    Eigen::FullPivLU<Eigen::MatrixXd> lu_solver(A_mna);

    if (!lu_solver.isInvertible()) {
        std::cerr << "ERROR: Circuit matrix is singular. This may indicate a floating node, "
                  << "a short circuit, or a disconnected circuit." << std::endl;
        return;
    }

    x = lu_solver.solve(b_mna);
    printResults(x);
}


void Circuit::printResults(const Eigen::VectorXd& solution) const {
    std::cout << "\n--- Simulation Results ---" << std::endl;

    // Print node voltages
    std::cout << "Node Voltages:" << std::endl;
    std::cout << "V(GND) = 0.0 V" << std::endl; // Ground is always 0V
    for (int i = 0; i < nextNodeId - 1; ++i) { // Iterating from node 1 up to max_node_id
        // Map matrix index (0 to num_nodes-2) back to actual node ID (1 to num_nodes-1)
        int actualNodeId = i + 1;
        std::string nodeName = "N" + std::to_string(actualNodeId); // Default if no custom name
        if (idToNodeName.count(actualNodeId)) {
            nodeName = idToNodeName.at(actualNodeId);
        }
        std::cout << "V(" << nodeName << ") = " << solution(i) << " V" << std::endl;
    }

    // Print currents through voltage sources (if any)
    if (numVoltageSources > 0) {
        std::cout << "\nCurrents Through Voltage Sources:" << std::endl;
        int current_idx_start = nextNodeId - 1; // Index where current unknowns start in the solution vector
        int v_source_count = 0;
        for (const auto& comp : components) {
            if (comp->type == Component::Type::VOLTAGE_SOURCE) {
                // Assuming sourceIndex was correctly assigned during stamping based on voltageSourceIdxCounter
                // Find the source index in the solution vector.
                // This would be better if the VoltageSource object itself stored its assigned index.
                // For this example, let's assume a simple mapping.
                std::cout << "I(" << comp->name << ") = " << solution(current_idx_start + v_source_count) << " A" << std::endl;
                v_source_count++;
            }
        }
    }
    std::cout << "--------------------------" << std::endl;
}



void Circuit::performDCAnalysis() {
    std::cout << "\nPerforming DC Analysis..." << std::endl;
    // 1. Build the MNA matrix for DC analysis (h=0)
    buildMNAMatrix(0.0, 0.0); // Pass 0 for h (DC analysis)

    // 2. Solve Ax = b
    solveMNASystem();
}


void Circuit::performTransientAnalysis(double timeStep, double stopTime, double startTime) {
     std::cout << "\nPerforming Transient Analysis..." << std::endl;
     std::cout << "Time Step (h): " << timeStep << " s, Stop Time: " << stopTime << " s" << std::endl;

     // Initialize capacitor voltages and inductor currents for time=0
     // (This requires a way to get initial values from the circuit definition, or default to 0)
     for (Component* comp : components) {
         if (comp->type == Component::Type::CAPACITOR) {
             dynamic_cast<Capacitor*>(comp)->setPreviousVoltage(0.0); // Initial capacitor voltage
         } else if (comp->type == Component::Type::INDUCTOR) {
             dynamic_cast<Inductor*>(comp)->setPreviousCurrent(0.0); // Initial inductor current
         }
     }

     std::cout << "\nTime\t";
     // Print headers for variables to be printed (e.g., V(node1), I(R1))
     // This requires a more sophisticated .print command parsing and storage.
     // For now, let's just print all node voltages.
     for (int i = 1; i < nextNodeId; ++i) { // Node 1 to max_node_id
         std::string nodeName = idToNodeName.count(i) ? idToNodeName.at(i) : ("N" + std::to_string(i));
         std::cout << "V(" << nodeName << ")\t";
     }
     std::cout << std::endl;


     for (double t = startTime; t <= stopTime + timeStep * 0.5; t += timeStep) { // Add 0.5*h for float comparison robustness
         // 1. Build MNA matrix for current time step 't'
         buildMNAMatrix(t, timeStep); // Pass current time and timeStep for transient calculations

         // 2. Solve Ax = b
         Eigen::FullPivLU<Eigen::MatrixXd> lu_solver(A_mna);
         if (!lu_solver.isInvertible()) {
             std::cerr << "ERROR at t = " << t << "s: Circuit matrix is singular. "
                       << "Simulation stopped." << std::endl;
             break;
         }
         Eigen::VectorXd current_solution = lu_solver.solve(b_mna);

         // Print results for current time step
         std::cout << t << "\t";
         for (int i = 0; i < nextNodeId - 1; ++i) { // Node 1 to max_node_id
             std::cout << current_solution(i) << "\t";
         }
         std::cout << std::endl;


         // 3. Update previous time step values for the next iteration (for C and L)
         for (Component* comp : components) {
             if (comp->type == Component::Type::CAPACITOR) {
                 // Voltage across capacitor is V(node1) - V(node2)
                 int v_node1 = comp->node1 == 0 ? 0 : current_solution(comp->node1 - 1);
                 int v_node2 = comp->node2 == 0 ? 0 : current_solution(comp->node2 - 1);
                 dynamic_cast<Capacitor*>(comp)->setPreviousVoltage(v_node1 - v_node2);
             } else if (comp->type == Component::Type::INDUCTOR) {
                 // Current through inductor is one of the unknowns in the solution vector
                 // This assumes Inductor added its current as an unknown.
                 // If it used the R/I equivalence, you'd need to calculate I_L from V_L and V_prev.
                 // Assuming Inductor added its current as an unknown, find its index.
                 // This requires a more robust way to map voltage source/inductor currents to their index.
                 // For a simple example, let's say the first voltage source current is at node_count-1, first inductor at node_count, etc.
                 // This needs careful indexing.
                 // For now, let's skip updating inductor current unless its index is explicitly managed.
             }
         }
     }
     std::cout << "Transient Analysis Complete." << std::endl;
 }


void Circuit::addComponent(const std::string& typeStr, const std::string& name,
                            const std::string& node1Str, const std::string& node2Str,
                            double value, const std::vector<double>& params) {
     // Basic input validation
     if (name.empty() || node1Str.empty() || node2Str.empty()) {
         std::cerr << "ERROR: Missing parameters for 'add' command." << std::endl;
         return;
     }

     // Check for duplicate component name
     for (const auto& comp : components) {
         if (comp->name == name) {
             std::cerr << "ERROR: Component with name '" << name << "' already exists." << std::endl;
             return;
         }
     }

     // Convert node names to internal IDs
     int n1_id = getNodeId(node1Str);
     int n2_id = getNodeId(node2Str);

     Component* newComp = nullptr;
     if (typeStr == "R") {
         if (value <= 0) { std::cerr << "Error: Resistance must be positive." << std::endl; return; } //[cite: 92]
         newComp = new Resistor(name, n1_id, n2_id, value);
     } else if (typeStr == "C") {
         if (value <= 0) { std::cerr << "Error: Capacitance must be positive." << std::endl; return; } //[cite: 98]
         newComp = new Capacitor(name, n1_id, n2_id, value);
     } else if (typeStr == "L") {
         if (value <= 0) { std::cerr << "Error: Inductance must be positive." << std::endl; return; } //[cite: 100]
         newComp = new Inductor(name, n1_id, n2_id, value);
     } else if (typeStr == "V") {
         newComp = new VoltageSource(name, n1_id, n2_id, value);
     } else if (typeStr == "I") {
         newComp = new CurrentSource(name, n1_id, n2_id, value);
     } else if (typeStr == "D") {
         double Is = params.size() > 0 ? params[0] : 1e-12;
         double eta = params.size() > 1 ? params[1] : 1.0;
         double Vt = params.size() > 2 ? params[2] : 0.026;
         newComp = new Diode(name, n1_id, n2_id, Is, eta, Vt);
     } else if (typeStr == "E") {  // VCVS
         if (params.size() < 3) {
             std::cerr << "Error: VCVS requires controlling nodes and gain." << std::endl;
             return;
         }
         int ctrlN1 = getNodeId(std::to_string((int)params[0]));
         int ctrlN2 = getNodeId(std::to_string((int)params[1]));
         newComp = new VCVS(name, n1_id, n2_id, ctrlN1, ctrlN2, params[2]);
     } else if (typeStr == "G") {  // VCCS
         if (params.size() < 3) {
             std::cerr << "Error: VCCS requires controlling nodes and gain." << std::endl;
             return;
         }
         int ctrlN1 = getNodeId(std::to_string((int)params[0]));
         int ctrlN2 = getNodeId(std::to_string((int)params[1]));
         newComp = new VCCS(name, n1_id, n2_id, ctrlN1, ctrlN2, params[2]);
     } else if (typeStr == "H") {  // CCVS
         if (params.size() < 2) {
             std::cerr << "Error: CCVS requires controlling component and gain." << std::endl;
             return;
         }
         newComp = new CCVS(name, n1_id, n2_id, std::to_string((int)params[0]), params[1]);
     } else if (typeStr == "F") {  // CCCS
         if (params.size() < 2) {
             std::cerr << "Error: CCCS requires controlling component and gain." << std::endl;
             return;
         }
         newComp = new CCCS(name, n1_id, n2_id, std::to_string((int)params[0]), params[1]);
     }

     // TODO: Add support for Diodes, SIN, PULSE, dependent sources (E, G, H, F)
     // For SIN/PULSE, 'params' vector would hold Voffset, Vamplitude, Frequency, etc.

     if (newComp) {
         components.push_back(newComp);
         std::cout << "Added " << typeStr << " " << name << " between " << node1Str << " and " << node2Str << " with value " << value << std::endl;
     } else {
         std::cerr << "ERROR: Unknown component type '" << typeStr << "' or invalid parameters." << std::endl;
     }
 }

 void Circuit::listNodes() const {
     std::cout << "Available nodes: ";// [cite: 107]
     bool first = true;
     for (const auto& pair : idToNodeName) {
         if (!first) {
             std::cout << ", ";
         }
         std::cout << pair.second << " (" << pair.first << ")";
         first = false;
     }
     std::cout << std::endl;
 }

 void Circuit::listComponents(const std::string& typeFilter) const {
     std::cout << "Listing Components";
     if (!typeFilter.empty()) {
         std::cout << " (Type: " << typeFilter << ")";
     }
     std::cout << ":" << std::endl;

     bool found = false;
     for (const auto& comp : components) {
         std::string compTypeStr;
         switch (comp->type) {
             case Component::Type::RESISTOR: compTypeStr = "Resistor"; break;
             case Component::Type::CAPACITOR: compTypeStr = "Capacitor"; break;
             case Component::Type::INDUCTOR: compTypeStr = "Inductor"; break;
             case Component::Type::VOLTAGE_SOURCE: compTypeStr = "VoltageSource"; break;
             case Component::Type::CURRENT_SOURCE: compTypeStr = "CurrentSource"; break;
             case Component::Type::DIODE: compTypeStr = "Diode"; break;
         }

         if (typeFilter.empty() || compTypeStr == typeFilter) {
             std::cout << "  Type: " << compTypeStr << ", Name: " << comp->name
                       << ", Node1: " << idToNodeName.at(comp->node1)
                       << ", Node2: " << idToNodeName.at(comp->node2)
                       << ", Value: " << comp->value << std::endl;
             found = true;
         }
     }
     if (!found && !typeFilter.empty()) {
         std::cout << "  No " << typeFilter << " components found." << std::endl;
     } else if (!found) {
         std::cout << "  No components defined." << std::endl;
     }
 }

 void Circuit::renameNode(const std::string& oldName, const std::string& newName) {
     if (nodeNameToId.find(oldName) == nodeNameToId.end()) {
         std::cerr << "ERROR: Node " << oldName << " does not exist in the circuit" << std::endl;// [cite: 109]
         return;
     }
     if (nodeNameToId.find(newName) != nodeNameToId.end() && newName != oldName) {
         std::cerr << "ERROR: Node name " << newName << " already exists" << std::endl;// [cite: 110]
         return;
     }

     int nodeId = nodeNameToId[oldName];
     nodeNameToId.erase(oldName);
     nodeNameToId[newName] = nodeId;
     idToNodeName[nodeId] = newName; // Update reverse map
     std::cout << "SUCCESS: Node renamed from " << oldName << " to " << newName << std::endl; //[cite: 107]
 }

 void Circuit::handlePrintCommand(const std::string& commandLine) {
     // This function would parse the .print command more thoroughly.
     // For now, solveMNASystem and performTransientAnalysis directly print.
     std::cerr << "WARNING: '.print' command is not fully implemented yet. "
               << "DC and Transient analyses print all voltages by default." << std::endl;
     std::cerr << "Example format: .print TRAN <Tstep> <Tstop> [<Tstart>] [<Tmaxstep>] V(n001) I(R1)" << std::endl;// [cite: 112]
 }






/*
 *
 *
 */












// [Rest of the Circuit class implementations...]
// Continue with all the other method implementations from the original code
// This would include buildMNAMatrix, solveMNASystem, printResults, etc.
// The implementations would be exactly the same as in the original code