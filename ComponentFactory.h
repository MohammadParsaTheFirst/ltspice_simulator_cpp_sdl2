//
// Created by parsa on 8/5/2025.
//

#ifndef COMPONENTFACTORY_H
#define COMPONENTFACTORY_H

#include "Component.h"
#include "Circuit.h"

class ComponentFactory {
public:
    static Component* createComponent(
        const std::string& typeStr,
        const std::string& name,
        int n1_id,
        int n2_id,
        double value,
        const std::vector<double>& numericParams,
        const std::vector<std::string>& stringParams,
        bool isSinusoidal,
        Circuit* circuit
    );

    static std::shared_ptr<Component> createComponentFromType(const QString& typeStr);
};

#endif //COMPONENTFACTORY_H
