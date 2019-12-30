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

#include <assert.h>

#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <vector>

#include "third_party/cpp-taskflow/taskflow/taskflow.hpp"
#include "yecs/common.h"
#include "yecs/component_storage.h"
#include "yecs/component_types_builder.h"
#include "yecs/entity_query.h"
#include "yecs/entity_set.h"
#include "yecs/system.h"

namespace yecs
{
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

    // Set system execution ordering: execute System0 before System1.
    template <typename SystemT0, typename SystemT1>
    void Precede();

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

    // Get number of components of specified type.
    template <typename ComponentT>
    size_t GetNumComponents() const;

    // Direct access to components.
    template <typename ComponentT>
    ComponentT& GetComponentByIndex(ComponentIndex index);

    template <typename ComponentT>
    const ComponentT& GetComponentByIndex(ComponentIndex index) const;

    // Run systems.
    void Run();

    // Wipe out all the component and systems.
    void Reset();

private:
    template <typename ComponentT>
    auto& GetComponentCollection();

    // Each time entity space is out, we extend an array by this number of elements.
    static constexpr uint32_t kEntitySizeIncrement = 128;

    // Data associated with a system.
    struct SystemInvoke
    {
        tf::Task                task;
        std::unique_ptr<System> system;
    };

    using ComponetsBase = ComponentStorageBase;
    template <typename T>
    using Components    = DenseComponentStorage<T>;
    using ComponentsMap = std::unordered_map<std::type_index, std::unique_ptr<ComponentStorageBase>>;
    using SystemsMap    = std::unordered_map<std::type_index, SystemInvoke>;

    // Entity array: true if entity exists, false if not.
    std::mutex        entity_mutex_;
    std::vector<bool> entities_;
    // Component arrays.
    std::mutex    component_mutex_;
    ComponentsMap components_;
    // Systems.
    std::mutex system_mutex_;
    SystemsMap systems_;

    // Task flow stuff.
    tf::Taskflow taskflow_;
    tf::Executor executor_;

    friend class EntityQuery;
    friend class ComponentAccess;
};

class ComponentAccess
{
public:
    template <typename ComponentT>
    DenseComponentStorage<ComponentT>& Write();

    template <typename ComponentT>
    const DenseComponentStorage<ComponentT>& Read() const;

private:
    explicit ComponentAccess(World& world);

    World& world_;

    friend class World;
};

template <typename ComponentT>
inline auto& World::GetComponentCollection()
{
    auto index = GetTypeIndex<ComponentT>();
    assert(components_.find(index) != components_.cend());

    auto collection = static_cast<Components<ComponentT>*>(components_[index].get());
    return *collection;
}

template <typename ComponentT>
inline void World::RegisterComponent()
{
    std::lock_guard<std::mutex> lock(component_mutex_);

    auto index = GetTypeIndex<ComponentT>();
    if (components_.find(index) != components_.cend())
    {
        throw std::runtime_error("World: component type already registered.");
    }

    components_.emplace(index, std::make_unique<DenseComponentStorage<ComponentT>>());
}

template <typename ComponentT>
inline ComponentT& World::AddComponent(Entity entity)
{
    std::lock_guard<std::mutex> lock(component_mutex_);

    return GetComponentCollection<ComponentT>().AddComponent(entity);
}

template <typename ComponentT>
inline ComponentT& World::GetComponent(Entity entity)
{
    return GetComponentCollection<ComponentT>().GetComponent(entity);
}

// Direct component access.
template <typename ComponentT>
size_t World::GetNumComponents() const
{
    return GetComponentCollection<ComponentT>().size();
}

template <typename ComponentT>
ComponentT& World::GetComponentByIndex(ComponentIndex i)
{
    return GetComponentCollection<ComponentT>()[i];
}

template <typename ComponentT>
const ComponentT& World::GetComponentByIndex(ComponentIndex i) const
{
    return GetComponentCollection<ComponentT>()[i];
}

template <typename SystemT, typename... Args>
inline void World::RegisterSystem(Args&&... args)
{
    std::lock_guard<std::mutex> lock(system_mutex_);

    auto index = GetTypeIndex<SystemT>();

    if (systems_.find(index) != systems_.cend())
    {
        throw std::runtime_error("World: system type already registered");
    }

    auto system = std::make_unique<SystemT>(std::forward<Args>(args)...);

    SystemInvoke invoke;
    invoke.system = std::make_unique<SystemT>(std::forward<Args>(args)...);
    invoke.task   = taskflow_.emplace([system = invoke.system.get(), this]() {
        ComponentAccess access(*this);
        EntityQuery     query(*this);

        system->Run(access, query);
    });

    systems_.emplace(index, std::move(invoke));
}

template <typename ComponentT>
inline World::EntityBuilder& World::EntityBuilder::AddComponent()
{
    world_.AddComponent<ComponentT>(entity_);
    return *this;
}

inline ComponentAccess::ComponentAccess(World& world) : world_(world) {}

template <typename ComponentT>
inline DenseComponentStorage<ComponentT>& ComponentAccess::Write()
{
    return world_.GetComponentCollection<ComponentT>();
}

template <typename ComponentT>
inline const DenseComponentStorage<ComponentT>& ComponentAccess::Read() const
{
    return world_.GetComponentCollection<ComponentT>();
}

template <typename SystemT0, typename SystemT1>
inline void World::Precede()
{
    auto index0 = GetTypeIndex<SystemT0>();
    auto index1 = GetTypeIndex<SystemT1>();

    auto system0 = systems_.find(index0);
    auto system1 = systems_.find(index1);

    if (system0 == systems_.cend() || system1 == systems_.cend())
    {
        throw std::runtime_error("World: system type not found");
    }

    system0.second.task.Precede(system1.second.task);
}
}  // namespace yecs
