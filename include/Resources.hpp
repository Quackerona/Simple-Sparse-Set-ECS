#ifndef RESOURCES_HPP
#define RESOURCES_HPP

enum State
{
    Setup,
    Exit
};

class Resources
{
    private:
        Resources() : state(Setup), keepAlive(true) {}

        friend class Engine;
    public:
        Resources(const Resources&) = delete;
        Resources(Resources&&) = delete;

        Resources& operator=(const Resources&) = delete;
        Resources& operator=(Resources&&) = delete;

        State state;
        bool keepAlive;
};

#endif