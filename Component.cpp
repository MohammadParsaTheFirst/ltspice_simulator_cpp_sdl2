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

VoltageSource::VoltageSource(const std::string& n, int n1, int n2, SourceType st, double p1, double p2, double p3)
    : Component(Type::VOLTAGE_SOURCE, n, n1, n2, 0.0), sourceType(st), param1(p1), param2(p2), param3(p3) {}

ACVoltageSource::ACVoltageSource(const std::string& name, int node1, int node2)
    : Component(Type::AC_VOLTAGE_SOURCE, name, node1, node2, 1.0) {}

CurrentSource::CurrentSource(const std::string& n, int n1, int n2, SourceType st, double p1, double p2, double p3)
    : Component(Type::CURRENT_SOURCE, n, n1, n2, 0.0), sourceType(st), param1(p1), param2(p2), param3(p3) {}

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


// -------------------------------- MNA Stamping Implementations for AC Sweep --------------------------------
void Resistor::stampMNA_AC(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double omega, int idx) {
    stampMNA(A, b, ci, nodeIdToMnaIndex, 0, 0, idx);
}

void Capacitor::stampMNA_AC(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double omega, int idx) {
    double admittance = omega * value;
    if (admittance < 1e-12)
        admittance = 1e-12;

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground)
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(node1)) += admittance;
    if (!n2_is_ground)
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(node2)) += admittance;
    if (!n1_is_ground && !n2_is_ground) {
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(node2)) -= admittance;
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(node1)) -= admittance;
    }
}

void Inductor::stampMNA_AC(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double omega, int idx) {
    if (omega < 1e-9)
        omega = 1e-9;
    double admittance = 1.0 / (omega * value);

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground)
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(node1)) += admittance;
    if (!n2_is_ground)
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(node2)) += admittance;
    if (!n1_is_ground && !n2_is_ground) {
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(node2)) -= admittance;
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(node1)) -= admittance;
    }
}

void Diode::stampMNA_AC(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double omega, int idx) {
    double conductance = 1.0;

    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground)
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(node1)) += conductance;
    if (!n2_is_ground)
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(node2)) += conductance;
    if (!n1_is_ground && !n2_is_ground) {
        A(nodeIdToMnaIndex.at(node1), nodeIdToMnaIndex.at(node2)) -= conductance;
        A(nodeIdToMnaIndex.at(node2), nodeIdToMnaIndex.at(node1)) -= conductance;
    }
}

void VoltageSource::stampMNA_AC(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double omega, int idx) {
    stampMNA(A, b, ci, nodeIdToMnaIndex, 0, 0, idx);
}
void ACVoltageSource::stampMNA_AC(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double omega, int idx) {
    stampMNA(A, b, ci, nodeIdToMnaIndex, omega, 0, idx);
}
void CurrentSource::stampMNA_AC(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double omega, int idx) {
    stampMNA(A, b, ci, nodeIdToMnaIndex, 0, 0, idx);
}
void VCVS::stampMNA_AC(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double omega, int idx) {
    stampMNA(A, b, ci, nodeIdToMnaIndex, 0, 0, idx);
}
void VCCS::stampMNA_AC(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double omega, int idx) {
    stampMNA(A, b, ci, nodeIdToMnaIndex, 0, 0, idx);
}
void CCVS::stampMNA_AC(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double omega, int idx) {
    stampMNA(A, b, ci, nodeIdToMnaIndex, 0, 0, idx);
}
void CCCS::stampMNA_AC(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex, double omega, int idx) {
    stampMNA(A, b, ci, nodeIdToMnaIndex, 0, 0, idx);
}
// -------------------------------- MNA Stamping Implementations for AC Sweep --------------------------------


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
        A(idx, nodeIdToMnaIndex.at(node1)) += 1.0;
    }
    if (!n2_is_ground) {
        A(nodeIdToMnaIndex.at(node2), idx) -= 1.0;
        A(idx, nodeIdToMnaIndex.at(node2)) -= 1.0;
    }

    b(idx) += getCurrentValue(time);
}

