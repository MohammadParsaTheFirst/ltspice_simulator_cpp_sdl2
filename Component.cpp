#include "component.h"

// -------------------------------- Constructor impementation --------------------------------
Resistor::Resistor(const std::string& n, int n1, int n2, double v)
    : Component(Type::RESISTOR, n, n1, n2, v) {}

Capacitor::Capacitor(const std::string& n, int n1, int n2, double v)
    : Component(Type::CAPACITOR, n, n1, n2, v), V_prev(0.0) {}

Inductor::Inductor(const std::string& n, int n1, int n2, double v)
    : Component(Type::INDUCTOR, n, n1, n2, v), I_prev(0.0) {}

Diode::Diode(const std::string& n, int n1, int n2, double is, double et, double vt)
    : Component(Type::DIODE, n, n1, n2, 0.0), Is(is), eta(et), Vt(vt), V_prev(0.7) {}

VoltageSource::VoltageSource(const std::string& n, int n1, int n2, std::unique_ptr<IWaveformStrategy> wf)
    : Component(Type::VOLTAGE_SOURCE, n, n1, n2, 0.0), waveForm(std::move(wf)) {}

CurrentSource::CurrentSource(const std::string& n, int n1, int n2, std::unique_ptr<IWaveformStrategy> wf)
    : Component(Type::CURRENT_SOURCE, n, n1, n2, 0.0), waveForm(std::move(wf)) {}

VCVS::VCVS(const std::string& n, int n1, int n2, int c_n1, int c_n2, double g)
    : Component(Type::VCVS, n, n1, n2, 0.0), ctrlNode1(c_n1), ctrlNode2(c_n2), gain(g) {}

VCCS::VCCS(const std::string& n, int n1, int n2, int c_n1, int c_n2, double g)
    : Component(Type::VCCS, n, n1, n2, 0.0), ctrlNode1(c_n1), ctrlNode2(c_n2), gain(g) {}

CCVS::CCVS(const std::string& n, int n1, int n2, const std::string &c_name, double g)
    : Component(Type::CCVS, n, n1, n2, 0.0), ctrlCompName(std::move(c_name)), gain(g) {}

CCCS::CCCS(const std::string& n, int n1, int n2, const std::string &c_name, double g)
    : Component(Type::CCCS, n, n1, n2, 0.0), ctrlCompName(std::move(c_name)), gain(g) {}
// -------------------------------- Constructor impementation --------------------------------


// -------------------------------- Update state implementation --------------------------------
void Capacitor::updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, int groundNodeID) {
    double v1 = 0.0, v2 = 0.0;

    if (node1 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1 : node1;
        v1 = solution(n1_idx);
    }
    if (node2 != groundNodeID) {
        int n2_idx = (node2 > groundNodeID) ? node2 - 1 : node2;
        v2 = solution(n2_idx);
    }

    V_prev = v1 - v2;
}

void Inductor::updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, int groundNodeID) {
    auto it = ci.find(name);
    if (it != ci.end()) {
        I_prev = solution(it->second);
    }
}

void Diode::updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, int groundNodeID) {
    double v1 = 0.0, v2 = 0.0;

    if (node1 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1 : node1;
        v1 = solution(n1_idx);
    }
    if (node2 != groundNodeID) {
        int n2_idx = (node2 > groundNodeID) ? node2 - 1 : node2;
        v2 = solution(n2_idx);
    }
    V_prev = v1 - v2;
}
// -------------------------------- Update state implementation --------------------------------


// -------------------------------- Reset initial values --------------------------------
void Capacitor::reset() {
    V_prev = 0.0;
}

void Inductor::reset() {
    I_prev = 0.0;
}

void Diode::reset() {
    V_prev = 0.0;
}
// -------------------------------- Reset initial values --------------------------------


// -------------------------------- MNA Stamping Implementations --------------------------------
void Resistor::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, double time, double h, int groundNodeID, int idx) {
    double conductance = 1.0 / value;
    int n1_idx = -1, n2_idx = -1;

    if (node1 != groundNodeID) {
        n1_idx = (node1 > groundNodeID) ? node1 - 1 : node1;
    }
    if (node2 != groundNodeID) {
        n2_idx = (node2 > groundNodeID) ? node2 - 1 : node2;
    }
    if (n1_idx != -1) {
        A(n1_idx, n1_idx) += conductance;
    }
    if (n2_idx != -1) {
        A(n2_idx, n2_idx) += conductance;
    }

    if (n1_idx != -1 && n2_idx != -1) {
        A(n1_idx, n2_idx) -= conductance;
        A(n2_idx, n1_idx) -= conductance;
    }
}

