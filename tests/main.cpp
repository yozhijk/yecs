#include <iostream>

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

int main()
{
    World world;
    world.RegisterComponent<Position>();
    world.RegisterComponent<Velocity>();

    auto  entity   = world.CreateEntity().AddComponent<Position>().AddComponent<Velocity>().Build();
    auto& position = world.GetComponent<Position>(entity);
    position.x     = 100;
    position.y     = 100;

    struct TestSystem : public System
    {
        void Run(ComponentAccess& access, EntityQuery& entity_query) override
        {
            auto& positions  = access.Read<Velocity>();
            auto& velocities = access.Write<Position>();

            // auto entities = entity_query();
            // entities.FilterInPlace([](Entity e) { return true; });

            std::cout << "Positions size: " << positions.size() << "\n";
            std::cout << "Velocities size: " << velocities.size() << "\n";
        }
    };

    world.RegisterSystem<TestSystem>();
    world.Run();

    world.DestroyEntity(entity);
    return 0;
}