#include "world.h"

namespace yecs
{
void World::Run()
{
    for (auto& system : systems_)
    {
        ComponentAccess access(*this);
        EntityQuery     query(*this);

        system->Run(access, query);
    }
}

void World::Reset()
{
    entities_.clear();
    components_.clear();
    systems_.clear();
}

World::EntityBuilder World::CreateEntity()
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
}  // namespace yecs