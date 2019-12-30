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
/** \brief Component storage interface.
 *
 * This class is mainly used for type erasure at this point, since DenseComponentStorage is
 * hardcoded in many places, but might be useful for different component implementations in the future.
 **/
class ComponentStorageBase
{
public:
    virtual ~ComponentStorageBase() = 0;

    // Collection size
    virtual size_t size() const = 0;

    // True if entity has a component in this collection
    virtual bool HasComponent(Entity entity) const = 0;

    // Remove component from entity.
    virtual void RemoveComponent(Entity entity) = 0;
};

/** \brief Component storage storing entities in a dense array.
 *
 * Components are stored in dense array and hash map is being used for entity to component mapping.
 **/
template <typename T>
class DenseComponentStorage : public ComponentStorageBase
{
public:
    DenseComponentStorage()           = default;
    ~DenseComponentStorage() override = default;

    DenseComponentStorage(const DenseComponentStorage&) = delete;
    DenseComponentStorage& operator=(const DenseComponentStorage&) = delete;

    DenseComponentStorage(DenseComponentStorage&&);
    DenseComponentStorage&& operator=(DenseComponentStorage&&);

    // Get collection size.
    size_t size() const override { return components_.size(); }

    // True if entity has a component in this collection.
    bool HasComponent(Entity entity) const override;

    // Remove component from entity.
    void RemoveComponent(Entity entity) override;

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

inline ComponentStorageBase::~ComponentStorageBase() {}

template <typename T>
inline DenseComponentStorage<T>::DenseComponentStorage(DenseComponentStorage&& rhs)
    : component_index_(std::move(rhs.component_index_)), components_(std::move(rhs.components_))
{
}

template <typename T>
inline DenseComponentStorage<T>&& DenseComponentStorage<T>::operator=(DenseComponentStorage&& rhs)
{
    component_index_ = std::move(rhs.component_index_);
    components_      = std::move(rhs.components_);
}

template <typename T>
inline bool DenseComponentStorage<T>::HasComponent(Entity entity) const
{
    return component_index_.find(entity) != component_index_.cend();
}

template <typename T>
inline T& DenseComponentStorage<T>::AddComponent(Entity entity)
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
inline const T& DenseComponentStorage<T>::GetComponent(Entity entity) const
{
    if (!HasComponent(entity))
    {
        throw std::runtime_error("ComponentCollection: Entity does not have a component");
    }

    ComponentIndex index = component_index_.find(entity)->second;
    return components_[index];
}

template <typename T>
inline T& DenseComponentStorage<T>::GetComponent(Entity entity)
{
    if (!HasComponent(entity))
    {
        throw std::runtime_error("ComponentCollection: Entity does not have a component");
    }

    ComponentIndex index = component_index_[entity];
    return components_[index];
}

template <typename T>
inline void DenseComponentStorage<T>::RemoveComponent(Entity entity)
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
inline T& DenseComponentStorage<T>::operator[](ComponentIndex index)
{
    return components_[index];
}

template <typename T>
inline const T& DenseComponentStorage<T>::operator[](ComponentIndex index) const
{
    return components_[index];
}
}  // namespace yecs
