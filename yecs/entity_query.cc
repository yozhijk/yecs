#include "entity_query.h"

#include "yecs/world.h"

namespace yecs
{
EntityQuery::EntityQuery(World& world) : world_(world) {}
EntitySet EntityQuery::operator()() const
{
    EntitySet::EntityStorage entities;
    for (uint32_t i = 0; i < world_.entities_.size(); ++i)
    {
        if (world_.entities_[i])
            entities.push_back(i);
    }
    return EntitySet(std::move(entities));
}
}  // namespace yecs