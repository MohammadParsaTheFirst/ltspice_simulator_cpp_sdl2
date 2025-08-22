#ifndef COMPONENT_H
#define COMPONENT_H

#include <Eigen/Dense>
#include <QString>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <QDataStream>

class Circuit;

const double PI = 3.141592;

// -------------------------------- Component Class and Its Implementations --------------------------------
class Component {
public:
    enum class Type {
        RESISTOR,
        CAPACITOR,
        INDUCTOR,
        VOLTAGE_SOURCE, CURRENT_SOURCE,
        DIODE,
        VCVS, VCCS, CCVS, CCCS,
        AC_VOLTAGE_SOURCE
    };

    Type type;
    std::string name;
    int node1;
    int node2;
    double value;

    Component() : type(Type::RESISTOR), node1(-1), node2(-1), value(0.0) {}
    Component(Type t, const std::string& n, int n1, int n2, double v) : type(t), name(std::move(n)), node1(n1), node2(n2), value(v) {}
    virtual ~Component() {}

    virtual void reset() {}
    virtual void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int> &ci,
        const std::map<int, int>& nodeIdToMnaIndex, double time, double h, int idx) = 0;
    virtual void stampMNA_AC(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int>& ci,
        const std::map<int, int>& nodeIdToMnaIndex, double omega, int idx) = 0;
    virtual void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) {}
    virtual bool isNonlinear() const { return false; }
    virtual std::string getName() const { return name; }
    virtual bool needsCurrentUnknown() const { return false; }

    virtual QString getTypeString() const = 0;
    virtual void serialize(QDataStream& out) const;
    virtual void deserialize(QDataStream& in);
};

class Resistor : public Component {
public:
    Resistor() : Component() {}
    Resistor(const std::string& n, int n1, int n2, double v);
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &,const std::map<int, int>& nodeIdToMnaIndex,  double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;
    QString getTypeString() const override { return "Resistor"; }
};

class Capacitor : public Component {
private:
    double V_prev;
public:
    Capacitor() : Component(), V_prev(0.0) {}
    Capacitor(const std::string& n, int n1, int n2, double v);
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) override;
    void reset() override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    QString getTypeString() const override { return "Capacitor"; }
    void serialize(QDataStream& out) const override;
    void deserialize(QDataStream& in) override;
};

class Inductor : public Component {
private:
    double I_prev;
public:
    Inductor() : Component(), I_prev(0.0) {}
    Inductor(const std::string& n, int n1, int n2, double v);
    bool needsCurrentUnknown() const override { return true; }
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) override;
    void reset() override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &,const std::map<int, int>& nodeIdToMnaIndex,  double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    QString getTypeString() const override { return "Inductor"; }
    void serialize(QDataStream& out) const override;
    void deserialize(QDataStream& in) override;
};

class Diode : public Component {
private:
    double Is;
    double Vt;
    double eta;
    double V_prev;
public:
    Diode() : Component(), Is(1e-12), Vt(0.026), eta(1.0), V_prev(0.7) {}
    Diode(const std::string& n, int n1, int n2, double Is = 1e-12, double eta = 1.0, double Vt = 0.026);
    bool isNonlinear() const override { return true; }
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;
    void setPreviousVoltage(double v) { V_prev = v; }
    void reset() override;

    QString getTypeString() const override { return "Diode"; }
    void serialize(QDataStream& out) const override;
    void deserialize(QDataStream& in) override;
};

class VoltageSource : public Component {
public:
    enum class SourceType {DC, Sinusoidal};
private:
    SourceType sourceType;
    double param1, param2, param3;
public:
    VoltageSource() : Component(), sourceType(SourceType::DC), param1(0), param2(0), param3(0) {}
    VoltageSource(const std::string& name, int node1, int node2, SourceType type, double p1, double p2, double p3);

    SourceType getSourceType() const { return sourceType; }
    double getParam1() const { return param1; }
    double getParam2() const { return param2; }
    double getParam3() const { return param3; }

