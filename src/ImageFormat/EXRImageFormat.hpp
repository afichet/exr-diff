//
// Copyright (c) 2021 Alban Fichet <alban.fichet at gmx.fr>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
//  * Neither the name of %ORGANIZATION% nor the names of its contributors may be
// used to endorse or promote products derived from this software without specific
// prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#pragma once

#include "XYZImage.hpp"

#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>
#include "../colortools.hpp"

class EXRImageFormat: public XYZImage
{
  public:
    EXRImageFormat(const char *filename, float exposureValue = 0.f)
      : XYZImage(0, 0)
    {
        float      *rgba = nullptr;
        int         width, height;
        const char *err = nullptr;
        int         ret = LoadEXR(&rgba, &width, &height, filename, &err);

        if (ret != TINYEXR_SUCCESS) {
            std::cerr << "[error] cannot load file: " << filename << std::endl;
            if (err) {
                std::cerr << "[error] " << err << std::endl;
                FreeEXRErrorMessage(err);
            }

            throw ret;
        }

        const float exposure_mul = std::exp2(exposureValue);

        // Now allocate memory and conver to XYZ colorspace
        resize(width, height);

        #pragma omp parallel for
        for (size_t i = 0; i < _width * _height; i++) {
            for (int c = 0; c < 3; c++) {
                rgba[4 * i + c] *= exposure_mul;
            }

            lin_rgb_to_xyz(&rgba[4 * i], &data_xyz()[3 * i]);
        }

        free(rgba);
    }

    virtual ~EXRImageFormat() {}
};