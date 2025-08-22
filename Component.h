#ifndef COMPONENT_H
#define COMPONENT_H

#include <Eigen/Dense>
#include <QString>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <fstream>
#include <qdatastream.h>
#include <cereal/types/polymorphic.hpp>
#include "Serialization.h"

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

    Component();
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

    // virtual void save_binary(QDataStream& out) const;
    // virtual void load_binary(QDataStream& in);

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(type), CEREAL_NVP(name), CEREAL_NVP(node1), CEREAL_NVP(node2), CEREAL_NVP(value));
    }
};

class Resistor : public Component {
public:
    Resistor();

    Resistor(const std::string& n, int n1, int n2, double v);
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &,const std::map<int, int>& nodeIdToMnaIndex,  double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    // void save_binary(QDataStream &out) const override;
    // void load_binary(QDataStream &in) override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Component>(this));
    }
};

class Capacitor : public Component {
private:
    double V_prev;
public:

    Capacitor();

    Capacitor(const std::string& n, int n1, int n2, double v);
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) override;
    void reset() override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    // void save_binary(QDataStream &out) const override;
    // void load_binary(QDataStream &in) override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(V_prev));
    }
};

class Inductor : public Component {
private:
    double I_prev;
public:
    Inductor();

    Inductor(const std::string& n, int n1, int n2, double v);
    bool needsCurrentUnknown() const override { return true; }
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) override;
    void reset() override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &,const std::map<int, int>& nodeIdToMnaIndex,  double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    // void save_binary(QDataStream &out) const override;
    // void load_binary(QDataStream &in) override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(I_prev));
    }
};

class Diode : public Component {
private:
    double Is;
    double Vt;
    double eta;
    double V_prev;
public:
    Diode();

    Diode(const std::string& n, int n1, int n2, double Is = 1e-12, double eta = 1.0, double Vt = 0.026);
    bool isNonlinear() const override { return true; }
    void updateState(const Eigen::VectorXd& solution, const std::map<std::string, int>& ci, const std::map<int, int>& nodeIdToMnaIndex) override;
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;
    void setPreviousVoltage(double v) { V_prev = v; }
    void reset() override;

    // void save_binary(QDataStream &out) const override;
    // void load_binary(QDataStream &in) override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(Is), CEREAL_NVP(Vt), CEREAL_NVP(eta), CEREAL_NVP(V_prev));
    }
};

class VoltageSource : public Component {
public:
    enum class SourceType {DC, Sinusoidal};
private:
    SourceType sourceType;
    double param1, param2, param3;
public:
    VoltageSource();
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

    // void save_binary(QDataStream &out) const override;
    // void load_binary(QDataStream &in) override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(sourceType), CEREAL_NVP(param1), CEREAL_NVP(param2), CEREAL_NVP(param3));
    }
};

// AC voltage source
class ACVoltageSource : public Component {
public:
    ACVoltageSource();

    ACVoltageSource(const std::string& name, int node1, int node2);

    bool needsCurrentUnknown() const override { return true; }
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;
    double getValueAtFrequency(double omega) const;
};

class CurrentSource : public Component {
public:
    enum class SourceType {DC, Sinusoidal};
private:
    SourceType sourceType;
    double param1, param2, param3;
public:
    CurrentSource();
    CurrentSource(const std::string& n, int n1, int n2, SourceType type, double p1, double p2, double p3);

    SourceType getSourceType() const { return sourceType; }
    double getParam1() const { return param1; }
    double getParam2() const { return param2; }
    double getParam3() const { return param3; }

    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;
    void setValue(double v);
    double getCurrentValue(double time) const;

    // void save_binary(QDataStream &out) const override;
    // void load_binary(QDataStream &in) override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(sourceType), CEREAL_NVP(param1), CEREAL_NVP(param2), CEREAL_NVP(param3));
    }
};

// VCVS - Type E
class VCVS : public Component {
private:
    int ctrlNode1, ctrlNode2;
    double gain;
public:
    VCVS();
    VCVS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain);

    int getCtrlNode1() const {return ctrlNode1;}
    int getCtrlNode2() const {return ctrlNode2;}
    double getGain() const {return gain;}

    bool needsCurrentUnknown() const override { return true; }
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    // void save_binary(QDataStream &out) const override;
    // void load_binary(QDataStream &in) override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(ctrlNode1), CEREAL_NVP(ctrlNode2), CEREAL_NVP(gain));
    }
};

// VCCS - Type G
class VCCS : public Component {
private:
    int ctrlNode1, ctrlNode2;
    double gain;
public:
    VCCS();
    VCCS(const std::string& n, int n1, int n2, int ctrlN1, int ctrlN2, double gain);

    int getCtrlNode1() const {return ctrlNode1;}
    int getCtrlNode2() const {return ctrlNode2;}
    double getGain() const {return gain;}

    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>& nodeIdToMnaIndex, double, double , int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    // void save_binary(QDataStream &out) const override;
    // void load_binary(QDataStream &in) override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(ctrlNode1), CEREAL_NVP(ctrlNode2), CEREAL_NVP(gain));
    }
};

// CCVS - Type H
class CCVS : public Component {
private:
    std::string ctrlCompName;
    double gain;
    int sourceIndex;
public:
    CCVS();
    CCVS(const std::string& n, int n1, int n2, const std::string& ctrlComp, double gain);

    std::string getCtrlCompName() const {return ctrlCompName;}
    double getGain() const {return gain;}
    int getSourceIndex() const {return sourceIndex;}

    bool needsCurrentUnknown() const override { return true; }
    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    // void save_binary(QDataStream &out) const override;
    // void load_binary(QDataStream &in) override;
};

// CCCS - Type F
class CCCS : public Component {
private:
    std::string ctrlCompName;
    double gain;
public:
    CCCS();
    CCCS(const std::string& n, int n1, int n2, const std::string& ctrlComp, double gain);

    std::string getCtrlCompName() const {return ctrlCompName;}
    double getGain() const {return gain;}

    void stampMNA(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int> &, const std::map<int, int>& nodeIdToMnaIndex, double, double, int) override;
    void stampMNA_AC(Eigen::MatrixXd&, Eigen::VectorXd&, const std::map<std::string, int>&, const std::map<int, int>&, double, int) override;

    // void save_binary(QDataStream &out) const override;
    // void load_binary(QDataStream &in) override;
};
// -------------------------------- Component Class and Its Implementations --------------------------------


CEREAL_REGISTER_TYPE(Resistor)
CEREAL_REGISTER_TYPE(Capacitor)
CEREAL_REGISTER_TYPE(Inductor)
CEREAL_REGISTER_TYPE(VoltageSource)
CEREAL_REGISTER_TYPE(CurrentSource)
CEREAL_REGISTER_TYPE(Diode)
CEREAL_REGISTER_TYPE(CCCS)
CEREAL_REGISTER_TYPE(CCVS)
CEREAL_REGISTER_TYPE(VCCS)
CEREAL_REGISTER_TYPE(VCVS)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, Resistor)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, Capacitor)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, Inductor)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, VoltageSource)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, CurrentSource)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, Diode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, CCVS)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, VCVS)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, CCCS)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, VCCS)

#endif // COMPONENT_H