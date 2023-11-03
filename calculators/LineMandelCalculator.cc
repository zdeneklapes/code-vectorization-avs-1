/**
 * @file LineMandelCalculator.cc
 * @author FULL NAME <xlogin00@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date DATE
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>
#include <string.h>


#include "LineMandelCalculator.h"


LineMandelCalculator::LineMandelCalculator(unsigned matrixBaseSize, unsigned limit) :
        BaseMandelCalculator(matrixBaseSize, limit, "LineMandelCalculator") {
    // @TODO allocate & prefill memory
    data = (int *) (aligned_alloc(64, height * width * sizeof(int)));
    z_real = (float *) (aligned_alloc(64, width * sizeof(float)));
    z_imag = (float *) (aligned_alloc(64, width * sizeof(float)));
}

LineMandelCalculator::~LineMandelCalculator() {
    // @TODO cleanup the memory
    free(data);
    free(z_real);
    free(z_imag);
    data = NULL;
    z_real = NULL;
    z_imag = NULL;
}

int *LineMandelCalculator::calculateMandelbrot() {
    // @TODO implement the calculator & return array of integers
#pragma omp simd simdlen(64) linear(i) aligned(data: 64)
    for (int i = 0; i < height * width; ++i) {
        data[i] = limit;
    }

    for (int i_index = 0; i_index < height / 2; i_index++) {
        int i_index_first_cell = i_index * width;
        float imag = y_start + i_index * dy;
        int i_index_from_bottom = (height - i_index - 1) * width;

#pragma omp simd simdlen(64) linear(r_index)
        for (int r_index = 0; r_index < width; r_index++) {
            z_real[r_index] = x_start + r_index * dx; // current real value
            z_imag[r_index] = imag;
        }

        int cell_count = width;
#pragma omp simd
        for (int iteration = 0; iteration < limit; ++iteration) {
#pragma omp simd simdlen(64)
            for (int r_index = 0; r_index < width; r_index++) {
                if (data[i_index_first_cell + r_index] != limit) {
                    // Skip already calculated cells
                    continue;
                }

                // Calculate next iteration
                float r2 = z_real[r_index] * z_real[r_index];
                float i2 = z_imag[r_index] * z_imag[r_index];

                if (r2 + i2 > 4.0f) {
                    data[i_index_first_cell + r_index] = iteration; // Set cell in the part above the X-axis
                    data[i_index_from_bottom + r_index] = iteration; // Set cell in the part below the X-axis
                    cell_count = cell_count - 1;
                    continue;
                }

                z_imag[r_index] = 2.0f * z_real[r_index] * z_imag[r_index] + imag;
                z_real[r_index] = r2 - i2 + (x_start + r_index * dx);
            }
            if (!cell_count) break;
        }
    }

    return data;
}
