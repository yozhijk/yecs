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

#include "yecs/common.h"

namespace yecs
{
/** \brief Represets a collection of entities and provides filtering operations.
 *
 * EntitySet is primarily designed to be used as a result of entity queries for
 * systems. EntitySet provides and API to filter entities based on binary predicate.
 **/
class EntitySet
{
public:
    using EntityStorage = std::vector<Entity>;

    // Copies are forbidden.
    // TODO: do we need them?
    EntitySet(const EntitySet&) = delete;
    EntitySet& operator=(const EntitySet&) = delete;

    // Filter a set of entities inplace.
    template <typename F>
    EntitySet& FilterInPlace(F&& f);

    // Filter a set of entities and create a new one as a result.
    // Regular version.
    template <typename F>
    EntitySet Filter(F&& f) const&;
    // R-value version reusing internal storage of a temp object.
    template <typename F>
    EntitySet Filter(F&& f) &&;

    // Return entities.
    const EntityStorage& entities() const { return entities_; }

private:
    // Construct from L-value storage.
    EntitySet(const EntityStorage& entities) : entities_(entities) {}
    // Construct from temp storage (moving it in).
    EntitySet(EntityStorage&& entities) : entities_(std::move(entities)) {}
    // Entity storage.
    EntityStorage entities_;

    friend class EntityQuery;
};

template <typename F>
inline EntitySet& EntitySet::FilterInPlace(F&& f)
{
    auto new_end = std::partition(entities_.begin(), entities_.end(), std::forward<F>(f));
    entities_.resize(std::distance(entities_.begin(), new_end));
    return *this;
}

template <typename F>
inline EntitySet EntitySet::Filter(F&& f) &&
{
    auto new_end = std::partition(entities_.begin(), entities_.end(), std::forward<F>(f));
    entities_.resize(std::distance(entities_.begin(), new_end));
    return EntitySet(std::move(entities_));
}

template <typename F>
inline EntitySet EntitySet::Filter(F&& f) const&
{
    auto new_end = std::partition(entities_.begin(), entities_.end(), std::forward<F>(f));
    return EntitySet(EntityStorage(entities_.begin(), new_end));
}
}  // namespace yecs
