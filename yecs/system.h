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
#include "yecs/component_collection.h"
#include "yecs/entity_query.h"

namespace yecs
{
class ComponentAccess
{
public:
    template <typename ComponentT>
    ComponentCollection<ComponentT>& Get()
    {
    }

    template <typename ComponentT>
    const ComponentCollection<ComponentT>& Get() const
    {
    }
};

struct SystemInput
{
    const ComponentAccess& read;
    ComponentAccess&       write;
    EntityQuery&           query_entities;
};

class System
{
public:
    virtual ~System()                    = default;
    virtual void Run(SystemInput& input) = 0;
};
}  // namespace yecs
