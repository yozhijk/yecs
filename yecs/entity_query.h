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
#include "yecs/entity_set.h"

namespace yecs
{
class World;

/**
 * @brief An interface providing entity querying functionality to System subclasses.
 *
 * EntityQuery's single purpose is to serve as a medium between World and System subclasses,
 * guarding world from unattended access.
 **/
class EntityQuery
{
public:
    explicit EntityQuery(World& world) noexcept;

    // Do not allow copies.
    EntityQuery(const EntityQuery&) = delete;
    EntityQuery operator=(const EntityQuery&) = delete;

    /**
     * @brief Return an EntitySet containing all entities in the world.
     *
     * EntitySet further provides filtering functionlaity on its entities.
     *
     * @return EntitySet with all the entities in the world.
     **/
    EntitySet operator()() const;

private:
    // Reference to our world object.
    World& world_;
};
}  // namespace yecs
