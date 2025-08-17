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
void Capacitor::updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) {
    double v1 = 0.0, v2 = 0.0;

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground) {
        v1 = solution(nodeIdToMnaIndex.at(node1));
    }
    if (!n2_is_ground) {
        v2 = solution(nodeIdToMnaIndex.at(node2));
    }

    V_prev = v1 - v2;
}

void Inductor::updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) {
    auto it = ci.find(name);
    if (it != ci.end()) {
        I_prev = solution(it->second);
    }
}

void Diode::updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) {
    double v1 = 0.0, v2 = 0.0;

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground) {
        v1 = solution(nodeIdToMnaIndex.at(node1));
    }
    if (!n2_is_ground) {
        v2 = solution(nodeIdToMnaIndex.at(node2));
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
void Resistor::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double time, double h, int idx) {
    double conductance = 1.0 / value;

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground) {
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(node1)) += conductance;
    }
    if (!n2_is_ground) {
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(node2)) += conductance;
    }
    if (!n1_is_ground && !n2_is_ground) {
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(node2)) -= conductance;
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(node1)) -= conductance;
    }
}

void Capacitor::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double time, double h, int idx) {
    // For DC analysis (h=0), a capacitor is an open circuit, so we do nothing.
    if (h == 0.0)
        return;

    double G_eq = value / h;
    double I_eq = G_eq * V_prev;

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground) {
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(node1)) += G_eq;
        b(nodeIdToMnaIndex.at(node1)) += I_eq;
    }
    if (!n2_is_ground) {
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(node2)) += G_eq;
        b(nodeIdToMnaIndex.at(node2)) -= I_eq;
    }
    if (!n1_is_ground && !n2_is_ground) {
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(node2)) -= G_eq;
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(node1)) -= G_eq;
    }
}

void Inductor::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double time, double h, int idx) {
    if (idx == -1) {
        std::cerr << "ERROR: Inductor '" << name << "' was not assigned a current index." << std::endl;
        return;
    }

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground) {
        A(nodeIdToMnaIndex.at(node1), idx) += 1.0;
        A(idx, nodeIdToMnaIndex.at(node1)) += 1.0;
    }
    if (!n2_is_ground) {
        A(nodeIdToMnaIndex.at(node2), idx) -= 1.0;
        A(idx, nodeIdToMnaIndex.at(node2)) -= 1.0;
    }

    if (h != 0.0) {
        A(idx, idx) -= value / h; // Change D matrix in A
        b(idx) -= (value / h) * I_prev;  // Change the RHS matrix
    }
}

void Diode::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double time, double h, int idx) {
    const double Gmin = 1e-12;

    const double I = Is * (exp(V_prev / (eta * Vt)) - 1.0);
    const double Gd = (Is / (eta * Vt)) * exp(V_prev / (eta * Vt)) + Gmin;
    const double Ieq = I - Gd * V_prev;

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground)
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(node1)) += Gd;
    if (!n2_is_ground)
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(node2)) += Gd;
    if (!n1_is_ground && !n2_is_ground) {
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(node2)) -= Gd;
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(node1)) -= Gd;
    }

    if (!n1_is_ground) {
        b(nodeIdToMnaIndex.at(node1)) -= Ieq;
    }
    if (!n2_is_ground) {
        b(nodeIdToMnaIndex.at(node2)) += Ieq;
    }
}

void VoltageSource::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci,const std::map<int, int>& nodeIdToMnaIndex, double time, double h, int idx) {
    if (idx == -1) {
        std::cerr << "ERROR: VoltageSource '" << name << "' was not assigned a current index." << std::endl;
        return;
    }

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground) {
        A(nodeIdToMnaIndex.at(node1), idx) += 1.0;
    }
    if (!n2_is_ground) {
        A(nodeIdToMnaIndex.at(node2), idx) -= 1.0;
    }
    if (!n1_is_ground) {
        A(idx, nodeIdToMnaIndex.at(node1)) += 1.0;
    }
    if (!n2_is_ground) {
        A(idx, nodeIdToMnaIndex.at(node2)) -= 1.0;
    }

    b(idx) += waveForm->getValue(time);
}

