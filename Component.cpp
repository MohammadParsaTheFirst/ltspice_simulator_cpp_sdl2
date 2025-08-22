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


// -------------------------------- Fuck --------------------------------
void Component::serialize(QDataStream& out) const {
    out << QString::fromStdString(name) << (qint32)node1 << (qint32)node2 << value;
}
void Component::deserialize(QDataStream& in) {
    QString qName;
    qint32 n1, n2;
    in >> qName >> n1 >> n2 >> value;
    name = qName.toStdString();
    node1 = n1;
    node2 = n2;
}

void Capacitor::serialize(QDataStream& out) const {
    Component::serialize(out);
    out << V_prev;
}
void Capacitor::deserialize(QDataStream& in) {
    Component::deserialize(in);
    in >> V_prev;
}

void VoltageSource::serialize(QDataStream& out) const {
    Component::serialize(out);
    out << (qint32)sourceType << param1 << param2 << param3;
}
void VoltageSource::deserialize(QDataStream& in) {
    Component::deserialize(in);
    qint32 st;
    in >> st >> param1 >> param2 >> param3;
    sourceType = (SourceType)st;
}

void Inductor::serialize(QDataStream& out) const {
    Component::serialize(out);
    out << I_prev;
}
void Inductor::deserialize(QDataStream& in) {
    Component::deserialize(in);
    in >> I_prev;
}

void Diode::serialize(QDataStream& out) const {
    Component::serialize(out);
    out << Is << Vt << eta << V_prev;
}
void Diode::deserialize(QDataStream& in) {
    Component::deserialize(in);
    in >> Is >> Vt >> eta >> V_prev;
}

void CurrentSource::serialize(QDataStream& out) const {
    Component::serialize(out);
    out << (qint32)sourceType << param1 << param2 << param3;
}
void CurrentSource::deserialize(QDataStream& in) {
    Component::deserialize(in);
    qint32 st;
    in >> st >> param1 >> param2 >> param3;
    sourceType = (SourceType)st;
}

void VCVS::serialize(QDataStream& out) const {
    Component::serialize(out);
    out << (qint32)ctrlNode1 << (qint32)ctrlNode2 << gain;
}
void VCVS::deserialize(QDataStream& in) {
    Component::deserialize(in);
    qint32 cn1, cn2;
    in >> cn1 >> cn2 >> gain;
    ctrlNode1 = cn1;
    ctrlNode2 = cn2;
}

void VCCS::serialize(QDataStream& out) const {
    Component::serialize(out);
    out << (qint32)ctrlNode1 << (qint32)ctrlNode2 << gain;
}
void VCCS::deserialize(QDataStream& in) {
    Component::deserialize(in);
    qint32 cn1, cn2;
    in >> cn1 >> cn2 >> gain;
    ctrlNode1 = cn1;
    ctrlNode2 = cn2;
}

void CCVS::serialize(QDataStream& out) const {
    Component::serialize(out);
    out << QString::fromStdString(ctrlCompName) << gain;
}
void CCVS::deserialize(QDataStream& in) {
    Component::deserialize(in);
    QString ctrlName;
    in >> ctrlName >> gain;
    ctrlCompName = ctrlName.toStdString();
}

void CCCS::serialize(QDataStream& out) const {
    Component::serialize(out);
    out << QString::fromStdString(ctrlCompName) << gain;
}
void CCCS::deserialize(QDataStream& in) {
    Component::deserialize(in);
    QString ctrlName;
    in >> ctrlName >> gain;
    ctrlCompName = ctrlName.toStdString();
}