void Capacitor::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, double time, double h, int groundNodeID, int idx) {
    // For DC analysis (h=0), a capacitor is an open circuit, so we do nothing.
    if (h == 0.0)
        return;

    double G_eq = value / h;
    double I_eq = G_eq * V_prev;

    if (node1 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1 : node1;
        A(n1_idx, n1_idx) += G_eq;
        b(n1_idx) += I_eq;
    }
    if (node2 != groundNodeID) {
        int n2_idx = (node2 > groundNodeID) ? node2 - 1 : node2;
        A(n2_idx, n2_idx) += G_eq;
        b(n2_idx) -= I_eq;
    }
    if (node1 != groundNodeID && node2 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1 : node1;
        int n2_idx = (node2 > groundNodeID) ? node2 - 1 : node2;
        A(n1_idx, n2_idx) -= G_eq;
        A(n2_idx, n1_idx) -= G_eq;
    }
}

void Inductor::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, double time, double h, int groundNodeID, int idx) {
    if (idx == -1) {
        std::cerr << "ERROR: Inductor '" << name << "' was not assigned a current index." << std::endl;
        return;
    }

    if (node1 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1 : node1;
        A(n1_idx, idx) += 1.0;
        A(idx, n1_idx) += 1.0;
    }
    if (node2 != groundNodeID) {
        int n2_idx = (node2 > groundNodeID) ? node2 - 1 : node2;
        A(n2_idx, idx) -= 1.0;
        A(idx, n2_idx) -= 1.0;
    }

    if (h != 0.0) {
        A(idx, idx) -= value / h; // Change D matrix in A
        b(idx) -= (value / h) * I_prev;  // Change the RHS matrix
    }
}

void Diode::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, double time, double h, int groundNodeID, int idx) {
    const double Gmin = 1e-12;

    const double I = Is * (exp(V_prev / (eta * Vt)) - 1.0);
    const double Gd = (Is / (eta * Vt)) * exp(V_prev / (eta * Vt)) + Gmin;
    const double Ieq = I - Gd * V_prev;

    int n1_idx = (node1 != groundNodeID) ? ((node1 > groundNodeID) ? node1 - 1 : node1) : -1;
    int n2_idx = (node2 != groundNodeID) ? ((node2 > groundNodeID) ? node2 - 1 : node2) : -1;

    if (n1_idx != -1)
        A(n1_idx, n1_idx) += Gd;
    if (n2_idx != -1)
        A(n2_idx, n2_idx) += Gd;
    if (n1_idx != -1 && n2_idx != -1) {
        A(n1_idx, n2_idx) -= Gd;
        A(n2_idx, n1_idx) -= Gd;
    }

    if (n1_idx != -1) {
        b(n1_idx) -= Ieq;
    }
    if (n2_idx != -1) {
        b(n2_idx) += Ieq;
    }
}

void VoltageSource::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, double time, double h, int groundNodeID, int idx) {
    if (idx == -1) {
        std::cerr << "ERROR: VoltageSource '" << name << "' was not assigned a current index." << std::endl;
        return;
    }

    if (node1 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1:node1;
        A(n1_idx, idx) += 1.0;
    }
    if (node2 != groundNodeID) {
        int n2_idx = (node2 > groundNodeID) ? node2 - 1:node2;
        A(n2_idx, idx) -= 1.0;
    }
    if (node1 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1:node1;
        A(idx, n1_idx) += 1.0;
    }
    if (node2 != groundNodeID) {
        int n2_idx = (node2 > groundNodeID) ? node2 - 1:node2;
        A(idx, n2_idx) -= 1.0;
    }

    b(idx) += waveForm->getValue(time);
}

void CurrentSource::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, double time, double h, int groundNodeID, int idx) {
    double currentValue = waveForm->getValue(time);
    if (node1 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1:node1;
        b(n1_idx) -= currentValue;
    }
    if (node2 != groundNodeID) {
        int n2_idx = (node2 > groundNodeID) ? node2 - 1:node2;
        b(n2_idx) += currentValue;
    }
}