void ACVoltageSource::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci,const std::map<int, int>& nodeIdToMnaIndex, double timeOrOmega, double h, int idx) {
    if (idx == -1) {
        std::cerr << "ERROR: VoltageSource '" << name << "' was not assigned a current index." << std::endl;
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

    b(idx) += getValueAtFrequency(timeOrOmega);
}

void CurrentSource::stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex,double time, double h, int idx) {
    bool n1_is_ground = !nodeIdToMnaIndex.count(node1);
    bool n2_is_ground = !nodeIdToMnaIndex.count(node2);

    if (!n1_is_ground) {
        b(nodeIdToMnaIndex.at(node1)) -= getCurrentValue(time);
    }
    if (!n2_is_ground) {
        b(nodeIdToMnaIndex.at(node2)) += getCurrentValue(time);
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
    if (sourceType == SourceType::DC)
        param1 = v;
}

void CurrentSource::setValue(double i) {
    if (sourceType == SourceType::DC)
        param1 = i;
}
// -------------------------------- Set Values for DC Sweep --------------------------------


// -------------------------------- Get Values of independent sources --------------------------------
double VoltageSource::getCurrentValue(double time) const {
    if (sourceType == SourceType::DC)
        return param1;
    else
        return param1 + param2 * sin(2*PI*param3*time);
}

double CurrentSource::getCurrentValue(double time) const {
    if (sourceType == SourceType::DC)
        return param1;
    else
        return param1 + param2 * sin(2*PI*param3*time);
}

double ACVoltageSource::getValueAtFrequency(double omega) const {
    return this->value;
}
// -------------------------------- Get Values of independent sources --------------------------------


// -------------------------------- Saving data with binary files with fstream --------------------------------
void Component::save_binary(std::ofstream& file) const {
    file.write(reinterpret_cast<const char*>(&type), sizeof(type));
    size_t name_len = name.size();
    file.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
    file.write(name.c_str(), name_len);
    file.write(reinterpret_cast<const char*>(&node1), sizeof(node1));
    file.write(reinterpret_cast<const char*>(&node2), sizeof(node2));
    file.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void Capacitor::save_binary(std::ofstream& file) const {
    Component::save_binary(file);
    file.write(reinterpret_cast<const char*>(&V_prev), sizeof(V_prev));
}

void Inductor::save_binary(std::ofstream& file) const {
    Component::save_binary(file);
    file.write(reinterpret_cast<const char*>(&I_prev), sizeof(I_prev));
}

void Diode::save_binary(std::ofstream& file) const {
    Component::save_binary(file);
    file.write(reinterpret_cast<const char*>(&Is), sizeof(Is));
    file.write(reinterpret_cast<const char*>(&Vt), sizeof(Vt));
    file.write(reinterpret_cast<const char*>(&eta), sizeof(eta));
    file.write(reinterpret_cast<const char*>(&V_prev), sizeof(V_prev));
}

void VoltageSource::save_binary(std::ofstream& file) const {
    Component::save_binary(file);
    file.write(reinterpret_cast<const char*>(&sourceType), sizeof(sourceType));
    file.write(reinterpret_cast<const char*>(&param1), sizeof(param1));
    file.write(reinterpret_cast<const char*>(&param2), sizeof(param2));
    file.write(reinterpret_cast<const char*>(&param3), sizeof(param3));
}

void CurrentSource::save_binary(std::ofstream& file) const {
    Component::save_binary(file);
    file.write(reinterpret_cast<const char*>(&sourceType), sizeof(sourceType));
    file.write(reinterpret_cast<const char*>(&param1), sizeof(param1));
    file.write(reinterpret_cast<const char*>(&param2), sizeof(param2));
    file.write(reinterpret_cast<const char*>(&param3), sizeof(param3));
}

void VCVS::save_binary(std::ofstream& file) const {
    Component::save_binary(file);
    file.write(reinterpret_cast<const char*>(&ctrlNode1), sizeof(ctrlNode1));
    file.write(reinterpret_cast<const char*>(&ctrlNode2), sizeof(ctrlNode2));
    file.write(reinterpret_cast<const char*>(&gain), sizeof(gain));
}

void VCCS::save_binary(std::ofstream& file) const {
    Component::save_binary(file);
    file.write(reinterpret_cast<const char*>(&ctrlNode1), sizeof(ctrlNode1));
    file.write(reinterpret_cast<const char*>(&ctrlNode2), sizeof(ctrlNode2));
    file.write(reinterpret_cast<const char*>(&gain), sizeof(gain));
}

void save_string_binary(std::ofstream& file, const std::string& str) {
    size_t len = str.size();
    file.write(reinterpret_cast<const char*>(&len), sizeof(len));
    file.write(str.c_str(), len);
}

void CCVS::save_binary(std::ofstream& file) const {
    Component::save_binary(file);
    save_string_binary(file, ctrlCompName);
    file.write(reinterpret_cast<const char*>(&gain), sizeof(gain));
}

void CCCS::save_binary(std::ofstream& file) const {
    Component::save_binary(file);
    save_string_binary(file, ctrlCompName);
    file.write(reinterpret_cast<const char*>(&gain), sizeof(gain));
}
// -------------------------------- Saving data with binary files with fstream --------------------------------


// -------------------------------- Loading data from binary files with fstream --------------------------------
void Component::load_binary(std::ifstream& file) {
    size_t name_len;
    file.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
    char* name_buf = new char[name_len + 1];
    file.read(name_buf, name_len);
    name_buf[name_len] = '\0';
    name = name_buf;
    delete[] name_buf;
    file.read(reinterpret_cast<char*>(&node1), sizeof(node1));
    file.read(reinterpret_cast<char*>(&node2), sizeof(node2));
    file.read(reinterpret_cast<char*>(&value), sizeof(value));
}

void Capacitor::load_binary(std::ifstream& file) {
    Component::load_binary(file);
    file.read(reinterpret_cast<char*>(&V_prev), sizeof(V_prev));
}

void Inductor::load_binary(std::ifstream& file) {
    Component::load_binary(file);
    file.read(reinterpret_cast<char*>(&I_prev), sizeof(I_prev));
}

void Diode::load_binary(std::ifstream& file) {
    Component::load_binary(file);
    file.read(reinterpret_cast<char*>(&Is), sizeof(Is));
    file.read(reinterpret_cast<char*>(&Vt), sizeof(Vt));
    file.read(reinterpret_cast<char*>(&eta), sizeof(eta));
    file.read(reinterpret_cast<char*>(&V_prev), sizeof(V_prev));
}

void VoltageSource::load_binary(std::ifstream& file) {
    Component::load_binary(file);
    file.read(reinterpret_cast<char*>(&sourceType), sizeof(sourceType));
    file.read(reinterpret_cast<char*>(&param1), sizeof(param1));
    file.read(reinterpret_cast<char*>(&param2), sizeof(param2));
    file.read(reinterpret_cast<char*>(&param3), sizeof(param3));
}

void CurrentSource::load_binary(std::ifstream& file) {
    Component::load_binary(file);
    file.read(reinterpret_cast<char*>(&sourceType), sizeof(sourceType));
    file.read(reinterpret_cast<char*>(&param1), sizeof(param1));
    file.read(reinterpret_cast<char*>(&param2), sizeof(param2));
    file.read(reinterpret_cast<char*>(&param3), sizeof(param3));
}

void VCVS::load_binary(std::ifstream& file) {
    Component::load_binary(file);
    file.read(reinterpret_cast<char*>(&ctrlNode1), sizeof(ctrlNode1));
    file.read(reinterpret_cast<char*>(&ctrlNode2), sizeof(ctrlNode2));
    file.read(reinterpret_cast<char*>(&gain), sizeof(gain));
}

void VCCS::load_binary(std::ifstream& file) {
    Component::load_binary(file);
    file.read(reinterpret_cast<char*>(&ctrlNode1), sizeof(ctrlNode1));
    file.read(reinterpret_cast<char*>(&ctrlNode2), sizeof(ctrlNode2));
    file.read(reinterpret_cast<char*>(&gain), sizeof(gain));
}

std::string load_string_binary(std::ifstream& file) {
    size_t len;
    file.read(reinterpret_cast<char*>(&len), sizeof(len));
    char* buf = new char[len + 1];
    file.read(buf, len);
    buf[len] = '\0';
    std::string str = buf;
    delete[] buf;
    return str;
}

void CCVS::load_binary(std::ifstream& file) {
    Component::load_binary(file);
    ctrlCompName = load_string_binary(file);
    file.read(reinterpret_cast<char*>(&gain), sizeof(gain));
}

void CCCS::load_binary(std::ifstream& file) {
    Component::load_binary(file);
    ctrlCompName = load_string_binary(file);
    file.read(reinterpret_cast<char*>(&gain), sizeof(gain));
}
// -------------------------------- Loading data with binary files with fstream --------------------------------