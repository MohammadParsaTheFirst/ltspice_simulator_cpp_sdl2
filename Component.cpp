#include "component.h"
#include "Eigen/Dense"
#include <iostream>
#include <limits>

// Resistor implementation
Resistor::Resistor(const std::string& n, int n1, int n2, double v)
    : Component(Type::RESISTOR, n, n1, n2, v) {}

void Resistor::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx, double h) {
    double conductance = 1.0 / value;
    if (node1 != 0) {
        A(node1, node1) += conductance;
    }
    if (node2 != 0) {
        A(node2, node2) += conductance;
    }
    if (node1 != 0 && node2 != 0) {
        A(node1, node2) -= conductance;
        A(node2, node1) -= conductance;
    }
}

// Capacitor implementation
Capacitor::Capacitor(const std::string& n, int n1, int n2, double v)
    : Component(Type::CAPACITOR, n, n1, n2, v), V_prev(0.0) {}

void Capacitor::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx, double h) {
    if (h == 0.0 || std::abs(h) < std::numeric_limits<double>::epsilon()) {
        return;
    }

    double conductance_eq = value / h;
    double current_source_eq = value * V_prev / h;

    if (node1 != 0) {
        A(node1, node1) += conductance_eq;
    }
    if (node2 != 0) {
        A(node2, node2) += conductance_eq;
    }
    if (node1 != 0 && node2 != 0) {
        A(node1, node2) -= conductance_eq;
        A(node2, node1) -= conductance_eq;
    }

    if (node1 != 0) {
        b(node1) -= current_source_eq;
    }
    if (node2 != 0) {
        b(node2) += current_source_eq;
    }
}

// Inductor implementation
Inductor::Inductor(const std::string& n, int n1, int n2, double v)
    : Component(Type::INDUCTOR, n, n1, n2, v), I_prev(0.0) {}

void Inductor::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx, double h) {
    if (h == 0.0 || std::abs(h) < std::numeric_limits<double>::epsilon()) {
        if (idx == -1) {
            std::cerr << "ERROR: Inductor needs an assigned index for DC analysis." << std::endl;
            return;
        }

        if (node1 != 0) {
            A(idx, node1) = 1.0;
            A(node1, idx) = 1.0;
        }
        if (node2 != 0) {
            A(idx, node2) = -1.0;
            A(node2, idx) = -1.0;
        }
        A(idx, idx) = 0.0;
        b(idx) = 0.0;
        return;
    }

    if (idx == -1) {
        std::cerr << "ERROR: Inductor needs an assigned index for transient analysis." << std::endl;
        return;
    }

    if (node1 != 0) {
        A(node1, idx) += 1.0;
        A(idx, node1) += 1.0;
    }
    if (node2 != 0) {
        A(node2, idx) -= 1.0;
        A(idx, node2) -= 1.0;
    }

    A(idx, idx) -= (value / h);
    b(idx) -= (value / h) * I_prev;
}


// Add this implementation:
Diode::Diode(const std::string& n, int n1, int n2, double Is, double eta, double Vt)
    : Component(Type::DIODE, n, n1, n2, 0.0), Is(Is), Vt(Vt), eta(eta), V_prev(0.0) {}

void Diode::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx, double h) {
    // Simple diode model using Newton-Raphson iteration
    const double Vd = V_prev;  // Start with previous voltage
    const double I = Is * (exp(Vd / (eta * Vt)) - 1.0);
    const double Gd = Is / (eta * Vt) * exp(Vd / (eta * Vt)); // Diode conductance

    // Stamp the diode as a resistor + current source
    if (node1 != 0) {
        A(node1, node1) += Gd;
    }
    if (node2 != 0) {
        A(node2, node2) += Gd;
    }
    if (node1 != 0 && node2 != 0) {
        A(node1, node2) -= Gd;
        A(node2, node1) -= Gd;
    }

    // Current source stamp
    double Ieq = I - Gd * Vd;
    if (node1 != 0) {
        b(node1) -= Ieq;
    }
    if (node2 != 0) {
        b(node2) += Ieq;
    }
}


// VoltageSource implementation
VoltageSource::VoltageSource(const std::string& n, int n1, int n2, double v, int sIdx)
    : Component(Type::VOLTAGE_SOURCE, n, n1, n2, v), sourceIndex(sIdx) {}

