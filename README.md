# Simple Sparse Set ECS
Simple header-only implementation of a sparse set ECS model.

>[!WARNING]
>This is a test project made for fun, it is NOT production-ready whatsoever.

This project includes the following features:
- An ECS world.
    - Entity creation.
    - Dynamic sparse sets creation for components.
    - Views to filter any entity with a specific combination of components.
- A global resource container.
    - States.
    - Keep-Alive (So you can shut off the program at anytime).
- A convenient engine class to quickly manage said features.
    - Systems to process both entities and components.

A full example can be found within the ```SimpleExample.cpp``` file.
