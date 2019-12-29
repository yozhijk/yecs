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

#include <vector>

#include "gtest/gtest.h"
#include "yecs/yecs.h"

class Test : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(Test, CreateWorld)
{
    using namespace yecs;
    World world;
}

TEST_F(Test, RegisterComponent)
{
    using namespace yecs;
    World world;

    struct Position
    {
        float x, y, z;
    };

    ASSERT_NO_THROW(world.RegisterComponent<Position>());
}

TEST_F(Test, RegisterSameTypeComponent)
{
    using namespace yecs;
    World world;

    struct Position
    {
        float x, y, z;
    };

    ASSERT_NO_THROW(world.RegisterComponent<Position>());
    ASSERT_THROW(world.RegisterComponent<Position>(), std::runtime_error);
}

TEST_F(Test, RegisterMultipleComponents)
{
    using namespace yecs;
    World world;

    struct Position
    {
        float x, y, z;
    };

    struct Velocity
    {
        float x, y, z;
    };

    struct Mass
    {
        float m;
    };

    ASSERT_NO_THROW(world.RegisterComponent<Position>());
    ASSERT_NO_THROW(world.RegisterComponent<Velocity>());
    ASSERT_NO_THROW(world.RegisterComponent<Mass>());
}

TEST_F(Test, CreateEntity)
{
    using namespace yecs;
    World world;

    struct Position
    {
        float x, y, z;
    };

    struct Velocity
    {
        float x, y, z;
    };

    struct Mass
    {
        float m;
    };

    ASSERT_NO_THROW(world.RegisterComponent<Position>());
    ASSERT_NO_THROW(world.RegisterComponent<Velocity>());
    ASSERT_NO_THROW(world.RegisterComponent<Mass>());

    ASSERT_NO_THROW(world.CreateEntity().AddComponent<Position>().AddComponent<Mass>().Build());
}

TEST_F(Test, CreateManyEntities)
{
    using namespace yecs;
    World world;

    struct Position
    {
        float x, y, z;
    };

    struct Velocity
    {
        float x, y, z;
    };

    struct Mass
    {
        float m;
    };

    ASSERT_NO_THROW(world.RegisterComponent<Position>());
    ASSERT_NO_THROW(world.RegisterComponent<Velocity>());
    ASSERT_NO_THROW(world.RegisterComponent<Mass>());

    for (auto i = 0u; i < 256u; ++i)
    {
        switch (i & 0x3)
        {
        case 0:
            ASSERT_NO_THROW(world.CreateEntity().AddComponent<Position>().Build());
            break;
        case 1:
            ASSERT_NO_THROW(world.CreateEntity().AddComponent<Position>().AddComponent<Velocity>().Build());
            break;
        case 2:
            ASSERT_NO_THROW(world.CreateEntity().AddComponent<Position>().AddComponent<Mass>().Build());
            break;
        }
    }
}

TEST_F(Test, RegisterSystem)
{
    using namespace yecs;
    World world;

    struct Position
    {
        float x, y, z;
    };

    struct Velocity
    {
        float x, y, z;
    };

    struct Mass
    {
        float m;
    };

    ASSERT_NO_THROW(world.RegisterComponent<Position>());
    ASSERT_NO_THROW(world.RegisterComponent<Velocity>());
    ASSERT_NO_THROW(world.RegisterComponent<Mass>());

    struct TestSystem : public System
    {
        void Run(ComponentAccess& access, EntityQuery& entity_query) override {}
    };

    ASSERT_NO_THROW(world.RegisterSystem<TestSystem>());
}

TEST_F(Test, RegisterSameTypeSystems)
{
    using namespace yecs;
    World world;

    struct Position
    {
        float x, y, z;
    };

    struct Velocity
    {
        float x, y, z;
    };

    struct Mass
    {
        float m;
    };

    ASSERT_NO_THROW(world.RegisterComponent<Position>());
    ASSERT_NO_THROW(world.RegisterComponent<Velocity>());
    ASSERT_NO_THROW(world.RegisterComponent<Mass>());

    struct TestSystem : public System
    {
        void Run(ComponentAccess& access, EntityQuery& entity_query) override {}
    };

    ASSERT_NO_THROW(world.RegisterSystem<TestSystem>());
    ASSERT_THROW(world.RegisterSystem<TestSystem>(), std::runtime_error);
}

