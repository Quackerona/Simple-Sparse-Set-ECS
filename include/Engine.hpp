#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "World.hpp"
#include "Resources.hpp"

class Engine
{
    private:
        World world;
        Resources resources;

        std::vector<void(*)(World&, Resources&)> systems;
    public:
        Engine() = default;
        Engine(const Engine&) = delete;
        Engine(Engine&&) = delete;

        Engine& operator=(const Engine&) = delete;
        Engine& operator=(Engine&&) = delete;

        void AddSystem(void(*system)(World&, Resources&))
        {
            systems.push_back(system);
        }

        void Run()
        {
            while (resources.keepAlive)
            {
                for (auto func : systems)
                    func(world, resources);
            }
        }
};

#endif