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
#include "colortools.hpp"
// #include "colormap.hpp"

#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>
#include <lodepng.h>
#include <tclap/CmdLine.h>

#include "ColorMap/TabulatedColorMap.hpp"
#include "ColorMap/BBGRColorMap.hpp"

int main(int argc, char* argv[])
{
  int ret_val = 0;

  float *rgba_1  = nullptr;
  float *rgba_2  = nullptr;
  unsigned char *rgb_out = nullptr;

  int width_1, height_1;
  int width_2, height_2;

  const char* err = nullptr;
  int ret = TINYEXR_SUCCESS;

  ColorMap* cmap = nullptr;

  try {
    TCLAP::CmdLine cmd("Difference tool for OpenEXR files", ' ', "0.1");

    TCLAP::UnlabeledValueArg<std::string> file_1Arg("file1", "File 1", true, "input_1.exr", "input file 1");
    TCLAP::UnlabeledValueArg<std::string> file_2Arg("file2", "File 2", true, "input_2.exr", "input file 2");

    TCLAP::ValueArg<std::string> fileoutArg("o", "output", "Output file", true, "out.png", "string");
    TCLAP::SwitchArg             scaleSwitch("s", "scale", "Add a scale next to the difference.", cmd, false);
	  TCLAP::ValueArg<float>       maxArg("m","max","Max value to use for Delta E 2000", false, 10.f , "Float");
	  TCLAP::ValueArg<float>       exposureArg("e","exposure","Set the exposure compensation value to use for input files", false, 0.f , "Float");
    TCLAP::ValueArg<std::string> colormapArg("c", "colormap", "Color map to use", false, "inferno", "magma, inferno, plasma, viridis");

    cmd.add(file_1Arg);
    cmd.add(file_2Arg);

    cmd.add(fileoutArg);
    cmd.add(maxArg);
    cmd.add(exposureArg);
    cmd.add(colormapArg);

    cmd.parse(argc, argv);

    const char* file_1 = file_1Arg.getValue().c_str();
    const char* file_2 = file_2Arg.getValue().c_str();
    const char* file_out = fileoutArg.getValue().c_str();
    const float max_deltaE = maxArg.getValue();
    const bool displayScale = scaleSwitch.getValue();
    const float exposure = exposureArg.getValue();
    const char* colormap = colormapArg.getValue().c_str();

    // Load the two EXR files to compare
    ret = LoadEXR(&rgba_1, &width_1, &height_1, file_1, &err);

    if (ret != TINYEXR_SUCCESS) {
      std::cerr << "[error] cannot load file: " << file_1 << std::endl;
      if (err) {
        std::cerr << "[error] " << err << std::endl;
        FreeEXRErrorMessage(err);
      }

      ret_val = -1;
      goto clean_exit;
    } 

    ret = LoadEXR(&rgba_2, &width_2, &height_2, file_2, &err);

    if (ret != TINYEXR_SUCCESS) {
        std::cerr << "[error] cannot load file: " << file_2 << std::endl;
      if (err) {
        std::cerr << "[error] " << err << std::endl;
        FreeEXRErrorMessage(err);
      }

      ret_val = -1;
      goto clean_exit;
    } 

    // Ensure they have the same dimensions
    if (width_1 != width_2 || height_1 != height_2) {
      std::cerr << "[error] Image dimensions mismatch." << std::endl;

      ret_val = -1;
      goto clean_exit;
    }

    // Create the colormap
    try {
      if (strcmp(colormap, "bbgr") == 0) {
        cmap = new BBGRColorMap();
      } else {
        cmap = new TabulatedColorMap(colormap);
      }
    } catch (int e) {
      std::cerr << "[error] Cannot create the colormap." << std::endl;
      
      ret_val = -1;
      goto clean_exit;
    }

    {
      const int width = width_1;
      const int height = height_1;

      // We need to determine the width of the output image depending on the
      // display of the color scale on the right or not
      const int width_scale = 30;
      const int width_out = (displayScale) ? width + width_scale : width;

      rgb_out = new unsigned char[width_out * height * 4];

      const float exposure_mul = std::exp2(exposure);

      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          const int offset = y * width + x; 

          float* px_rgb_1 = &rgba_1[4 * offset];
          float* px_rgb_2 = &rgba_2[4 * offset];

          // Apply exposure compensation
          for (int c = 0; c < 3; c++) {
            px_rgb_1[c] *= exposure_mul;
            px_rgb_2[c] *= exposure_mul;
          }

          // Convert colors to Lab space
          float Lab_1[3], Lab_2[3];
          lin_rgb_to_Lab(px_rgb_1, Lab_1);
          lin_rgb_to_Lab(px_rgb_2, Lab_2);

          // Compute the Delta E 2000 difference
          const float deltaE = deltaE2000(Lab_1, Lab_2);

          // Find a color maping for the Delta E value
          float scale_rgb[3];
          cmap->getRGBValue(deltaE, 0.f, max_deltaE, scale_rgb);
          
          // Set the output file pixel values
          for (int c = 0; c < 3; c++) {
            rgb_out[4 * (y * width_out + x) + c] = 255 * scale_rgb[c];
          }

          rgb_out[4 * (y * width_out + x) + 3] = 255;
        }
      }

      // If we use a color scale on the right, add it to the output image
      if (displayScale) {
        for (int y = 0; y < height; y++) {
          float v = float(height - y) / float(height - 1);
          float scale_rgb[3];
          cmap->getRGBValue(v, scale_rgb);

          for (int x = width; x < width_out; x++) {
            for (int c = 0; c < 3; c++) {
              rgb_out[4 * (y * width_out + x) + c] = 255 * scale_rgb[c];
            }
            rgb_out[4 * (y * width_out + x) + 3] = 255;
          }
        }
      }

      unsigned int err_encode = lodepng::encode(file_out, rgb_out, width_out, height);
    }
  } catch (TCLAP::ArgException &e) { 
    std::cerr << "[error] " << e.error() << " for arg " << e.argId() << std::endl; 
  }
  
clean_exit:
  free(rgba_1); // release memory of image data
  free(rgba_2); // release memory of image data
  delete[] rgb_out;
  delete cmap;

  return ret_val;
}