TEST_F(Test, CountEntitiesSystem)
{
    using namespace yecs;
    World world;

    struct Position
    {
        float x, y, z;
    };

    struct Velocity
    {
        float x, y, z;
    };

    struct Mass
    {
        float m;
    };

    ASSERT_NO_THROW(world.RegisterComponent<Position>());
    ASSERT_NO_THROW(world.RegisterComponent<Velocity>());
    ASSERT_NO_THROW(world.RegisterComponent<Mass>());

    struct CountingSystem : public System
    {
        CountingSystem(std::uint32_t& pos_count, std::uint32_t& vel_count)
            : pos_count_(pos_count), vel_count_(vel_count)
        {
        }
        void Run(ComponentAccess& access, EntityQuery& entity_query) override
        {
            auto& positions  = access.Read<Position>();
            auto& velocities = access.Read<Velocity>();

            vel_count_ =
                static_cast<std::uint32_t>(entity_query()
                                               .Filter([&velocities](Entity e) { return velocities.HasComponent(e); })
                                               .entities()
                                               .size());

            pos_count_ = static_cast<std::uint32_t>(
                entity_query().Filter([&positions](Entity e) { return positions.HasComponent(e); }).entities().size());
        }

        std::uint32_t& pos_count_;
        std::uint32_t& vel_count_;
    };

    constexpr auto kNumEntities = 256;
    for (auto i = 0u; i < kNumEntities; ++i)
    {
        switch (i & 0x1)
        {
        case 0:
            ASSERT_NO_THROW(world.CreateEntity().AddComponent<Position>().Build());
            break;
        case 1:
            ASSERT_NO_THROW(world.CreateEntity().AddComponent<Velocity>().Build());
            break;
        }
    }

    std::uint32_t pos_count = 0;
    std::uint32_t vel_count = 0;
    ASSERT_NO_THROW(world.RegisterSystem<CountingSystem>(pos_count, vel_count));

    ASSERT_NO_THROW(world.Run());
    ASSERT_EQ(pos_count, kNumEntities / 2);
    ASSERT_EQ(vel_count, kNumEntities / 2);
}

TEST_F(Test, SimplePhysicsSystem)
{
    using namespace yecs;
    World world;

    struct Position
    {
        float x = 0.f;
        float y = 0.f;
        float z = 0.f;
    };

    struct Velocity
    {
        float x = 1.f;
        float y = 1.f;
        float z = 1.f;
    };

    ASSERT_NO_THROW(world.RegisterComponent<Position>());
    ASSERT_NO_THROW(world.RegisterComponent<Velocity>());

    struct PhysicsSystem : public System
    {
        void Run(ComponentAccess& access, EntityQuery& entity_query) override
        {
            auto& positions  = access.Write<Position>();
            auto& velocities = access.Read<Velocity>();

            auto entities = entity_query()
                                .Filter([&velocities, &positions](Entity e) {
                                    return positions.HasComponent(e) && velocities.HasComponent(e);
                                })
                                .entities();

            constexpr float dt = 1.f;
            for (auto e : entities)
            {
                auto& pos_comp = positions.GetComponent(e);
                auto& vel_comp = velocities.GetComponent(e);

                pos_comp.x += dt * vel_comp.x;
                pos_comp.y += dt * vel_comp.y;
                pos_comp.z += dt * vel_comp.z;
            }
        }
    };

    std::vector<Entity> entities;

    constexpr auto kNumEntities = 256;
    for (auto i = 0u; i < kNumEntities; ++i)
    {
        switch (i & 0x1)
        {
        case 0:
            entities.push_back(world.CreateEntity().AddComponent<Position>().Build());
            break;
        case 1:
            entities.push_back(world.CreateEntity().AddComponent<Position>().AddComponent<Velocity>().Build());
            break;
        }
    }

    ASSERT_NO_THROW(world.RegisterSystem<PhysicsSystem>());

    for (auto i = 0u; i < 10u; ++i) { ASSERT_NO_THROW(world.Run()); }

    for (auto i = 0u; i < kNumEntities; ++i)
    {
        auto& position = world.GetComponent<Position>(entities[i]);
        switch (i & 0x1)
        {
        case 0:
        {
            ASSERT_EQ(position.x, 0.f);
            ASSERT_EQ(position.y, 0.f);
            ASSERT_EQ(position.z, 0.f);
            break;
        }
        case 1:
            ASSERT_EQ(position.x, 10.f);
            ASSERT_EQ(position.y, 10.f);
            ASSERT_EQ(position.z, 10.f);
            break;
        }
    }
}