#include "ComponentFactory.h"

Component* ComponentFactory::createComponent(
        const std::string& typeStr,
        const std::string& name,
        int n1_id,
        int n2_id,
        double value,
        const std::vector<double>& numericParams,
        const std::vector<std::string>& stringParams,
        bool isSinusoidal,
        Circuit* circuit)
{
    Component* newComp = nullptr;

    if (typeStr == "R") {
        if (value <= 0)
            throw std::runtime_error("Resistance cannot be zero or negative");
        newComp = new Resistor(name, n1_id, n2_id, value);
    }
    else if (typeStr == "C") {
        if (value <= 0)
            throw std::runtime_error("Capacitance cannot be zero or negative");
        newComp = new Capacitor(name, n1_id, n2_id, value);
    }
    else if (typeStr == "L") {
        if (value <= 0)
            throw std::runtime_error("Inductance cannot be zero or negative");
        newComp = new Inductor(name, n1_id, n2_id, value);
    }
    else if (typeStr == "V") {
        if (isSinusoidal)
            newComp = new VoltageSource(name, n1_id, n2_id, VoltageSource::SourceType::Sinusoidal, numericParams[0], numericParams[1], numericParams[2]);
        else
            newComp = new VoltageSource(name, n1_id, n2_id, VoltageSource::SourceType::DC, value, 0.0, 0.0);
    }
    else if (typeStr == "AC") {
        newComp = new ACVoltageSource(name, n1_id, n2_id);
    }
    else if (typeStr == "I") {
        if (isSinusoidal)
            newComp = new CurrentSource(name, n1_id, n2_id, CurrentSource::SourceType::Sinusoidal, numericParams[0], numericParams[1], numericParams[2]);
        else
            newComp = new CurrentSource(name, n1_id, n2_id, CurrentSource::SourceType::DC, value, 0.0, 0.0);
    }
    else if (typeStr == "D")
        newComp = new Diode(name, n1_id, n2_id, 1e-12, 1.0, 0.026);

    else if (typeStr == "E") { // VCVS
        int ctrlN1 = circuit->getNodeId(stringParams[0]);
        int ctrlN2 = circuit->getNodeId(stringParams[1]);
        newComp = new VCVS(name, n1_id, n2_id, ctrlN1, ctrlN2, value);
    }
    else if (typeStr == "G") { // VCCS
        int ctrlN1 = circuit->getNodeId(stringParams[0]);
        int ctrlN2 = circuit->getNodeId(stringParams[1]);
        newComp = new VCCS(name, n1_id, n2_id, ctrlN1, ctrlN2, value);
    }
    else if (typeStr == "H") // CCVS
        newComp = new CCVS(name, n1_id, n2_id, stringParams[0], value);

    else if (typeStr == "F") // CCCS
        newComp = new CCCS(name, n1_id, n2_id, stringParams[0], value);

    else {
        std::string errorString = "Element " + name + " not found in library.";
        throw std::runtime_error(errorString);
    }

    return newComp;
}

std::shared_ptr<Component> ComponentFactory::createComponentFromType(const QString& typeStr) {
    if (typeStr == "Resistor") return std::make_shared<Resistor>();
    if (typeStr == "Capacitor") return std::make_shared<Capacitor>();
    if (typeStr == "Inductor") return std::make_shared<Inductor>();
    if (typeStr == "VoltageSource") return std::make_shared<VoltageSource>();
    if (typeStr == "CurrentSource") return std::make_shared<CurrentSource>();
    if (typeStr == "ACVoltageSource") return std::make_shared<ACVoltageSource>();
    if (typeStr == "Diode") return std::make_shared<Diode>();
    if (typeStr == "VCVS") return std::make_shared<VCVS>();
    if (typeStr == "VCCS") return std::make_shared<VCCS>();
    if (typeStr == "CCVS") return std::make_shared<CCVS>();
    if (typeStr == "CCCS") return std::make_shared<CCCS>();

    return nullptr;
}