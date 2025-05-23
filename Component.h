#ifndef COMPONENT_H
#define COMPONENT_H

#include "Eigen/Dense"
#include <string>

class Circuit; // Forward declaration

// Base class for all circuit components
class Component {
public:
    enum class Type { RESISTOR, CAPACITOR, INDUCTOR, VOLTAGE_SOURCE, CURRENT_SOURCE, DIODE };

    Type type;
    std::string name;
    int node1; // Internal integer node ID
    int node2; // Internal integer node ID
    double value;

    // Constructor
    Component(Type t, const std::string& n, int n1, int n2, double v)
        : type(t), name(n), node1(n1), node2(n2), value(v) {}

    // Virtual destructor for proper memory management
    virtual ~Component() {}

    // Pure virtual method to stamp the component's contribution onto the MNA matrix
    virtual void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx = -1, double h = 0.0) = 0;
};


// Derived classes for specific components
class Resistor : public Component {
public:
    Resistor(const std::string& n, int n1, int n2, double v);
    void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx = -1, double h = 0.0) override;
};

class Capacitor : public Component {
private:
    double V_prev; // Stores voltage across capacitor from previous time step for transient analysis
public:
    Capacitor(const std::string& n, int n1, int n2, double v);
    void setPreviousVoltage(double v) { V_prev = v; }
    double getPreviousVoltage() const { return V_prev; }
    void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx = -1, double h = 0.0) override;
};

class Inductor : public Component {
private:
    double I_prev; // Stores current through inductor from previous time step for transient analysis
public:
    Inductor(const std::string& n, int n1, int n2, double v);
    void setPreviousCurrent(double i) { I_prev = i; }
    double getPreviousCurrent() const { return I_prev; }
    void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx = -1, double h = 0.0) override;
};


class Diode : public Component {
private:
    double Is;    // Saturation current
    double Vt;    // Thermal voltage (typically 26mV at room temp)
    double eta;   // Ideality factor (typically 1-2)
    double V_prev; // Previous voltage for NR iteration

public:
    Diode(const std::string& n, int n1, int n2, double Is = 1e-12, double eta = 1.0, double Vt = 0.026);
    void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx = -1, double h = 0.0) override;
    void setPreviousVoltage(double v) { V_prev = v; }
};



class VoltageSource : public Component {
private:
    int sourceIndex; // Index for this voltage source's current in the MNA solution vector
public:
    VoltageSource(const std::string& n, int n1, int n2, double v, int sIdx = -1);
    void setSourceIndex(int idx) { sourceIndex = idx; }
    void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx = -1, double h = 0.0) override;
};

class CurrentSource : public Component {
public:
    CurrentSource(const std::string& n, int n1, int n2, double v);
    void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx = -1, double h = 0.0) override;
};




// Voltage Controlled Voltage Source
class VCVS : public Component {
private:
    int ctrlNode1, ctrlNode2; // Controlling nodes
    double gain;
public:
    VCVS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain);
    void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx = -1, double h = 0.0) override;
};

// Voltage Controlled Current Source
class VCCS : public Component {
private:
    int ctrlNode1, ctrlNode2; // Controlling nodes
    double gain;
public:
    VCCS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain);
    void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx = -1, double h = 0.0) override;
};

// Current Controlled Voltage Source
class CCVS : public Component {
private:
    std::string ctrlCompName; // Name of controlling component
    double gain;
    int sourceIndex;
public:
    CCVS(const std::string& n, int n1, int n2, const std::string& ctrlComp, double gain);
    void setSourceIndex(int idx) { sourceIndex = idx; }
    void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx = -1, double h = 0.0) override;
};

// Current Controlled Current Source
class CCCS : public Component {
private:
    std::string ctrlCompName; // Name of controlling component
    double gain;
public:
    CCCS(const std::string& n, int n1, int n2, const std::string& ctrlComp, double gain);
    void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx = -1, double h = 0.0) override;
};



#endif // COMPONENT_H