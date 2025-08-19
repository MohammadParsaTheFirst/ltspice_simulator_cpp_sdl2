#include "circuit.h"
#include <limits>

void printWelcome () {
    std::cout << "Welcome to LTspice OOP Project Sharif University of Technology (Terminal Mode)!" << std::endl;
}

void printCredits() {
    std::cout << "CREDITS: " << std::endl;
    std::cout << "     Mohammad Parsa Dini, Parsa Paktinat" << std::endl;
}

void printHelp() {
    std::cout << "\n------------ LTspice OOP Project Sharif University of Technology Help ------------\n";
    std::cout << "COMPONENT DEFINITION:\n";
    std::cout << "  delete <type><name>\n";
    std::cout << "  delete GND <nodeName>\n";
    std::cout << "  add <type><name> <node1> <node2> <value> [params...]\n";
    std::cout << "  Examples:\n";
    std::cout << "    R: add Rload n1 n2 1k\n";
    std::cout << "    C: add Cfilt out GND 1u\n";
    std::cout << "    L: add Lchoke in n1 10m\n";
    std::cout << "    GND: add GND N001\n";
    std::cout << "    V (DC): add Vcc vdd GND 5\n";
    std::cout << "    I (DC): add I_bias n_bias GND 1m\n";
    std::cout << "    V (SIN): add Vsig in GND SIN(0 1 1k)  (offset=0, amp=1, freq=1k)\n";
    std::cout << "    I (SIN): add Isig in GND SIN(0 1m 50) (offset=0, amp=1m, freq=50)\n";
    std::cout << "    D (Diode): add D1 fwd rev (uses default model)\n";
    std::cout << "    E (VCVS): add Evcvs n_out GND n_in GND 2.5 (V(n_out) = 2.5 * V(n_in))\n";
    std::cout << "    G (VCCS): add Gvccs n_out GND n_in GND 5m (I(n_out) = 5m * V(n_in))\n";
    std::cout << "    H (CCVS): add Hccvs n_out GND V_sense 50 (V(n_out) = 50 * I(V_sense))\n";
    std::cout << "    F (CCCS): add Fcccs n_out GND V_sense 10 (I(n_out) = 10 * I(V_sense))\n\n";
    std::cout << "CIRCUIT MANAGEMENT:\n";
    std::cout << "  .nodes          - List all defined nodes\n";
    std::cout << "  .list           - List all components\n";
    std::cout << "  .rename <old> <new> - Rename a node\n\n";
    std::cout << "FILE MANAGEMENT:\n";
    std::cout << "  show existing schematics - Obvious!\n";
    std::cout << "  fileHere                 - Show the path of the file right now!\n\n";
    std::cout << "ANALYSIS:\n";
    std::cout << "  .DC <SourceName> <StartVal> <EndVal> <Increment> - Perform DC sweep analysis\n";
    std::cout << "  .TRAN <Tstop> [<Tstep>] [<Tstart>]               - Perform transient analysis\n\n";
    std::cout << "PRINTING:\n";
    std::cout << "  .print TRAN <Tstop> [<Tstep>] [<Tstart>] <variable1> <variable1> ...               - Print the transient results \n";
    std::cout << "  .print DC <SourceName> <StartVal> <EndVal> <Increment> <variable1> <variable1> ... - Print the DC sweep results\n\n";
    std::cout << "GENERAL:\n";
    std::cout << "  help            - Show this help message\n";
    std::cout << "  exit            - Quit the program\n";
    std::cout << "  clear           - Clear the schematic\n";
    std::cout << "----------------------------------------------------------------------------------\n" << std::endl;
}

