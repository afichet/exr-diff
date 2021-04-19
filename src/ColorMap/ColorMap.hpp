#pragma once


class ColorMap 
{
public:
    ColorMap() {}

    virtual void getRGBValue(float v, float RGB[3]) const = 0;

    virtual void getRGBValue(float v, float v_min, float v_max, float RGB[3]) const = 0;
    // {
    //     getRGBValue((v - v_min) / (v_max - v_min), RGB);
    // }
};