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

namespace yecs
{
class EntityQuery;
class ComponentAccess;

/**
 * @brief API for system implementers.
 *
 * World talks to registered systems via System interface calling System::Run() on every
 * registered system every time World::Run() is called.
 **/
class System
{
public:
    // Dtor.
    virtual ~System() = default;

    /**
     * @brief Run single step of system simulation.
     *
     * World calls this method once per World::Run invocation.
     * TODO: components access triggers a search in a components hash table every time.
     *
     * @param access API for comoponent acccess.
     * @param entity_query API for entity queries.
     **/
    virtual void Run(ComponentAccess& access, EntityQuery& entity_query, tf::Subflow& subflow) = 0;
};
}  // namespace yecs
