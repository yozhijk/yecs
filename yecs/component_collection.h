/****************************************************************************
MIT License

Copyright (c) 2019 Dmitry Kozlov (dmitry.a.kozlov@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************/
#pragma once

#include <unordered_map>
#include <vector>

#include "yecs/common.h"

namespace yecs
{
class ComponentCollectionBase
{
public:
    virtual ~ComponentCollectionBase() = 0;

    // Collection size
    virtual size_t size() const = 0;

    // True if entity has a component in this collection
    virtual bool HasComponent(Entity entity) const = 0;

    // Create component.
    virtual ComponentIndex CreateComponent() = 0;

    // Remove component from entity.
    virtual void RemoveComponent(Entity entity) = 0;

    // Set component by index.
    virtual void SetComponent(Entity entity, ComponentIndex index) = 0;
};

template <typename T>
class ComponentCollection : public ComponentCollectionBase
{
public:
    ComponentCollection()           = default;
    ~ComponentCollection() override = default;

    ComponentCollection(const ComponentCollection&) = delete;
    ComponentCollection& operator=(const ComponentCollection&) = delete;

    ComponentCollection(ComponentCollection&&);
    ComponentCollection&& operator=(ComponentCollection&&);

    // Get collection size.
    size_t size() const override { return components_.size(); }

    // True if entity has a component in this collection.
    bool HasComponent(Entity entity) const override;

    // Create component and return its index.
    ComponentIndex CreateComponent() override;

    // Remove component from entity.
    void RemoveComponent(Entity entity) override;

    // Set component by index.
    void SetComponent(Entity entity, ComponentIndex index) override;

    // Get component for entity, throws std::runtime_error if
    // HasComponent(entity) == false.
    T&       GetComponent(Entity entity);
    const T& GetComponent(Entity entity) const;

    // Add a component to an entity.
    T& AddComponent(Entity entity);

    // Access component by index.
    T&       operator[](ComponentIndex index);
    const T& operator[](ComponentIndex index) const;

private:
    std::unordered_map<Entity, ComponentIndex> component_index_;
    std::vector<T>                             components_;
};

inline ComponentCollectionBase::~ComponentCollectionBase() {}

template <typename T>
inline ComponentCollection<T>::ComponentCollection(ComponentCollection&& rhs)
    : component_index_(std::move(rhs.component_index_)), components_(std::move(rhs.components_))
{
}

template <typename T>
inline ComponentCollection<T>&& ComponentCollection<T>::operator=(ComponentCollection&& rhs)
{
    component_index_ = std::move(rhs.component_index_);
    components_      = std::move(rhs.components_);
}

template <typename T>
inline bool ComponentCollection<T>::HasComponent(Entity entity) const
{
    return component_index_.find(entity) != component_index_.cend();
}

template <typename T>
inline T& ComponentCollection<T>::AddComponent(Entity entity)
{
    if (HasComponent(entity))
    {
        throw std::runtime_error("ComponentCollection: Entity already has a component");
    }

    component_index_[entity] = components_.size();
    components_.emplace_back();
    return components_.back();
}

template <typename T>
inline ComponentIndex ComponentCollection<T>::CreateComponent()
{
    components_.emplace_back();
    return components_.size() - 1;
}

template <typename T>
inline void ComponentCollection<T>::SetComponent(Entity entity, ComponentIndex index)
{
    if (HasComponent(entity))
    {
        throw std::runtime_error("ComponentCollection: Entity already has a component");
    }

    component_index_[entity] = index;
}

template <typename T>
inline const T& ComponentCollection<T>::GetComponent(Entity entity) const
{
    if (!HasComponent(entity))
    {
        throw std::runtime_error("ComponentCollection: Entity does not have a component");
    }

    ComponentIndex index = component_index_.find(entity)->second;
    return components_[index];
}

template <typename T>
inline T& ComponentCollection<T>::GetComponent(Entity entity)
{
    if (!HasComponent(entity))
    {
        throw std::runtime_error("ComponentCollection: Entity does not have a component");
    }

    ComponentIndex index = component_index_[entity];
    return components_[index];
}

template <typename T>
inline void ComponentCollection<T>::RemoveComponent(Entity entity)
{
    if (!HasComponent(entity))
    {
        throw std::runtime_error("ComponentCollection: Entity does not have a component");
    }

    if (components_.size() > 1)
    {
        ComponentIndex last_index = components_.size() - 1;
        ComponentIndex index      = component_index_[entity];

        std::swap(components_[index], components_[last_index]);

        for (auto& ci : component_index_)
        {
            if (ci.second == last_index)
            {
                ci.second = index;
                break;
            }
        }
    }

    component_index_.erase(entity);
    components_.resize(components_.size() - 1);
}

template <typename T>
inline T& ComponentCollection<T>::operator[](ComponentIndex index)
{
    return components_[index];
}

template <typename T>
inline const T& ComponentCollection<T>::operator[](ComponentIndex index) const
{
    return components_[index];
}

}  // namespace yecs
