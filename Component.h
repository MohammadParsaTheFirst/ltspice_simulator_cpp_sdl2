#ifndef COMPONENT_H
#define COMPONENT_H

#include <Eigen/Dense>
#include "WaveForm.h"
#include <string>
#include <iostream>
#include <memory>
#include <map>

class Circuit;

// -------------------------------- Component Class and Its Implementations --------------------------------
class Component {
public:
    enum class Type {
        RESISTOR,
        CAPACITOR,
        INDUCTOR,
        VOLTAGE_SOURCE, CURRENT_SOURCE,
        DIODE,
        VCVS, VCCS, CCVS, CCCS
    };

    Type type;
    std::string name;
    int node1;
    int node2;
    double value;

    Component(Type t, const std::string& n, int n1, int n2, double v) : type(t), name(std::move(n)), node1(n1), node2(n2), value(v) {}

    virtual ~Component() {}
    virtual void reset() {}
    virtual void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int> &ci,
        double time, double h, int groundNodeID, int idx) = 0;
    virtual void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, int groundNodeID) {}
    virtual bool isNonlinear() const { return false; }
    virtual bool needsCurrentUnknown() const { return false; }

    std::string getName() const {
        return name;
    }
};

class Resistor : public Component {
public:
    Resistor(const std::string& n, int n1, int n2, double v);
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, double, double, int, int) override;
};

class Capacitor : public Component {
private:
    double V_prev;
public:
    Capacitor(const std::string& n, int n1, int n2, double v);
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, int groundNodeID) override;
    void reset() override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, double, double, int, int) override;
};

class Inductor : public Component {
private:
    double I_prev;
public:
    Inductor(const std::string& n, int n1, int n2, double v);
    bool needsCurrentUnknown() const override { return true; }
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, int groundNodeID) override;
    void reset() override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, double, double, int, int) override;
};

class Diode : public Component {
private:
    double Is;
    double Vt;
    double eta;
    double V_prev;

public:
    Diode(const std::string& n, int n1, int n2, double Is = 1e-12, double eta = 1.0, double Vt = 0.026);
    bool isNonlinear() const override { return true; }
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, int groundNodeID) override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, double, double, int, int) override;
    void setPreviousVoltage(double v) { V_prev = v; }
    void reset() override;
};

class VoltageSource : public Component {
private:
    std::unique_ptr<IWaveformStrategy> waveForm;
public:
    VoltageSource(const std::string& name, int node1, int node2, std::unique_ptr<IWaveformStrategy> wf);
    bool needsCurrentUnknown() const override { return true; }
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, double, double, int, int) override;
    void setValue(double v);
};

class CurrentSource : public Component {
private:
    std::unique_ptr<IWaveformStrategy> waveForm;
public:
    CurrentSource(const std::string& n, int n1, int n2, std::unique_ptr<IWaveformStrategy> wf);
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, double, double, int, int) override;
    void setValue(double v);
};

// VCVS - Type E
class VCVS : public Component {
private:
    int ctrlNode1, ctrlNode2;
    double gain;
public:
    VCVS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain);
    bool needsCurrentUnknown() const override { return true; }
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, double, double, int, int) override;
};

// VCCS - Type G
class VCCS : public Component {
private:
    int ctrlNode1, ctrlNode2;
    double gain;
public:
    VCCS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain);
    void stampMNA(Eigen::MatrixXd& A, Eigen::VectorXd& b, const std::map<std::string, int> &ci,
        double time, double h, int groundNodeID, int idx) override;
};

// CCVS - Type H
class CCVS : public Component {
private:
    std::string ctrlCompName;
    double gain;
    int sourceIndex;
public:
    CCVS(const std::string& n, int n1, int n2, const std::string& ctrlComp, double gain);
    bool needsCurrentUnknown() const override { return true; }
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, double, double, int, int) override;
};

// CCCS - Type F
class CCCS : public Component {
private:
    std::string ctrlCompName;
    double gain;
public:
    CCCS(const std::string& n, int n1, int n2, const std::string& ctrlComp, double gain);
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, double, double, int, int) override;
};
// -------------------------------- Component Class and Its Implementations --------------------------------

#endif // COMPONENT_H