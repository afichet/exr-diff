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
#include "EXRImageFormat.hpp"

class ImageModule
{
  public:
    static XYZImage *load(const std::string &filename, float exposure = 0.f)
    {
        // Check if the filename size is long enough
        if (filename.size() < 5) {
            throw -1;
        }

        const char *filename_ext = &filename.c_str()[filename.size() - 4];

        if (strcmp(filename_ext, ".exr") == 0
            || strcmp(filename_ext, ".EXR") == 0) {
            return new EXRImageFormat(filename.c_str(), exposure);
        } else {
            std::cerr << "[error] Unknown file format " << filename_ext
                      << std::endl;
            std::cerr << "[error] Image name: " << filename << std::endl;
            throw -2;
        }
    }
};