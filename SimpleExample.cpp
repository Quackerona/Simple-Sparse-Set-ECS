#include <cassert>
#include <iostream>

#include "Engine.hpp"

struct Position
{
    float x = 0;
    float y = 0;
};

struct Size
{
    float w = 0;
    float h = 0;
};

void MySystem(World& world, Resources& resources)
{
    // Spawning a new entity.
    Entity entity{world.SpawnEntity()};

    // Assigning entity components.
    world.AddComponent<Position>(entity, {20.0f, 13.0f});
    world.AddComponent<Size>(entity, {100.0f, 300.0f});

    // Creating a view into every entity with a combination of components.
    std::optional<View<Position, Size>> view{world.CreateView<Position, Size>()};

    // Checking if entity has a component.
    assert(view->HasComponent<Position>(entity) == true);

    // Retrieving component info individually.
    Position* position{view->GetComponent<Position>(entity)};
    Size* size{view->GetComponent<Size>(entity)};
    assert(position->x == 20.0f);
    assert(size->w == 100.0f);

    // Loop through all entities with said combination of components.
    view->Each([](Position& position, Size& size){
        assert(position.x == 20.0f);
        assert(size.w == 100.0f);
    });

    // Last example, but with expanded parameters.
    view->Each([](Entity entity, Position& position, Size& size){
        assert(entity.GetID() == 0);
    });

    // Despawning an entity individually.
    world.DespawnEntity(entity);

    // If you have a lot of entities to be cleaned up, you can despawn all of them all at once.
    world.ClearEntities();

    // Changing state
    resources.state = Exit;

    std::cout << "End!" << std::endl;

    // Closes the program when done.
    if (resources.state == Exit) // This if-statement is not needed, this is purely for documentation
        resources.keepAlive = false;
}

int main()
{
    Engine engine;

    // Adding systems (Can have multiple systems.)
    engine.AddSystem(MySystem);

    // Run the engine when done setting up.
    engine.Run();
}