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

    world.DestroyEntity(entity);
    return 0;
}