void CurrentSource::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex,double time, double h, int idx) {
    double currentValue = waveForm->getValue(time);

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground) {
        b(nodeIdToMnaIndex.at(node1)) -= currentValue;
    }
    if (!n2_is_ground) {
        b(nodeIdToMnaIndex.at(node2)) += currentValue;
    }
}

void VCVS::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex,double time, double h, int idx) {
    if (idx == -1) {
        std::cerr << "ERROR: VCVS '" << name << "' was not assigned a current index." << std::endl;
        return;
    }

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground) {
        A(nodeIdToMnaIndex.at(node1), idx) += 1.0;
        A(idx, nodeIdToMnaIndex.at(node1)) += 1.0;
    }
    if (!n2_is_ground) {
        A(nodeIdToMnaIndex.at(node2), idx) -= 1.0;
        A(idx, nodeIdToMnaIndex.at(node2)) -= 1.0;
    }

    if (nodeIdToMnaIndex.count(ctrlNode1)) {
        A(idx, nodeIdToMnaIndex.at(ctrlNode1)) -= gain;
    }
    if (nodeIdToMnaIndex.count(ctrlNode2)) {
        A(idx, nodeIdToMnaIndex.at(ctrlNode2)) += gain;
    }
}

void VCCS::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex,double time, double h, int idx) {
    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);
    bool ctrlNode1_is_ground = !nodeIdToMnaIndex.count(ctrlNode1);
    bool ctrlNode2_is_ground = !nodeIdToMnaIndex.count(ctrlNode2);

    if (!n1_is_ground && !ctrlNode1_is_ground) {
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(ctrlNode1)) += gain;
    }
    if (!n1_is_ground && !ctrlNode2_is_ground) {
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(ctrlNode2)) -= gain;
    }
    if (!n2_is_ground && !ctrlNode1_is_ground) {
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(ctrlNode1)) -= gain;
    }
    if (!n2_is_ground && !ctrlNode2_is_ground) {
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(ctrlNode2)) += gain;
    }
}

void CCVS::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci,const std::map<int, int>& nodeIdToMnaIndex, double time, double h, int idx) {
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

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

     if (!n1_is_ground) {
        A(nodeIdToMnaIndex.at(node1), idx) += 1.0;
        A(idx, nodeIdToMnaIndex.at(node1)) += 1.0;
    }
    if (!n2_is_ground) {
        A(nodeIdToMnaIndex.at(node2), idx) -= 1.0;
        A(idx, nodeIdToMnaIndex.at(node2)) -= 1.0;
    }

    A(idx, ctrl_idx) -= gain;
}

void CCCS::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci,const std::map<int, int>& nodeIdToMnaIndex, double time, double h, int idx) {
    auto it = ci.find(ctrlCompName);
    if (it == ci.end()) {
        std::cerr << "ERROR: Controlling component '" << ctrlCompName << "' for CCCS '" << name << "' not found or has no current." << std::endl;
        return;
    }
    int ctrl_idx = it->second;

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground)
        A(nodeIdToMnaIndex.at(node1), ctrl_idx) += gain;
    if (!n2_is_ground)
        A(nodeIdToMnaIndex.at(node2), ctrl_idx) -= gain;
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


CEREAL_REGISTER_TYPE(Resistor)
CEREAL_REGISTER_TYPE(Capacitor)
CEREAL_REGISTER_TYPE(Inductor)
CEREAL_REGISTER_TYPE(Diode)
CEREAL_REGISTER_TYPE(VoltageSource)
CEREAL_REGISTER_TYPE(CurrentSource)
CEREAL_REGISTER_TYPE(VCVS)
CEREAL_REGISTER_TYPE(VCCS)
CEREAL_REGISTER_TYPE(CCVS)
CEREAL_REGISTER_TYPE(CCCS)


CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, Resistor);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, Capacitor);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, Inductor);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, Diode);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, VoltageSource);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, CurrentSource);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, VCVS);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, VCCS);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, CCVS);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, CCCS);
