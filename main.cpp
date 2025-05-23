#include "circuit.h"
#include <iostream>
#include <sstream>
#include <string>

int main() {
    Circuit circuit;
    std::string command;

    std::cout << "Welcome to Mini-LTSpice (Terminal Mode)!" << std::endl;
    std::cout << "Type 'help' for commands, 'exit' to quit." << std::endl;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);

        std::stringstream ss(command);
        std::string cmdType;
        ss >> cmdType;

        if (cmdType == "exit") {
            break;
        } else if (cmdType == "help") {
            std::cout << "Commands:" << std::endl;
            std::cout << "  add <type> <name> <node1> <node2> <value>" << std::endl;
            std::cout << "    Types: R, C, L, V, I, D (diode), E (VCVS), G (VCCS), H (CCVS), F (CCCS)" << std::endl;
            std::cout << "    Examples:" << std::endl;
            std::cout << "      add R R1 1 2 1000" << std::endl;
            std::cout << "      add D D1 1 2 [Is=1e-12] [eta=1.0] [Vt=0.026]" << std::endl;
            std::cout << "      add E VCVS1 3 4 1 2 10.0 (VCVS: V(3,4) = 10*V(1,2))" << std::endl;

            std::cout << "  .nodes - List all defined nodes" << std::endl; //[cite: 107]
            std::cout << "  .list [component_type] - List all components, or specific types (e.g., .list Resistor)" << std::endl;// [cite: 107]
            std::cout << "  .rename node <old_name> <new_name> - Rename a node" << std::endl; //[cite: 107]
            std::cout << "  .dc - Perform DC analysis" << std::endl;
            std::cout << "  .tran <timestep> <stoptime> [starttime] - Perform transient analysis (e.g., .tran 1u 1m)" << std::endl;
            std::cout << "  .print <analysis_type> <variable1> <variable2> ... (e.g., .print TRAN V(1) I(R1))" << std::endl;// [cite: 112]
        } else if (cmdType == "add") {
            std::string type, name, node1, node2;
            double value;
            if (!(ss >> type >> name >> node1 >> node2 >> value)) {
                std::cerr << "ERROR: Invalid 'add' command format. Expected: add <type> <name> <node1> <node2> <value>" << std::endl; //[cite: 94, 99, 101]
                continue;
            }
            circuit.addComponent(type, name, node1, node2, value);
        } else if (cmdType == ".nodes") {
            circuit.listNodes();
        } else if (cmdType == ".list") {
            std::string filter;
            ss >> filter;
            circuit.listComponents(filter);
        } else if (cmdType == ".rename") {
            std::string subCommand, oldName, newName;
            if (!(ss >> subCommand >> oldName >> newName) || subCommand != "node") {
                std::cerr << "ERROR: Invalid .rename command syntax. Correct format: .rename node <old_name> <new_name>" << std::endl;// [cite: 111]
                continue;
            }
            circuit.renameNode(oldName, newName);
        } else if (cmdType == ".dc") {
            // Check for ground node presence before analysis
            if (!circuit.hasNode("GND")) { // This check requires a helper method
                std::cerr << "ERROR: No ground node detected in the circuit." << std::endl; //[cite: 148]
                continue;
            }
            circuit.performDCAnalysis();
        } else if (cmdType == ".tran") {
            double timeStep, stopTime, startTime = 0.0;
            if (!(ss >> timeStep >> stopTime)) {
                std::cerr << "ERROR: Invalid '.tran' command format. Expected: .tran <timestep> <stoptime> [starttime]" << std::endl;
                continue;
            }
            // Optional startTime
            if (ss >> startTime) {
                /* startTime was provided */

            }

            // Check for ground node presence before analysis
            if (!circuit.hasNode("GND")) { // This check requires a helper method
                std::cerr << "ERROR: No ground node detected in the circuit." << std::endl; //[cite: 148]
                continue;
            }
            circuit.performTransientAnalysis(timeStep, stopTime, startTime);
        } else if (cmdType == ".print") {
            circuit.handlePrintCommand(command);
        }
        else {
            std::cout << "Unknown command: " << command << ". Type 'help' for a list of commands." << std::endl;
        }
    }

    return 0;
}
