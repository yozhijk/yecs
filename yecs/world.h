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

// Disable warning as error for VS2019 build, taskflow has mutliple type conversion producing warning.
// As of Jan 4 2020, there is a pending pull request for that: https://github.com/cpp-taskflow/cpp-taskflow/pull/135
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4267)
#endif
#include "third_party/cpp-taskflow/taskflow/taskflow.hpp"
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "yecs/common.h"
#include "yecs/component_storage.h"
#include "yecs/component_types_builder.h"
#include "yecs/entity_query.h"
#include "yecs/entity_set.h"
#include "yecs/system.h"

namespace yecs
{
/**
 * @brief Provides primary ECS interface for clients.
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
    /**
     * @brief Helper class for easier entities construction.
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
        Entity Build() const noexcept { return entity_; }

    private:
        // Only World can create entity builders.
        EntityBuilder(Entity entity, World& world) noexcept : entity_(entity), world_(world) {}

        // Entity of interest.
        Entity entity_ = kInvalidEntity;
        // Reference to world to add components.
        World& world_;

        friend class World;
    };

public:
    World()  = default;
    ~World() = default;

    /**
     * @brief Register component type.
     *
     * An attempt to add a component of unregistered type to an entity leads to an exception being thrown.
     *
     * @tparam ComponentT The type of a component.
     * @tparam StorageT Optional component storage type.
     **/
    template <typename ComponentT, typename StorageT = DenseComponentStorage<ComponentT>>
    void RegisterComponent();

    /**
     * @brief Register a system.
     *
     * Registering a system essentially mean adding it into a list of systems called by World.
     * This internally creates an instance of a SystemT passing user supplied Args to its constructor.
     * Systems are indexed by their type, meaning it is not possible to have two systems of the same type
     * in the World.
     *
     * @tparam SystemT The type of a system.
     * @tparam Args Constructor argument types.
     *
     * @param args Actual list of constructor arguments.
     **/
    template <typename SystemT, typename... Args>
    void RegisterSystem(Args&&... args);

    /**
     * @brief Make one system to run before another one.
     *
     * By default systems can execute in arbitrary order (or even in parallel). Precede sets an order of system
     * execution.
     *
     * @tparam SystemT0 The system to execute before SystemT1.
     * @tparam SystemT1 The system to execute after SystemT0.
     **/
    template <typename SystemT0, typename SystemT1>
    void Precede();

    /**
     * @brief Create new entity.
     *
     * This method creates an empty entity and returns a builder instances which can be used to add components:
     * world.CreateEntity().AddComponent<Velocity>().AddComponent<Mass>().Build().
     *
     * @return New EntityBuilder instances.
     **/
    EntityBuilder CreateEntity();

    /**
     * @brief Destroy an entity.
     *
     * Destroys an entity along with its components.
     *
     * @param entity Entity to destroy,
     **/
    void DestroyEntity(Entity entity);

    /**
     * @brief Add component to an entity.
     *
     * Add component of a given type to an entity. A type should be registered in the World,
     * otherwise std::runtime_error is being thrown.
     *
     * @tparam ComponentT Component type to add.
     * @param entity Entity to add ComponentT component to.
     *
     * @return Ref to a component.
     * @throw std::runtime_error
     **/
    template <typename ComponentT>
    ComponentT& AddComponent(Entity entity);

    /**
     * @brief Get ref to a component of a given type.
     *
     * Get a ref to a component for an entity. A type should be registered in the World and
     * an entity should have ComponentT component, otherwise std::runtime_error is being thrown.
     *
     * @tparam ComponentT Component type.
     * @param entity Entity to get a component for.
     *
     * @return Ref to a component.
     * @throw std::runtime_error
     **/
    // Get component for an entity.
    template <typename ComponentT>
    ComponentT& GetComponent(Entity entity);

    /**
     * @brief Check if an entity has a component of a given type.
     *
     * Get a ref to a component for an entity. A type should be registered in the World and
     * otherwise std::runtime_error is being thrown.
     *
     * @tparam ComponentT Component type.
     * @param entity Entity to check a component for.
     *
     * @return true if entity has a component, false otherwise.
     * @throw std::runtime_error
     **/
    template <typename ComponentT>
    bool HasComponent(Entity entity) const;

    /**
     * @brief Get total number of components of a given type.
     *
     * A type should be registered in the World and otherwise std::runtime_error is being thrown.
     *
     * @tparam ComponentT Component type.
     *
     * @return Number of components of type ComponentT.
     * @throw std::runtime_error
     **/
    template <typename ComponentT>
    size_t GetNumComponents() const;

    /**
     * @brief Get a ref to a component given its index.
     *
     * A type should be registered in the World and otherwise std::runtime_error is being thrown.
     * index should be < than GetNumComponents<ComponentT>()
     *
     * @tparam ComponentT Component type.
     *
     * @return Ref to a component of type ComponentT at index.
     * @throw std::runtime_error
     **/
    template <typename ComponentT>
    ComponentT& GetComponentByIndex(ComponentIndex index);

