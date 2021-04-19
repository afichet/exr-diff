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
#include <cmath>


template<class Float>
void rgb_to_lin_rgb(const Float RGB_i[3], Float RGB_o[3])
{
    for (int i = 0; i < 3; i++) {
        if (RGB_i[i] < 0.04045) {
            RGB_o[i] = RGB_i[i] / 12.92;
        } else {
            RGB_o[i] = std::pow((RGB_i[i] + 0.055) / 1.055, 2.4);
        }
    }
}


template<class Float>
void lin_rgb_to_xyz(const Float RGB[3], Float XYZ[3])
{
    XYZ[0] = 0.4124564 * RGB[0] + 0.3575761 * RGB[1] + 0.1804375 * RGB[2];
    XYZ[1] = 0.2126729 * RGB[0] + 0.7151522 * RGB[1] + 0.0721750 * RGB[2];
    XYZ[2] = 0.0193339 * RGB[0] + 0.1191920 * RGB[1] + 0.9503041 * RGB[2];
}


// XYZ with each component in [0..1]
template<class Float>
void xyz_to_Lab(const Float XYZ[3], Float Lab[3])
{
    Float f[3];

    const Float epsilon = 0.008856;
    const Float kappa   = 903.3;

    for (int i = 0; i < 3; i++) {
        if (XYZ[i] > epsilon) {
            f[i] = std::pow(XYZ[i], 1. / 3.);
        } else {
            f[i] = (kappa * XYZ[i] + 16.) / 116.;
        }
    }

    Lab[0] = 116. * f[1] - 16.;
    Lab[1] = 500. * (f[0] - f[1]);
    Lab[2] = 200. * (f[1] - f[2]);
}


template<class Float>
void lin_rgb_to_Lab(const Float RGB[3], Float Lab[3])
{
    Float XYZ[3];
    lin_rgb_to_xyz(RGB, XYZ);
    xyz_to_Lab(XYZ, Lab);
}


template<class Float>
Float deltaE2000(const Float Lab_1[3], const Float Lab_2[3])
{
    const Float L1 = Lab_1[0];
    const Float a1 = Lab_1[1];
    const Float b1 = Lab_1[2];

    const Float L2 = Lab_2[0];
    const Float a2 = Lab_2[1];
    const Float b2 = Lab_2[2];

    const Float bar_L_prime = (L1 + L2) / 2.;

    const Float C1 = std::sqrt(a1 * a1 + b1 * b1);
    const Float C2 = std::sqrt(a2 * a2 + b2 * b2);

    const Float bar_C = (C1 + C2) / 2.;

    // bar_C**7
    Float bar_C_pow7 = bar_C * bar_C;                            // bar_C**2
    bar_C_pow7 = bar_C_pow7 * bar_C_pow7 * bar_C_pow7 * bar_C;   // bar_C**7

    const Float G
        = .5 * (1 - std::sqrt(bar_C_pow7 / (bar_C_pow7 + 6103515625)));

    const Float a_prime_1 = a1 * (1 + G);
    const Float a_prime_2 = a2 * (1 + G);

    const Float C_prime_1 = std::sqrt(a_prime_1 * a_prime_1 + b1 * b1);
    const Float C_prime_2 = std::sqrt(a_prime_2 * a_prime_2 + b2 * b2);

    const Float bar_C_prime = (C_prime_1 + C_prime_2) / 2.;

    Float h_prime_1 = std::atan2(a_prime_1, b1);
    if (h_prime_1 < 0) {
        h_prime_1 += 2. * M_PI;
    }

    Float h_prime_2 = std::atan2(a_prime_2, b2);
    if (h_prime_2 < 0) {
        h_prime_2 += 2. * M_PI;
    }

    const Float diff_h_prime = std::abs(h_prime_1 - h_prime_2);

    Float bar_H_prime;
    if (diff_h_prime > M_PI) {
        bar_H_prime = (h_prime_1 + h_prime_2 + 2. * M_PI) / 2.;
    } else {
        bar_H_prime = (h_prime_1 + h_prime_2) / 2.;
    }

    const Float T = 1. - 0.17 * std::cos(bar_H_prime - M_PI / 6.)
                    + 0.24 * std::cos(2. * bar_H_prime)
                    + 0.32 * std::cos(3. * bar_H_prime + M_PI / 30.)
                    - 0.20 * std::cos(4. * bar_H_prime - 7 * M_PI / 20.);

    Float delta_h_prime;
    if (diff_h_prime <= M_PI) {
        delta_h_prime = h_prime_2 - h_prime_1;
    } else if (diff_h_prime > M_PI && h_prime_2 <= h_prime_1) {
        delta_h_prime = h_prime_2 - h_prime_1 + 2. * M_PI;
    } else {
        delta_h_prime = h_prime_2 - h_prime_1 - 2. * M_PI;
    }

    const Float delta_L_prime = L2 - L1;
    const Float delta_C_prime = C_prime_2 - C_prime_1;
    const Float delta_H_prime
        = 2. * std::sqrt(C_prime_1 * C_prime_2) * std::sin(delta_h_prime / 2.);

    const Float bar_L_prime_minus50_sqr
        = (bar_L_prime - 50.) * (bar_L_prime - 50.);
    const Float S_L = 1.
                      + (0.015 * bar_L_prime_minus50_sqr)
                            / std::sqrt(20. + bar_L_prime_minus50_sqr);
    const Float S_C = 1. + 0.045 * bar_C_prime;
    const Float S_H = 1. + 0.015 * bar_C_prime * T;

    const Float bar_H_prime_r = ((bar_H_prime * 180. / M_PI) - 275.) / 25.;
    const Float delta_theta   = 30. * std::exp(-bar_H_prime_r * bar_H_prime_r);


    Float bar_C_prime_pow7 = bar_C_prime * bar_C_prime;   // bar_C**2
    bar_C_prime_pow7 = bar_C_prime_pow7 * bar_C_prime_pow7 * bar_C_prime_pow7
                       * bar_C_prime;   // bar_C**7

    const Float R_C
        = 2. * std::sqrt(bar_C_prime_pow7 / (bar_C_prime_pow7 + 6103515625));
    const Float R_T = -R_C * std::sin(2. * delta_theta);

    const Float K_L = 1.;
    const Float K_C = 1.;
    const Float K_H = 1.;

    const Float delta_L_r = delta_L_prime / (K_L * S_L);
    const Float delta_C_r = delta_C_prime / (K_C * S_C);
    const Float delta_H_r = delta_H_prime / (K_H * S_H);

    return std::sqrt(
        delta_L_r * delta_L_r + delta_C_r * delta_C_r + delta_H_r * delta_H_r
        + R_T * delta_C_r * delta_H_r);
}