void VoltageSource::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx, double h) {
    if (idx == -1) {
        std::cerr << "ERROR: VoltageSource needs an assigned index for MNA matrix." << std::endl;
        return;
    }
    this->setSourceIndex(idx);

    if (node1 != 0) {
        A(node1, sourceIndex) += 1.0;
    }
    if (node2 != 0) {
        A(node2, sourceIndex) -= 1.0;
    }

    if (node1 != 0) {
        A(sourceIndex, node1) += 1.0;
    }
    if (node2 != 0) {
        A(sourceIndex, node2) -= 1.0;
    }

    b(sourceIndex) += value;
}

// CurrentSource implementation
CurrentSource::CurrentSource(const std::string& n, int n1, int n2, double v)
    : Component(Type::CURRENT_SOURCE, n, n1, n2, v) {}

void CurrentSource::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx, double h) {
    if (node1 != 0) {
        b(node1) -= value;
    }
    if (node2 != 0) {
        b(node2) += value;
    }
}


// Add these implementations:

// VCVS implementation
VCVS::VCVS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain)
    : Component(Type::VOLTAGE_SOURCE, n, n1, n2, 0.0), ctrlNode1(ctrlN1), ctrlNode2(ctrlN2), gain(gain) {}

void VCVS::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx, double h) {
    if (idx == -1) {
        std::cerr << "ERROR: VCVS needs an assigned index for MNA matrix." << std::endl;
        return;
    }

    // Stamp as regular voltage source
    if (node1 != 0) {
        A(node1, idx) += 1.0;
        A(idx, node1) += 1.0;
    }
    if (node2 != 0) {
        A(node2, idx) -= 1.0;
        A(idx, node2) -= 1.0;
    }

    // Add controlling equation: Vout = gain * Vctrl
    if (ctrlNode1 != 0) {
        A(idx, ctrlNode1) -= gain;
    }
    if (ctrlNode2 != 0) {
        A(idx, ctrlNode2) += gain;
    }
}

// VCCS implementation
VCCS::VCCS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain)
    : Component(Type::CURRENT_SOURCE, n, n1, n2, 0.0), ctrlNode1(ctrlN1), ctrlNode2(ctrlN2), gain(gain) {}

void VCCS::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx, double h) {
    // Stamp the transconductance
    if (node1 != 0 && ctrlNode1 != 0) A(node1, ctrlNode1) += gain;
    if (node1 != 0 && ctrlNode2 != 0) A(node1, ctrlNode2) -= gain;
    if (node2 != 0 && ctrlNode1 != 0) A(node2, ctrlNode1) -= gain;
    if (node2 != 0 && ctrlNode2 != 0) A(node2, ctrlNode2) += gain;
}

// CCVS implementation
CCVS::CCVS(const std::string& n, int n1, int n2, const std::string& ctrlComp, double gain)
    : Component(Type::VOLTAGE_SOURCE, n, n1, n2, 0.0), ctrlCompName(ctrlComp), gain(gain), sourceIndex(-1) {}

void CCVS::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx, double h) {
    if (idx == -1) {
        std::cerr << "ERROR: CCVS needs an assigned index for MNA matrix." << std::endl;
        return;
    }
    this->setSourceIndex(idx);

    // Stamp as regular voltage source
    if (node1 != 0) {
        A(node1, sourceIndex) += 1.0;
        A(sourceIndex, node1) += 1.0;
    }
    if (node2 != 0) {
        A(node2, sourceIndex) -= 1.0;
        A(sourceIndex, node2) -= 1.0;
    }

    // The controlling current will be handled during the solving phase
    // We need to find the controlling component's current index
    // This would require additional logic in Circuit::buildMNAMatrix
    // For now, we'll assume the controlling component is a voltage source
    // and its current index is known
    // In a complete implementation, we'd need to track component-current mappings
    A(sourceIndex, sourceIndex) = -gain;
}

// CCCS implementation
CCCS::CCCS(const std::string& n, int n1, int n2, const std::string& ctrlComp, double gain)
    : Component(Type::CURRENT_SOURCE, n, n1, n2, 0.0), ctrlCompName(ctrlComp), gain(gain) {}

void CCCS::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, int idx, double h) {
    // Similar to CCVS, we need to know the controlling component's current index
    // For now, we'll assume it's a voltage source and its current is at idx
    if (node1 != 0) {
        A(node1, idx) += gain;
    }
    if (node2 != 0) {
        A(node2, idx) -= gain;
    }
}