    bool needsCurrentUnknown() const override { return true; }
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;
    void setValue(double v);
    double getCurrentValue(double time) const;

    QString getTypeString() const override { return "VoltageSource"; }
    void serialize(QDataStream& out) const override;
    void deserialize(QDataStream& in) override;
};

// AC voltage source
class ACVoltageSource : public Component {
public:
    ACVoltageSource() : Component() {}
    ACVoltageSource(const std::string& name, int node1, int node2);

    bool needsCurrentUnknown() const override { return true; }
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;
    double getValueAtFrequency(double omega) const;

    QString getTypeString() const override { return "ACVoltageSource"; }
};

class CurrentSource : public Component {
public:
    enum class SourceType {DC, Sinusoidal};
private:
    SourceType sourceType;
    double param1, param2, param3;
public:
    CurrentSource() : Component(), sourceType(SourceType::DC), param1(0), param2(0), param3(0) {}
    CurrentSource(const std::string& n, int n1, int n2, SourceType type, double p1, double p2, double p3);

    SourceType getSourceType() const { return sourceType; }
    double getParam1() const { return param1; }
    double getParam2() const { return param2; }
    double getParam3() const { return param3; }

    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;
    void setValue(double v);
    double getCurrentValue(double time) const;

    QString getTypeString() const override { return "CurrentSource"; }
    void serialize(QDataStream& out) const override;
    void deserialize(QDataStream& in) override;
};

// VCVS - Type E
class VCVS : public Component {
private:
    int ctrlNode1, ctrlNode2;
    double gain;
public:
    VCVS() : Component(), ctrlNode1(0), ctrlNode2(0), gain(0.0) {}
    VCVS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain);

    int getCtrlNode1() const {return ctrlNode1;}
    int getCtrlNode2() const {return ctrlNode2;}
    double getGain() const {return gain;}

    bool needsCurrentUnknown() const override { return true; }
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    QString getTypeString() const override { return "VCVS"; }
    void serialize(QDataStream& out) const override;
    void deserialize(QDataStream& in) override;
};

// VCCS - Type G
class VCCS : public Component {
private:
    int ctrlNode1, ctrlNode2;
    double gain;
public:
    VCCS() : Component(), ctrlNode1(0), ctrlNode2(0), gain(0.0) {}
    VCCS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain);

    int getCtrlNode1() const {return ctrlNode1;}
    int getCtrlNode2() const {return ctrlNode2;}
    double getGain() const {return gain;}

    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>& nodeIdToMnaIndex, double, double , int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    QString getTypeString() const override { return "VCCS"; }
    void serialize(QDataStream& out) const override;
    void deserialize(QDataStream& in) override;
};

// CCVS - Type H
class CCVS : public Component {
private:
    std::string ctrlCompName;
    double gain;
    int sourceIndex;
public:
    CCVS() : Component(), ctrlCompName(""), gain(0.0), sourceIndex(-1) {}
    CCVS(const std::string& n, int n1, int n2, const std::string& ctrlComp, double gain);

    std::string getCtrlCompName() const {return ctrlCompName;}
    double getGain() const {return gain;}
    int getSourceIndex() const {return sourceIndex;}

    bool needsCurrentUnknown() const override { return true; }
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    QString getTypeString() const override { return "CCVS"; }
    void serialize(QDataStream& out) const override;
    void deserialize(QDataStream& in) override;
};

// CCCS - Type F
class CCCS : public Component {
private:
    std::string ctrlCompName;
    double gain;
public:
    CCCS() : Component(), ctrlCompName(""), gain(0.0) {}
    CCCS(const std::string& n, int n1, int n2, const std::string& ctrlComp, double gain);

    std::string getCtrlCompName() const {return ctrlCompName;}
    double getGain() const {return gain;}

    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    QString getTypeString() const override { return "CCCS"; }
    void serialize(QDataStream& out) const override;
    void deserialize(QDataStream& in) override;
};
// -------------------------------- Component Class and Its Implementations --------------------------------

#endif // COMPONENT_H