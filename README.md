# yecs
Simplistic C++17 entity-component-system engine

## Build status
[![Build Status](https://travis-ci.org/yozhijk/yecs.svg?branch=master)](https://travis-ci.org/yozhijk/yecs)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/426af3d9a71f4dffabf777dfccdf2f0a)](https://www.codacy.com/manual/yozhijk/yecs?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=yozhijk/yecs&amp;utm_campaign=Badge_Grade)

## System requirements
### Windows
  - CMake 12.4 or later
  - Visual Studio 2017 or later

### OSX
  - CMake 12.4 or later
  - XCode

### Linux
  - CMake 12.4 or later
  - GCC 7.3 or later

## Build steps
```sh
git clone --recursive https://github.com/yozhijk/yecs.git yecs
mkdir build
cmake -S . -B build
cd build
make -j4
```
## Usage
To open a session, the user should create an instance of yecs::World, which provides majority of ECS client API.

Typical ECS worlflow has the following steps:

1) Define component types
2) Create entities
3) Implement and register systems
4) Run simulations

### Defining component types
Components are registered in yecs::World using yecs::World::RegisterComponent<T>:
  
```c
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

world.RegisterComponent<Position>();
world.RegisterComponent<Velocity>());
world.RegisterComponent<Mass>());
```

### Creating entities
Entities are creating via world.CreateEntity() call. This method returns a builder object allowing easy composition from multiple components:
  
```c
auto e = world.CreateEntity().AddComponent<Position>().AddComponent<Mass>().Build();
```

### Registering systems
Systems are implemented by subclassing yecs::System interace. Systems can request read or write access to components in Run() method via ComponentAccess interace passed in. Entities can be queried and filtered using entity_query object:

```c
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
```

Systems are registered in the world using World::RegisterSystem<T>() method. If specific order of system invocations is required, World::Precede<S0, S1> method can be used (which forces S0 to be executed prior to S1):
  
```c
world.Precede<PhysicsSystem, RenderingSystem>();
```

### Running simulation
A single step of a simulation (calling every system exactly once) is achieved using:

```c
world.Run();
```

Systems are executed in parallel wherever possible, but precedence constraints are fulfilled.
