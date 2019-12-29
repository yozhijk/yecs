#include <iostream>

#include "gtest/gtest.h"
#include "tests/tests.h"
#include "yecs/yecs.h"

using namespace std;
using namespace yecs;

struct Position
{
    float x, y;
};

struct Velocity
{
    float x, y;
};

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

int foo()
{
    World world;
    world.RegisterComponent<Position>();
    world.RegisterComponent<Velocity>();

    constexpr auto kNumEntities = 512;
    for (auto i = 0; i < kNumEntities; ++i)
    {
        if (i & 1)
        {
            world.CreateEntity().AddComponent<Position>().AddComponent<Velocity>().Build();
        }
        else
        {
            world.CreateEntity().AddComponent<Position>().Build();
        }
    }

    struct TestSystem : public System
    {
        void Run(ComponentAccess& access, EntityQuery& entity_query) override
        {
            auto& positions  = access.Read<Position>();
            auto& velocities = access.Write<Velocity>();

            auto entities = entity_query().Filter([&velocities](Entity e) { return velocities.HasComponent(e); });
            // entities.FilterInPlace([](Entity e) { return true; });

            std::cout << "Positions size: " << positions.size() << "\n";
            std::cout << "Velocities size: " << velocities.size() << "\n";
            std::cout << "Entities size: " << entities.entities().size() << "\n";
        }
    };

    world.RegisterSystem<TestSystem>();
    world.Run();

    world.Reset();
    return 0;
}