void VCVS::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, double time, double h, int groundNodeID, int idx) {
    if (idx == -1) {
        std::cerr << "ERROR: VCVS '" << name << "' was not assigned a current index." << std::endl;
        return;
    }

    if (node1 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1:node1;
        A(n1_idx, idx) += 1.0;
        A(idx, n1_idx) += 1.0;
    }
    if (node2 != groundNodeID) {
        int n2_idx = (node2 > groundNodeID) ? node2 - 1:node2;
        A(n2_idx, idx) -= 1.0;
        A(idx, n2_idx) -= 1.0;
    }

    if (ctrlNode1 != groundNodeID) {
        int cn1_idx = (ctrlNode1 > groundNodeID) ? ctrlNode1 - 1:ctrlNode1;
        A(idx, cn1_idx) -= gain;
    }
    if (ctrlNode2 != groundNodeID) {
        int cn2_idx = (ctrlNode2 > groundNodeID) ? ctrlNode2 - 1:ctrlNode2;
        A(idx, cn2_idx) += gain;
    }
}

void VCCS::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, double time, double h, int groundNodeID, int idx) {
    if (node1 != groundNodeID && ctrlNode1 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1:node1;
        int cn1_idx = (ctrlNode1 > groundNodeID) ? ctrlNode1 - 1:ctrlNode1;
        A(n1_idx, cn1_idx) += gain;
    }
    if (node1 != groundNodeID && ctrlNode2 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1:node1;
        int cn2_idx = (ctrlNode2 > groundNodeID) ? ctrlNode2 - 1:ctrlNode2;
        A(n1_idx, cn2_idx) -= gain;
    }
    if (node2 != groundNodeID && ctrlNode1 != groundNodeID) {
        int n2_idx = (node2 > groundNodeID) ? node2 - 1:node2;
        int cn1_idx = (ctrlNode1 > groundNodeID) ? ctrlNode1 - 1:ctrlNode1;
        A(n2_idx, cn1_idx) -= gain;
    }
    if (node2 != groundNodeID && ctrlNode2 != groundNodeID) {
        int n2_idx = (node2 > groundNodeID) ? node2 - 1:node2;
        int cn2_idx = (ctrlNode2 > groundNodeID) ? ctrlNode2 - 1:ctrlNode2;
        A(n2_idx, cn2_idx) += gain;
    }
}

void CCVS::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, double time, double h, int groundNodeID, int idx) {
    if (idx == -1) {
        std::cerr << "ERROR: CCVS '" << name << "' was not assigned a current index." << std::endl;
        return;
    }

    auto it = ci.find(ctrlCompName);
    if (it == ci.end()) {
        std::cerr << "ERROR: Controlling component '" << ctrlCompName << "' for CCVS '" << name << "' not found or has no current." << std::endl;
        return;
    }
    int ctrl_idx = it->second;

    if (node1 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1 : node1;
        A(n1_idx, idx) += 1.0;
        A(idx, n1_idx) += 1.0;
    }
    if (node2 != groundNodeID) {
        int n2_idx = (node2 > groundNodeID) ? node2 - 1:node2;
        A(n2_idx, idx) -= 1.0;
        A(idx, n2_idx) -= 1.0;
    }

    A(idx, ctrl_idx) -= gain;
}

void CCCS::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, double time, double h, int groundNodeID, int idx) {
    auto it = ci.find(ctrlCompName);
    if (it == ci.end()) {
        std::cerr << "ERROR: Controlling component '" << ctrlCompName << "' for CCCS '" << name << "' not found or has no current." << std::endl;
        return;
    }
    int ctrl_idx = it->second;

    if (node1 != groundNodeID) {
        int n1_idx = (node1 > groundNodeID) ? node1 - 1 : node1;
        A(n1_idx, ctrl_idx) += gain;
    }
    if (node2 != groundNodeID) {
        int n2_idx = (node2 > groundNodeID) ? node2 - 1:node2;
        A(n2_idx, ctrl_idx) -= gain;
    }
}
// -------------------------------- MNA Stamping Implementations --------------------------------


// -------------------------------- Set Values for DC Sweep --------------------------------
void VoltageSource::setValue(double v) {
    if (auto i = dynamic_cast<DCWaveform*>(waveForm.get()))
        i->setValue(v);
    else
        std::cout << "Cannot perform DC Sweep on non-dc source: " << name << std::endl;
}

void CurrentSource::setValue(double v) {
    if (auto i = dynamic_cast<DCWaveform*>(waveForm.get()))
        i->setValue(v);
    else
        std::cout << "Cannot perform DC Sweep on non-dc source: " << name << std::endl;
}
// -------------------------------- Set Values for DC Sweep --------------------------------