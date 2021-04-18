#pragma once

#include <vector>

inline float clamp(float v, float v_min = 0, float v_max = 1.f) 
{
    return std::min(std::max(v, v_min), v_max);
}

inline float place(float v, float v_min, float v_max) 
{
    v = clamp(v, v_min, v_max);
    return (v - v_min) / (v_max - v_min);
}

void getRGB(
    float v, float v_min, float v_max,
    float RGB[3]) 
{
    v = place(v, v_min, v_max);

    std::vector<float[3]> scale(6);
    scale[0][0] = 0; scale[0][1] = 0; scale[0][2] = 0;
    scale[1][0] = 0; scale[1][1] = 0; scale[1][2] = 1;
    scale[2][0] = 0; scale[2][1] = 1; scale[2][2] = 1;
    scale[3][0] = 0; scale[3][1] = 1; scale[3][2] = 0;
    scale[4][0] = 1; scale[4][1] = 1; scale[4][2] = 0;
    scale[5][0] = 1; scale[5][1] = 0; scale[5][2] = 0;

    std::vector<float> values(scale.size());

    for (int i = 0; i < scale.size(); i++) {
        values[i] = float(i) / float(scale.size() - 1);
    }

    for (int i = 1; i < scale.size(); i++) {
        if (v <= values[i]) {
            float interp = place(v, values[i - 1], values[i]);

            for (int c = 0; c < 3; c++) {
                RGB[c] = interp * scale[i][c] 
                    + (1.f - interp) * scale[i - 1][c];
            }

            return;
        }
    }

    for (int c = 0; c < 3; c++) {
        RGB[c] = scale[scale.size() - 1][c];
    }
}