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
    void RegisterSystem(ComponentTypes&& read, ComponentTypes&& write, Args&&... args);

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
    // Each time entity space is out, we extend an array by this number of elements.
    static constexpr uint32_t kEntitySizeIncrement = 128;

    // System data.
    struct SystemDesc
    {
        ComponentTypes          read;
        ComponentTypes          write;
        std::unique_ptr<System> system;

        SystemDesc(ComponentTypes&& r, ComponentTypes&& w, std::unique_ptr<System>&& s)
            : read{std::move(r)}, write{std::move(w)}, system{std::move(s)}
        {
        }
    };

    using ComponetsBase = ComponentCollectionBase;
    template <typename T>
    using Components    = ComponentCollection<T>;
    using ComponentsMap = std::unordered_map<std::type_index, std::unique_ptr<ComponentCollectionBase>>;

    // Entity array: true if entity exists, false if not.
    std::mutex        entity_mutex_;
    std::vector<bool> entities_;
    // Component arrays.
    std::mutex    component_mutex_;
    ComponentsMap components_;
    // Systems.
    std::mutex              system_mutex_;
    std::vector<SystemDesc> systems_;

    friend class EntityQuery;
};

template <typename ComponentT>
inline void World::RegisterComponent()
{
    std::lock_guard<std::mutex> lock(component_mutex_);

    auto index = GetTypeIndex<ComponentT>();
    assert(components_.find(index) == components_.cend());

    components_.emplace(index, std::make_unique<ComponentCollection<ComponentT>>());
}

template <typename ComponentT>
inline ComponentT& World::AddComponent(Entity entity)
{
    std::lock_guard<std::mutex> lock(component_mutex_);

    auto index = GetTypeIndex<ComponentT>();
    assert(components_.find(index) != components_.cend());

    auto collection = static_cast<Components<ComponentT>*>(components_[index].get());
    return collection->AddComponent(entity);
}

template <typename ComponentT>
inline ComponentT& World::GetComponent(Entity entity)
{
    auto index = GetTypeIndex<ComponentT>();
    assert(components_.find(index) != components_.cend());

    auto collection = static_cast<Components<ComponentT>*>(components_[index].get());
    return collection->GetComponent(entity);
}

// Direct component access.
template <typename ComponentT>
size_t World::GetNumComponents() const
{
    auto index = GetTypeIndex<ComponentT>();
    assert(components_.find(index) != components_.cend());

    auto collection = static_cast<Components<ComponentT>*>(components_[index].get());
    return collection->size();
}

template <typename ComponentT>
ComponentT& World::GetComponentByIndex(ComponentIndex i)
{
    auto index = GetTypeIndex<ComponentT>();
    assert(components_.find(index) != components_.cend());

    auto collection = static_cast<Components<ComponentT>*>(components_[index].get());
    return collection[i];
}

template <typename ComponentT>
const ComponentT& World::GetComponentByIndex(ComponentIndex i) const
{
    auto index = GetTypeIndex<ComponentT>();
    assert(components_.find(index) != components_.cend());

    auto collection = static_cast<Components<ComponentT>*>(components_[index].get());
    return collection[i];
}

template <typename SystemT, typename... Args>
inline void World::RegisterSystem(ComponentTypes&& read, ComponentTypes&& write, Args&&... args)
{
    std::lock_guard<std::mutex> lock(system_mutex_);

    systems_.emplace_back(std::move(read), std::move(write), std::make_unique<SystemT>(std::forward<Args>(args)...));
}

template <typename ComponentT>
inline World::EntityBuilder& World::EntityBuilder::AddComponent()
{
    world_.AddComponent<ComponentT>(entity_);
    return *this;
}
}  // namespace yecs
