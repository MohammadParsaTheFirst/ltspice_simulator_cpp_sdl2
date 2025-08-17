#ifndef COMPONENT_H
#define COMPONENT_H

#include <Eigen/Dense>
#include <cereal/types/memory.hpp>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/access.hpp>

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

    template<class Archive>
    void save(Archive& ar) const {
        ar(type, name, node1, node2, value);
    }

    std::string getName() const {
        return name;
    }
};


class Resistor : public Component {
public:
    Resistor(const std::string& n, int n1, int n2, double v);
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &,const std::map<int, int>& nodeIdToMnaIndex,  double, double, int) override;

    template<class Archive>
    void save(Archive& ar) const {
        ar(cereal::base_class<Component>(this));
    }

    template<class Archive>
    static void load_and_construct(Archive& ar, cereal::construct<Resistor>& construct) {
        Component::Type type;
        std::string name;
        int n1, n2;
        double v;
        ar(type, name, n1, n2, v);
        construct(name, n1, n2, v);
    }
};


class Capacitor : public Component {
private:
    double V_prev;
public:
    Capacitor(const std::string& n, int n1, int n2, double v);
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) override;
    void reset() override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;

    template<class Archive>
    void save(Archive& ar) const {
        ar(cereal::base_class<Component>(this), V_prev);
    }

    template <class Archive>
    static void load_and_construct(Archive& ar, cereal::construct<Capacitor>& construct) {
        Component::Type type;
        std::string name;
        int n1, n2;
        double v;
        double v_prev;
        ar(type, name, n1, n2, v, v_prev);
        construct(name, n1, n2, v);
        construct->V_prev = v_prev;
    }
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

    template<class Archive>
    void save(Archive& ar) const {
        ar(cereal::base_class<Component>(this), I_prev);
    }

    template <class Archive>
    static void load_and_construct(Archive& ar, cereal::construct<Inductor>& construct) {
        Component::Type type;
        std::string name;
        int n1, n2;
        double v;
        double i_prev;
        ar(type, name, n1, n2, v, i_prev);
        construct(name, n1, n2, v);
        construct->I_prev = i_prev;
    }
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

    template<class Archive>
    void save(Archive& ar) const {
        ar(cereal::base_class<Component>(this), Is, Vt, eta, V_prev);
    }

    template <class Archive>
    static void load_and_construct(Archive& ar, cereal::construct<Diode>& construct) {
        Component::Type type;
        std::string name;
        int n1, n2;
        double v;
        double Is, Vt, eta, V_prev;
        ar(type, name, n1, n2, v, Is, Vt, eta, V_prev);
        construct(name, n1, n2, Is, eta, Vt);
        construct->V_prev = V_prev;
    }
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

    template<class Archive>
    void save(Archive& ar) const {
        ar(cereal::base_class<Component>(this), sourceType, param1, param2, param3);
    }

    template <class Archive>
    static void load_and_construct(Archive& ar, cereal::construct<VoltageSource>& construct) {
        Component::Type type;
        std::string name;
        int n1, n2;
        double v;
        SourceType st;
        double p1, p2, p3;
        ar(type, name, n1, n2, v, st, p1, p2, p3);
        construct(name, n1, n2, st, p1, p2, p3);
    }
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

    template<class Archive> 
    void save(Archive& ar) const {
        ar(cereal::base_class<Component>(this), sourceType, param1, param2, param3);
    }

    template <class Archive>
    static void load_and_construct(Archive& ar, cereal::construct<CurrentSource>& construct) {
        Component::Type type;
        std::string name;
        int n1, n2;
        double v;
        SourceType st;
        double p1, p2, p3;
        ar(type, name, n1, n2, v, st, p1, p2, p3);
        construct(name, n1, n2, st, p1, p2, p3);
    }
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

    template<class Archive> 
    void save(Archive& ar) const {
        ar(cereal::base_class<Component>(this), ctrlNode1, ctrlNode2, gain);
    }

    template <class Archive>
    static void load_and_construct(Archive& ar, cereal::construct<VCVS>& construct) {
        Component::Type type;
        std::string name;
        int n1, n2;
        double v;
        int ctrlnode1, ctrlnode2;
        double gain;
        ar(type, name, n1, n2, v, ctrlnode1, ctrlnode2, gain);
        construct(name, n1, n2, ctrlnode1, ctrlnode2, gain);
    }
};


// VCCS - Type G
class VCCS : public Component {
private:
    int ctrlNode1, ctrlNode2;
    double gain;
public:
    VCCS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain);
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>& nodeIdToMnaIndex, double, double , int) override;

    template<class Archive> 
    void save(Archive& ar) const {
        ar(cereal::base_class<Component>(this), ctrlNode1, ctrlNode2, gain);
    }

    template <class Archive>
    static void load_and_construct(Archive& ar, cereal::construct<VCCS>& construct) {
        Component::Type type;
        std::string name;
        int n1, n2;
        double v;
        int ctrlnode1, ctrlnode2;
        double gain;
        ar(type, name, n1, n2, v, ctrlnode1, ctrlnode2, gain);
        construct(name, n1, n2, ctrlnode1, ctrlnode2, gain);
    }
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

    template<class Archive> 
    void save(Archive& ar) const {
        ar(cereal::base_class<Component>(this), ctrlCompName, gain);
    }

    template <class Archive>
    static void load_and_construct(Archive& ar, cereal::construct<CCVS>& construct) {
        Component::Type type;
        std::string name;
        int n1, n2;
        double v;
        std::string ctrlcompname;
        double gain;
        ar(type, name, n1, n2, v, ctrlcompname, gain);
        construct(name, n1, n2, ctrlcompname, gain);
    }
};


// CCCS - Type F
class CCCS : public Component {
private:
    std::string ctrlCompName;
    double gain;
public:
    CCCS(const std::string& n, int n1, int n2, const std::string& ctrlComp, double gain);
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;

    template<class Archive> 
    void save(Archive& ar) const {
        ar(cereal::base_class<Component>(this), ctrlCompName, gain);
    }

    template <class Archive>
    static void load_and_construct(Archive& ar, cereal::construct<CCCS>& construct) {
        Component::Type type;
        std::string name;
        int n1, n2;
        double v;
        std::string ctrlcompname;
        double gain;
        ar(type, name, n1, n2, v, ctrlcompname, gain);
        construct(name, n1, n2, ctrlcompname, gain);
    }
};
// -------------------------------- Component Class and Its Implementations --------------------------------

#endif // COMPONENT_H