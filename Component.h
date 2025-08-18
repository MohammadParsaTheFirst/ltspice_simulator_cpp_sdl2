#ifndef COMPONENT_H
#define COMPONENT_H

#include <Eigen/Dense>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>

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
        const std::map<int, int>& nodeIdToMnaIndex, double time, double h, int idx) = 0;
    virtual void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) {}
    virtual bool isNonlinear() const { return false; }
    virtual bool needsCurrentUnknown() const { return false; }

    virtual void save_binary(std::ofstream& file) const;
    virtual void load_binary(std::ifstream& file);
};


class Resistor : public Component {
public:
    Resistor(const std::string& n, int n1, int n2, double v);
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &,const std::map<int, int>& nodeIdToMnaIndex,  double, double, int) override;
};


class Capacitor : public Component {
private:
    double V_prev;
public:
    Capacitor(const std::string& n, int n1, int n2, double v);
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) override;
    void reset() override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void save_binary(std::ofstream& file) const override;
    void load_binary(std::ifstream& file) override;
};


class Inductor : public Component {
private:
    double I_prev;
public:
    Inductor(const std::string& n, int n1, int n2, double v);
    bool needsCurrentUnknown() const override { return true; }
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) override;
    void reset() override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &,const std::map<int, int>& nodeIdToMnaIndex,  double, double, int) override;
    void save_binary(std::ofstream& file) const override;
    void load_binary(std::ifstream& file) override;
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
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void setPreviousVoltage(double v) { V_prev = v; }
    void reset() override;
    void save_binary(std::ofstream& file) const override;
    void load_binary(std::ifstream& file) override;
};


class VoltageSource : public Component {
public:
    enum class SourceType {DC, Sinusoidal};
private:
    SourceType sourceType;
    double param1, param2, param3;
public:
    VoltageSource(const std::string& name, int node1, int node2, SourceType type, double p1, double p2, double p3);

    bool needsCurrentUnknown() const override { return true; }
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void setValue(double v);
    double getCurrentValue(double time) const;
    void save_binary(std::ofstream& file) const override;
    void load_binary(std::ifstream& file) override;
};


class CurrentSource : public Component {
public:
    enum class SourceType {DC, Sinusoidal};
private:
    SourceType sourceType;
    double param1, param2, param3;
public:
    CurrentSource(const std::string& n, int n1, int n2, SourceType type, double p1, double p2, double p3);
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void setValue(double v);
    double getCurrentValue(double time) const;
    void save_binary(std::ofstream& file) const override;
    void load_binary(std::ifstream& file) override;
};


// VCVS - Type E
class VCVS : public Component {
private:
    int ctrlNode1, ctrlNode2;
    double gain;
public:
    VCVS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain);
    bool needsCurrentUnknown() const override { return true; }
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void save_binary(std::ofstream& file) const override;
    void load_binary(std::ifstream& file) override;
};


// VCCS - Type G
class VCCS : public Component {
private:
    int ctrlNode1, ctrlNode2;
    double gain;
public:
    VCCS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain);
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>& nodeIdToMnaIndex, double, double , int) override;
    void save_binary(std::ofstream& file) const override;
    void load_binary(std::ifstream& file) override;
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
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void save_binary(std::ofstream& file) const override;
    void load_binary(std::ifstream& file) override;
};


// CCCS - Type F
class CCCS : public Component {
private:
    std::string ctrlCompName;
    double gain;
public:
    CCCS(const std::string& n, int n1, int n2, const std::string& ctrlComp, double gain);
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void save_binary(std::ofstream& file) const override;
    void load_binary(std::ifstream& file) override;
};
// -------------------------------- Component Class and Its Implementations --------------------------------

#endif // COMPONENT_H