    /**
     * @brief Get a const ref to a component given its index.
     *
     * A type should be registered in the World and otherwise std::runtime_error is being thrown.
     * index should be < than GetNumComponents<ComponentT>()
     *
     * @tparam ComponentT Component type.
     *
     * @return Const ref to a component of type ComponentT at index.
     * @throw std::runtime_error
     **/
    template <typename ComponentT>
    const ComponentT& GetComponentByIndex(ComponentIndex index) const;

    /**
     * @brief Run one step of a simulation.
     *
     * During one step of a simulation each registered system is called exactly once respecting execution order
     * constratints specified by the user.
     *
     * @tparam ComponentT Component type.
     *
     * @throw std::runtime_error
     **/
    void Run();

    /**
     * @brief Wipe out all the component and systems, return World to its initial state as if nothing
     * has been registered and executed.
     **/
    void Reset();

    /**
     * @brief Get a reference to a system.
     **/
    template <typename SystemT>
    SystemT& GetSystem();

private:
    // Get reference to a component storage of a specified type.
    // If type is not registered, throws std::runtime_error.
    template <typename ComponentT, typename StorageT = DenseComponentStorage<ComponentT>>
    StorageT& GetComponentStorage();

    // Each time entity space is out, we extend an array by this number of elements.
    static constexpr uint32_t kEntitySizeIncrement = 128;

    // Data associated with a system.
    struct SystemInvoke
    {
        tf::Task                task;
        std::unique_ptr<System> system;
    };

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

/**
 * @brief An interface providing access to components for System subclasses.
 *
 * ComponentAccess's single purpose is to serve as a medium between World and System subclasses,
 * guarding world from unattended access.
 **/
class ComponentAccess
{
public:
    /**
     * @brief Request component storage for write access.
     *
     * @tparam ComponentT The type of the component needed.
     * @tparam StorageT Optional storage type.
     *
     * @return Reference to component storage.
     **/
    template <typename ComponentT, typename StorageT = DenseComponentStorage<ComponentT>>
    StorageT& Write();

    /**
     * @brief Request component storage for read access.
     *
     * @tparam ComponentT The type of the component needed.
     * @tparam StorageT Optional storage type.
     *
     * @return Const reference to component storage.
     **/
    template <typename ComponentT, typename StorageT = DenseComponentStorage<ComponentT>>
    const StorageT& Read() const;

private:
    // Only world can create these objects.
    explicit ComponentAccess(World& world) noexcept;

    // Reference to our world object.
    World& world_;

    friend class World;
};

template <typename ComponentT, typename StorageT>
inline StorageT& World::GetComponentStorage()
{
    auto index = GetTypeIndex<ComponentT>();
    assert(components_.find(index) != components_.cend());

    auto storage = static_cast<StorageT*>(components_[index].get());
    return *storage;
}

template <typename ComponentT, typename StorageT>
inline void World::RegisterComponent()
{
    std::lock_guard<std::mutex> lock(component_mutex_);

    auto index = GetTypeIndex<ComponentT>();
    if (components_.find(index) != components_.cend())
    {
        throw std::runtime_error("World: component type already registered.");
    }

    components_.emplace(index, std::make_unique<StorageT>());
}

template <typename ComponentT>
inline ComponentT& World::AddComponent(Entity entity)
{
    std::lock_guard<std::mutex> lock(component_mutex_);

    return GetComponentStorage<ComponentT>().AddComponent(entity);
}

template <typename ComponentT>
inline ComponentT& World::GetComponent(Entity entity)
{
    return GetComponentStorage<ComponentT>().GetComponent(entity);
}

// Direct component access.
template <typename ComponentT>
size_t World::GetNumComponents() const
{
    return GetComponentStorage<ComponentT>().size();
}

template <typename ComponentT>
ComponentT& World::GetComponentByIndex(ComponentIndex i)
{
    return GetComponentStorage<ComponentT>()[i];
}

template <typename ComponentT>
const ComponentT& World::GetComponentByIndex(ComponentIndex i) const
{
    return GetComponentStorage<ComponentT>()[i];
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

    SystemInvoke invoke;
    invoke.system = std::make_unique<SystemT>(std::forward<Args>(args)...);
    invoke.task   = taskflow_.emplace([system = invoke.system.get(), this](tf::Subflow& subflow) {
        ComponentAccess access(*this);
        EntityQuery     query(*this);
        system->Run(access, query, subflow);
    });

    systems_.emplace(index, std::move(invoke));
}

template <typename ComponentT>
inline World::EntityBuilder& World::EntityBuilder::AddComponent()
{
    world_.AddComponent<ComponentT>(entity_);
    return *this;
}

inline ComponentAccess::ComponentAccess(World& world) noexcept : world_(world) {}

template <typename ComponentT, typename StorageT>
inline StorageT& ComponentAccess::Write()
{
    return world_.GetComponentStorage<ComponentT>();
}

template <typename ComponentT, typename StorageT>
inline const StorageT& ComponentAccess::Read() const
{
    return world_.GetComponentStorage<ComponentT>();
}

template <typename SystemT>
inline SystemT& World::GetSystem()
{
    auto index = GetTypeIndex<SystemT>();

    auto system = systems_.find(index);

    if (system == systems_.cend())
    {
        throw std::runtime_error("World: system type not found");
    }

    return reinterpret_cast<SystemT&>(*system->second.system.get());
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

    system0->second.task.precede(system1->second.task);
}
}  // namespace yecs
