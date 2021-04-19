#pragma once

#include "ColorMap.hpp"
#include <ColorMapData.hpp>
#include <vector>
#include <cstring>

class TabulatedColorMap: public ColorMap 
{
public:
    TabulatedColorMap()
        : _array(1)
    {}

    TabulatedColorMap(const char* name)
    {
        if (strcmp(name, "magma") == 0) {
            init(magma_data, 256);
        } else if (strcmp(name, "inferno") == 0) {
            init(inferno_data, 256);
        } else if (strcmp(name, "plasma") == 0) {
            init(plasma_data, 256);
        } else if (strcmp(name, "viridis") == 0) {
            init(viridis_data, 256);
        } else {
            std::cerr 
            << "[error] unknown color map." << std::endl
            << "[error] You can choose between magma, inferno, plasma or viridis." << std::endl;

            throw -1;
        }
    }

    virtual void getRGBValue(float v, float RGB[3]) const 
    {
        int closet_idx = v * (_array.size()/3 - 1);
        memcpy(RGB, &_array[3 * closet_idx], 3 * sizeof(float));
    }

    virtual void getRGBValue(float v, float v_min, float v_max, float RGB[3]) const 
    {
        getRGBValue((v - v_min) / (v_max - v_min), RGB);
    }

protected:
    void init(float * array, int n_elems) 
    {
        _array.resize(3 * n_elems);
        memcpy(_array.data(), array, 3 * n_elems * sizeof(float));
    }

private:
    std::vector<float> _array;
};