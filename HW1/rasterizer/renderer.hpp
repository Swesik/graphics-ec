#ifndef RENDERER_H
#define RENDERER_H

#include "entities.hpp"
#include "loader.hpp"
#include "rasterizer.hpp"

class Renderer {
public:
    Renderer(std::string configName)
        : configName(configName) {};

    void Render(int argc, char** argv);   // main render call

private:
    std::string configName;
};

#endif
