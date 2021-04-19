#pragma once

#include "BBGRColorMap.hpp"
#include "TabulatedColorMap.hpp"

class ColorMapModule
{
  public:
    static ColorMap *create(const std::string &name)
    {
        const char *colormap_name = name.c_str();

        if (strcmp(colormap_name, "bbgr") == 0) {
            return new BBGRColorMap();
        } else if (
            strcmp(colormap_name, "magma") == 0
            || strcmp(colormap_name, "inferno") == 0
            || strcmp(colormap_name, "plasma") == 0
            || strcmp(colormap_name, "viridis") == 0) {
            return new TabulatedColorMap(colormap_name);
        } else {
            std::cout << "[error] Invalid colormap name: " << name << std::endl;
            std::cout << "[error] Valid names are: bbgr, magma, inferno, "
                         "plasma or viridis"
                      << std::endl;

            throw -1;
        }
    }
};
