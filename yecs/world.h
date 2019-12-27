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

#include <cassert>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <vector>

#include "yecs/common.h"
#include "yecs/component_collection.h"
#include "yecs/system.h"

namespace yecs
{
/** \brief Helper class for easier entities construction.
 *
 * World returns entity builder as a result of CreateEntity method, which
 * conviniently allows to chain calls, like CreateEntity().AddComponent<Physics>().AddComponent<Graphics>().Build();
 **/
class EntityBuilder
{
public:
    // Copying is forbidden.
    EntityBuilder(EntityBuilder&) = delete;
    EntityBuilder& operator=(EntityBuilder&) = delete;

    // Add component of a given type.
    template <typename ComponentT>
    EntityBuilder& AddComponent();

    // Build entity (return its id).
    Entity Build() const { return entity_; }

private:
    // Only World can create entity builders.
    EntityBuilder(Entity entity, World& world) : entity_(entity), world_(world) {}

    // Entity of interest.
    Entity entity_ = kInvalidEntity;
    // Reference to world to add components.
    World& world_;

    friend class World;
};

/** \brief Provides primary ECS interface for clients.
 *
 * World is a host for all ECS data and provides an interface to all ECS clients.
 * World is capable of:
 *  - Registering components.
 *  - Registering systems.
 *  - Creating entities.
 *  - Running systems.
 **/
class World
{
public:
    World()  = default;
    ~World() = default;

    /** \brief Register component type.
     * An attempt to add a component of unregistered type to an entity leads to an exception being thrown.*/
    template <typename ComponentT>
    void RegisterComponent();

    // Register a system.
    template <typename SystemT, typename... Args>
    void RegisterSystem(Args&&... args);

    /** \brief Create new entity.
     *  Method returns a builder instance, allowing to chain calls adding components.*/
    EntityBuilder CreateEntity();

    // Release all entity data.
    void DestroyEntity(Entity entity);

    // Add component for an entity.
    template <typename ComponentT>
    ComponentT& AddComponent(Entity entity);

    // Get component for an entity.
    template <typename ComponentT>
    ComponentT& GetComponent(Entity entity);

    // Check if an entity has a component.
    template <typename ComponentT>
    bool HasComponent(Entity entity) const;

    // Run systems.
    void Run();

    // Wipe out all the component and systems.
    void Reset();

private:
    // Each time entity space is out, we extend an array by this number of elements.
    static constexpr std::uint32_t kEntitySizeIncrement = 128;

    // Entity array: true if entity exists, false if not.
    std::mutex        entity_mutex_;
    std::vector<bool> entities_;
    // Component arrays.
    std::mutex                                                                    component_mutex_;
    std::unordered_map<std::type_index, std::unique_ptr<ComponentCollectionBase>> components_;
    // Systems.
    std::mutex                           system_mutex_;
    std::vector<std::unique_ptr<System>> systems_;
};

template <typename ComponentT>
inline void World::RegisterComponent()
{
    std::lock_guard<std::mutex> lock(component_mutex_);

    auto index = GetTypeIndex<ComponentT>();
    if (components_.find(index) != components_.cend())
    {
        throw std::runtime_error("ECSManager: Component type already registered");
    }

    components_.emplace(index, std::make_unique<ComponentCollection<ComponentT>>());
}

template <typename ComponentT>
inline ComponentT& World::AddComponent(Entity entity)
{
    std::lock_guard<std::mutex> lock(component_mutex_);

    auto index = GetTypeIndex<ComponentT>();
    if (components_.find(index) == components_.cend())
    {
        throw std::runtime_error("ECSManager: Component type not registered");
    }

    auto collection = static_cast<ComponentCollection<ComponentT>*>(components_[index].get());
    return collection->AddComponent(entity);
}

template <typename ComponentT>
inline ComponentT& World::GetComponent(Entity entity)
{
    auto index = GetTypeIndex<ComponentT>();
    if (components_.find(index) == components_.cend())
    {
        throw std::runtime_error("ECSManager: Component type not registered");
    }

    auto collection = static_cast<ComponentCollection<ComponentT>*>(components_[index].get());
    return collection->GetComponent(entity);
}

template <typename SystemT, typename... Args>
inline void World::RegisterSystem(Args&&... args)
{
    std::lock_guard<std::mutex> lock(system_mutex_);

    systems_.push_back(std::make_unique<SystemT>(std::forward<Args>(args)...));
}

void World::Run()
{
    for (auto& system : systems_) { system->Run(*this); }
}

void World::Reset()
{
    entities_.clear();
    components_.clear();
    systems_.clear();
}

EntityBuilder World::CreateEntity()
{
    std::lock_guard<std::mutex> lock(entity_mutex_);

    auto id = kInvalidEntity;

    // Look for free entity in an existing array.
    for (auto i = 0; i < entities_.size(); ++i)
    {
        if (!entities_[i])
        {
            id = i;
        }
    }

    // If it is full, resize and add new entities.
    if (id == kInvalidEntity)
    {
        // Add one entity at the end.
        auto old_size = entities_.size();
        id            = old_size;

        // Set added elements to false (no entity).
        entities_.resize(old_size + kEntitySizeIncrement);
        std::fill(entities_.begin() + old_size, entities_.end(), false);
    }

    // Mark entity as existing.
    entities_[id] = true;
    return EntityBuilder(id, *this);
}

void World::DestroyEntity(Entity entity)
{
    std::lock_guard<std::mutex> component_lock(component_mutex_);
    std::lock_guard<std::mutex> entity_lock(entity_mutex_);

    for (auto& components : components_)
    {
        if (components.second->HasComponent(entity))
        {
            components.second->RemoveComponent(entity);
        }
    }

    entities_[entity] = false;
}

template <typename ComponentT>
inline EntityBuilder& EntityBuilder::AddComponent()
{
    world_.AddComponent<ComponentT>(entity_);
    return *this;
}

}  // namespace yecs