int main() {
    Circuit circuit;
    std::string command;

    printWelcome();
    printCredits();
    printHelp();

    // Load the last default circuit
    circuit.loadCircuitFromFile();

    while (true) {
        std::cout << ">>> ";
        if (!std::getline(std::cin, command))
            break;

        std::stringstream ss(command);
        std::string cmdType;
        ss >> cmdType;

        try {
            if (cmdType == "exit") {
                circuit.saveCircuitToFile();
                break;
            }

            else if (cmdType == "fileHere")
                std::cout << circuit.getPathRightNow() << std::endl;

            else if (cmdType == "help")
                printHelp();

            else if (cmdType == "clear")
                circuit.clearSchematic();

            else if (cmdType == "show") {
                std::string firstParam, secondParam;
                if (!(ss >> firstParam >> secondParam))
                    throw::std::runtime_error("Invalid command!");
                if (firstParam != "existing" || secondParam != "schematics")
                    throw::std::runtime_error("Invalid command!");
                circuit.saveCircuitToFile();
                circuit.showExistingFiles();
            }

            else if (cmdType == "add") {
                std::string comp_str, node1_str, node2_str;
                ss >> comp_str >> node1_str >> node2_str;

                if (node1_str == node2_str)
                    throw std::runtime_error("Nodes cannot be the same.");

                char type_char = comp_str[0];

                double value = 0.0;
                std::string value_str;

                std::vector<double> numericParams;
                std::vector<std::string> stringParams;
                bool isSinusoidal = false;

                // For diode model
                std::string model;
                if (type_char == 'R' || type_char == 'C' || type_char == 'L') {
                    if (!(ss >> value_str))
                        throw std::runtime_error("Missing value.");
                    value = parseSpiceValue(value_str);
                }
                else if (comp_str == "GND") {
                    circuit.addGround(node1_str);
                    std::cout << "Ground added." << std::endl;
                    continue;
                }
                else if (type_char == 'V' || comp_str.substr(0,13) == "CurrentSource") {
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
                            freq_str =next_token.substr(0, next_token.size() - 1);
                        double offset = parseSpiceValue(offset_str);
                        double amplitude = parseSpiceValue(amplitude_str);
                        double freq = parseSpiceValue(freq_str);
                        numericParams = {offset, amplitude, freq};
                    }
                    else
                        value = parseSpiceValue(next_token);
                }
                else if (type_char == 'D') {
                    if (!(ss >> model))
                        throw std::runtime_error("Missing value.");
                    if (model != "D" && model != "Z") {
                        std::string error_msg = "Model " + model + " not dound in library.";
                        throw std::runtime_error(error_msg);
                    }
                }
                else if (type_char == 'E' || type_char == 'G') {
                    std::string c_n1, c_n2;
                    if (!(ss >> c_n1 >> c_n2 >> value_str))
                        throw std::runtime_error("Missing parameters for time-dependent source.");
                    value = parseSpiceValue(value_str);
                    stringParams = {c_n1, c_n2};
                }
                else if (type_char == 'H' || type_char == 'F') {
                    std::string c_name;
                    if (!(ss >> c_name >> value_str))
                        throw std::runtime_error("Missing parameters for time-dependent source.");
                    value = parseSpiceValue(value_str);
                    stringParams = {c_name};
                }
                else {
                    std::string errorString = "Element " + comp_str + " not found in library.";
                    throw std::runtime_error(errorString);
                }
                if (comp_str.substr(0,13) == "CurrentSource")
                    type_char = 'I';
                circuit.addComponent(std::string(1, type_char), comp_str, node1_str, node2_str, value, numericParams, stringParams, isSinusoidal);
                command = std::string(1, type_char) + " " + command.substr(4);
                circuit.circuitNetList.push_back(command);
            }

            else if (cmdType == ".nodes")
                circuit.listNodes();

            else if (cmdType == ".list") {
                char componentType;
                ss >> componentType;
                circuit.listComponents(componentType);
            }

            else if (cmdType == ".rename") {
                std::string node, oldName, newName;
                if (!(ss >> node >> oldName >> newName))
                     throw std::runtime_error("Invalid syntax - correct form:\n.rename node <old_name> <new_name>");

                circuit.renameNode(oldName, newName);
            }

            else if (cmdType == ".DC") {
                std::string sourceName, startValue, endValue, increment;
                if (!(ss >> sourceName >> startValue >> endValue >> increment))
                    throw std::runtime_error("Invalid syntax - correct form:\n.DC <sourceName> <startValue> <endValue> <increment>");
                double startValueDouble = parseSpiceValue(startValue);
                double endVlaueDouble = parseSpiceValue(endValue);
                double incrementDouble = parseSpiceValue(increment);
                circuit.performDCAnalysis(sourceName, startValueDouble, endVlaueDouble, incrementDouble);
            }

            else if (cmdType == ".TRAN") {
                std::vector<std::string> params;
                std::string word;
                while (ss >> word)
                    params.push_back(word);
                double tstop = 0.0, tstart = 0.0, tmaxstep = 0.0;
                 if (params.size() < 1)
                    throw std::runtime_error("Invalid format. Use: .tran <stoptime> [starttime] [maxtimestep]");
                if (params.size() >= 1)
                    tstop = parseSpiceValue(params[0]);
                if (params.size() >= 2)
                    tstart = parseSpiceValue(params[1]);
                if (params.size() >= 3)
                    tmaxstep = parseSpiceValue(params[2]);
                circuit.performTransientAnalysis( tstop, tstart, tmaxstep);
            }

            else if (cmdType == ".print") {
                std::string analysisType;
                if (!(ss >> analysisType))
                    throw std::runtime_error("Syntax error in command.");

                if (analysisType == "TRAN") {
                    std::vector<std::string> variablesToPrint;
                    std::vector<std::string> params;
                    std::string word;
                    while (ss >> word)
                        params.push_back(word);

                    int j = 1;
                    double tstop = 0.0, tstart = 0.0, tmaxstep = 0.0;
                    if (params[0][0] == 'V' || params[0][0] == 'I')
                        throw std::runtime_error("Syntax error in command");
                    tstop = parseSpiceValue(params[0]);
                    if (params[1][0] != 'V' && params[1][0] != 'I') {
                        tstart = parseSpiceValue(params[1]);
                        j = 2;
                    }
                    if (params.size() >= 3) {
                        if (params[2][0] != 'V' && params[2][0] != 'I') {
                            tmaxstep = parseSpiceValue(params[2]);
                            j = 3;
                        }
                    }
                    
                    for (int i = j; i < params.size(); i++)
                        variablesToPrint.push_back(params[i]);

                    circuit.performTransientAnalysis(tstop, tstart, tmaxstep);
                    circuit.printTransientResults(variablesToPrint);
                }
                else if (analysisType == "DC") {
                    std::string sourceName, startValue, endValue, increment, variable;
                    if (!(ss >> sourceName >> startValue >> endValue >> increment >> variable))
                        throw std::runtime_error("Syntax error in command");
                    double startValueDouble = parseSpiceValue(startValue);
                    double endVlaueDouble = parseSpiceValue(endValue);
                    double incrementDouble = parseSpiceValue(increment);

                    circuit.performDCAnalysis(sourceName, startValueDouble, endVlaueDouble, incrementDouble);
                    circuit.printDcSweepResults(sourceName, variable);
                }
                else
                    throw std::runtime_error("Syntax error in command");
            }

            else if (cmdType == "delete") {
                std::string componentName, nodeName;
                if (!(ss >> componentName))
                    throw std::runtime_error("Missing component name.");
                if (componentName == "GND") {
                    ss >> nodeName;
                    circuit.deleteGround(nodeName);
                }
                else {
                    char typeChar = componentName[0];
                    circuit.deleteComponent(componentName, typeChar);
                }
            }

            else if (!cmdType.empty())
                throw std::runtime_error("Synatx error");

            else
                throw std::runtime_error("Invalid command!");
        }
        catch (const std::exception& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        }
    }

    return 0;
}