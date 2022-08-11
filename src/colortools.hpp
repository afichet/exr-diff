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

    // D65
    const Float coefs[3] = {0.950489, 1., 1.08840};

    for (int i = 0; i < 3; i++) {
        Float t = XYZ[i] / coefs[i];

        if (t > epsilon) {
            f[i] = std::cbrt(t);
        } else {
            f[i] = (kappa * t + 16.) / 116.;
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

    const Float C_star_1 = std::sqrt(a1 * a1 + b1 * b1);
    const Float C_star_2 = std::sqrt(a2 * a2 + b2 * b2);
    const Float bar_C_star = (C_star_1 + C_star_2) / 2.;

    Float bar_C_start_p7 = bar_C_star * bar_C_star;   // ^2
    bar_C_start_p7 = bar_C_start_p7 * bar_C_start_p7 * bar_C_start_p7 * bar_C_star; // ^7

    const Float G = .5 * (1. - std::sqrt((bar_C_start_p7 / (bar_C_start_p7 + 6103515625.))));
    const Float a_prime_1 = (1. + G) * a1;
    const Float a_prime_2 = (1. + G) * a2;
    // std::cout << ">> " << bar_C_start_p7 / (bar_C_start_p7 + 6103515625.) << std::endl;
    const Float C_prime_1 = std::sqrt(a_prime_1 * a_prime_1 + b1 * b1);
    const Float C_prime_2 = std::sqrt(a_prime_2 * a_prime_2 + b2 * b2);

    Float h_prime_1_rad = 0;

    if (b1 != 0 || a_prime_1 != 0) {
        h_prime_1_rad = std::atan2(b1, a_prime_1);

        if (h_prime_1_rad < 0) {
            h_prime_1_rad += 2. * M_PI;
        }
    }

    Float h_prime_2_rad = 0;

    if (b2 != 0 || a_prime_2 != 0) {
        h_prime_2_rad = std::atan2(b2, a_prime_2);

        if (h_prime_2_rad < 0) {
            h_prime_2_rad += 2. * M_PI;
        }
    }

    const Float p_C_prime_12 = C_prime_1 + C_prime_2;

    Float Delta_h_prime_rad = 0;

    if (p_C_prime_12 != 0) {
        if (std::abs(h_prime_2_rad - h_prime_1_rad) <= M_PI) {
            Delta_h_prime_rad = h_prime_2_rad - h_prime_1_rad;
        } else if (h_prime_2_rad - h_prime_1_rad > M_PI) {
            Delta_h_prime_rad = h_prime_2_rad - h_prime_1_rad - 2. * M_PI;
        } else {
            Delta_h_prime_rad = h_prime_2_rad - h_prime_1_rad + 2. * M_PI;
        }
    }

    const Float Delta_L_prime = L2 - L1;
    const Float Delta_C_prime = C_prime_2 - C_prime_1;
    const Float Delta_H_prime = 2. * std::sqrt(C_prime_1 * C_prime_2) * std::sin(Delta_h_prime_rad / 2.);

    const Float bar_L_prime = (L1 + L2) / 2.;
    const Float bar_C_prime = (C_prime_1 + C_prime_2) / 2.;

    Float bar_h_prime_rad = 0;

    if (p_C_prime_12 == 0) {
        bar_h_prime_rad = h_prime_1_rad + h_prime_2_rad;
    } else if (std::abs(h_prime_1_rad - h_prime_2_rad) <= M_PI) {
        bar_h_prime_rad = (h_prime_1_rad + h_prime_2_rad) / 2.;
    } else if (h_prime_1_rad + h_prime_2_rad < 2. * M_PI) {
        bar_h_prime_rad = (h_prime_1_rad + h_prime_2_rad + 2. * M_PI) / 2.;
    } else {
        bar_h_prime_rad = (h_prime_1_rad + h_prime_2_rad - 2. * M_PI) / 2.;
    }

    const Float T = 1.
        - 0.17 * std::cos(     bar_h_prime_rad - M_PI / 6.)
        + 0.24 * std::cos(2. * bar_h_prime_rad)
        + 0.32 * std::cos(3. * bar_h_prime_rad + M_PI / 30.)
        - 0.20 * std::cos(4. * bar_h_prime_rad - 7. * M_PI / 20.);

    const Float exp_v = (bar_h_prime_rad * 180. / M_PI - 275.) / 25.;
    const Float Delta_theta_rad = M_PI / 6. * std::exp(-exp_v * exp_v);

    Float bar_C_prime_p7 = bar_C_prime * bar_C_prime;
    bar_C_prime_p7 = bar_C_prime_p7 * bar_C_prime_p7 * bar_C_prime_p7 * bar_C_prime;

    Float bar_L_prime_m50_p2 = bar_L_prime - 50.;
    bar_L_prime_m50_p2 =bar_L_prime_m50_p2 * bar_L_prime_m50_p2;

    const Float R_C = 2. * std::sqrt(bar_C_prime_p7 / (bar_C_prime_p7 + 6103515625.));
    const Float S_L = 1. + 0.015 * bar_L_prime_m50_p2 / std::sqrt(20. + bar_L_prime_m50_p2);
    const Float S_C = 1. + 0.045 * bar_C_prime;
    const Float S_H = 1. + 0.015 * bar_C_prime * T;
    const Float R_T = -std::sin(2. * Delta_theta_rad) * R_C;

    const Float K_L = 1.;
    const Float K_C = 1.;
    const Float K_H = 1.;

    const Float delta_L_r = Delta_L_prime / (K_L * S_L);
    const Float delta_C_r = Delta_C_prime / (K_C * S_C);
    const Float delta_H_r = Delta_H_prime / (K_H * S_H);

    return std::sqrt(
          delta_L_r * delta_L_r
        + delta_C_r * delta_C_r
        + delta_H_r * delta_H_r
        + R_T * delta_C_r * delta_H_r);
}
