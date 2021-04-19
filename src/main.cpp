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

#include <iostream>
#include <cstddef>
#include "colortools.hpp"

#include <lodepng.h>
#include <tclap/CmdLine.h>

#include "ImageFormat/ImageModule.hpp"
#include "ColorMap/ColorMapModule.hpp"

int main(int argc, char *argv[])
{
    int ret_val = 0;

    std::string filename_1;
    std::string filename_2;
    std::string filename_out;

    std::string colormap_name;

    float max_deltaE;
    float exposure;
    bool  displayScale;

    XYZImage *image_1 = nullptr;
    XYZImage *image_2 = nullptr;

    ColorMap *cmap = nullptr;

    unsigned char *rgb_out = nullptr;

    // Parse command line
    try {
        TCLAP::CmdLine cmd("Difference tool for OpenEXR files", ' ', "0.1");

        TCLAP::UnlabeledValueArg<std::string>
            file_1Arg("file1", "File 1", true, "input_1.exr", "input file 1");
        TCLAP::UnlabeledValueArg<std::string>
            file_2Arg("file2", "File 2", true, "input_2.exr", "input file 2");

        TCLAP::ValueArg<std::string>
            fileoutArg("o", "output", "Output file", true, "out.png", "string");
        TCLAP::SwitchArg scaleSwitch(
            "s",
            "scale",
            "Add a scale next to the difference.",
            cmd,
            false);
        TCLAP::ValueArg<float> maxArg(
            "m",
            "max",
            "Max value to use for Delta E 2000",
            false,
            10.f,
            "Float");
        TCLAP::ValueArg<float> exposureArg(
            "e",
            "exposure",
            "Set the exposure compensation value to use for input files",
            false,
            0.f,
            "Float");
        TCLAP::ValueArg<std::string> colormapArg(
            "c",
            "colormap",
            "Color map to use",
            false,
            "bbgr",
            "bbgr, magma, inferno, plasma, viridis");

        cmd.add(file_1Arg);
        cmd.add(file_2Arg);

        cmd.add(fileoutArg);
        cmd.add(maxArg);
        cmd.add(exposureArg);
        cmd.add(colormapArg);

        cmd.parse(argc, argv);

        filename_1    = file_1Arg.getValue();
        filename_2    = file_2Arg.getValue();
        filename_out  = fileoutArg.getValue();
        colormap_name = colormapArg.getValue();

        max_deltaE   = maxArg.getValue();
        exposure     = exposureArg.getValue();
        displayScale = scaleSwitch.getValue();
    } catch (TCLAP::ArgException &e) {
        std::cerr << "[error] " << e.error() << " for arg " << e.argId()
                  << std::endl;

        ret_val = -1;
        goto clean_exit;
    }

    // Ensure the output file is in a PNG format
    if (filename_out.size() < 5) {
        std::cerr << "[error] Wrong output filename: too short" << std::endl;
        ret_val = -2;
        goto clean_exit;
    }

    {
        const char *filename_out_ext
            = &filename_out.c_str()[filename_out.size() - 4];

        if (strcmp(filename_out_ext, ".png") != 0
            && strcmp(filename_out_ext, ".PNG") != 0) {
            std::cerr << "[error] Wrong file extension for output."
                      << std::endl;
            std::cerr << "[error] Supported extensions: .png, .PNG"
                      << std::endl;

            ret_val = -3;
            goto clean_exit;
        }
    }

    // Load the two EXR files to compare
    try {
        image_1 = ImageModule::load(filename_1.c_str(), exposure);
        image_2 = ImageModule::load(filename_2.c_str(), exposure);
    } catch (int e) {
        std::cerr << "[error] Cannot load images." << std::endl;

        ret_val = -1;
        goto clean_exit;
    }

    // Ensure they have the same dimensions
    if (image_1->width() != image_2->width()
        || image_2->height() != image_2->height()) {
        std::cerr << "[error] Image dimensions mismatch." << std::endl;

        ret_val = -1;
        goto clean_exit;
    }

    // Create the colormap
    try {
        cmap = ColorMapModule::create(colormap_name);
    } catch (int e) {
        std::cerr << "[error] Cannot create the colormap." << std::endl;

        ret_val = -1;
        goto clean_exit;
    }

    {
        const size_t width  = image_1->width();
        const size_t height = image_1->height();

        // We need to determine the width of the output image depending on the
        // display of the color scale on the right or not
        const float  scale_percent = 0.05f;
        const size_t width_scale
            = std::max(30, int(scale_percent * float(width)));
        const size_t width_out = (displayScale) ? width + width_scale : width;

        rgb_out = new unsigned char[width_out * height * 4];

        #pragma omp parallel for
        for (size_t i = 0; i < width * height; i++) {
            const size_t x          = i % width;
            const size_t y          = i / width;
            const size_t offset_out = y * width_out + x;

            // Convert colors to Lab space
            float Lab_1[3], Lab_2[3];
            xyz_to_Lab(&image_1->data_xyz()[3 * i], Lab_1);
            xyz_to_Lab(&image_2->data_xyz()[3 * i], Lab_2);

            // Compute the Delta E 2000 difference
            const float deltaE = deltaE2000(Lab_1, Lab_2);

            // Find a color maping for the Delta E value
            float scale_rgb[3];
            cmap->getRGBValue(deltaE, 0.f, max_deltaE, scale_rgb);

            // Set the output file pixel values
            for (int c = 0; c < 3; c++) {
                rgb_out[4 * offset_out + c] = 255 * scale_rgb[c];
            }

            rgb_out[4 * offset_out + 3] = 255;
        }

        // If we use a color scale on the right, add it to the output image
        if (displayScale) {
            #pragma omp parallel for
            for (size_t y = 0; y < height; y++) {
                float v = float(height - 1 - y) / float(height - 1);
                float scale_rgb[3];
                cmap->getRGBValue(v, scale_rgb);

                for (size_t x = width; x < width_out; x++) {
                    for (int c = 0; c < 3; c++) {
                        rgb_out[4 * (y * width_out + x) + c]
                            = 255 * scale_rgb[c];
                    }
                    rgb_out[4 * (y * width_out + x) + 3] = 255;
                }
            }
        }

        lodepng::encode(filename_out, rgb_out, width_out, height);
    }

clean_exit:
    delete[] rgb_out;
    delete cmap;
    delete image_1;
    delete image_2;


    return ret_val;
}
