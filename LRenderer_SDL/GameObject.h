#pragma once
#include <vector>
#include <unordered_map>
#include <typeindex>

#include "Pool.h"
#include "Component.h"

class GameObject
{
public:
	int sceneIndex = -1;

    template<typename T>
    void AddComponent(T* component)
    {
        static_assert(std::is_base_of_v<Component, T>);

        auto type = std::type_index(typeid(T));
        components.insert(make_pair(type, component));
    }

    template<typename T>
    T* GetComponent()
    {
        static_assert(std::is_base_of_v<Component, T>);

        auto type = std::type_index(typeid(T));
        auto it = components.find(type);
        if (it == components.end()) {
            return NULL;
        }

        return (T *)it->second;
    }

private:
    std::unordered_multimap<std::type_index, Component *> components;
};

