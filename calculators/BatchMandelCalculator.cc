/**
 * @file BatchMandelCalculator.cc
 * @author FULL NAME <xlogin00@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over small batches
 * @date DATE
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>
#include <stdexcept>
#include <cmath>

#include "BatchMandelCalculator.h"

BatchMandelCalculator::BatchMandelCalculator(unsigned matrixBaseSize, unsigned limit) :
        BaseMandelCalculator(matrixBaseSize, limit, "BatchMandelCalculator") {
    // @TODO allocate & prefill memory
    data = (int *) (aligned_alloc(height * width * sizeof(int), 64));
    z_real = (float *) (aligned_alloc(width * sizeof(float), 64));
    z_imag = (float *) (aligned_alloc(width * sizeof(float), 64));
}

BatchMandelCalculator::~BatchMandelCalculator() {
    // @TODO cleanup the memory
    free(data);
    free(z_real);
    free(z_imag);
    data = NULL;
    z_real = NULL;
    z_imag = NULL;
}


int *BatchMandelCalculator::calculateMandelbrot() {
    // @TODO implement the calculation

    // Set all cells to limit
    for (int i = 0; i < height * width; ++i) {
        data[i] = limit;
    }


    for (int iteration = 0; iteration < limit; ++iteration) {
        int cell_count = width;
        for (int i = 0; i < height * width; ++i) {
            if (data[i] != limit) { // Skip already calculated cells
                continue;
            }

            // Calculate next iteration
            float r2 = z_real[i] * z_real[i];
            float i2 = z_imag[i] * z_imag[i];

            if (r2 + i2 > 4.0f) {
                data[i] = iteration; // Set cell in the part above the X-axis
                data[(height * width) - i - 1] = iteration; // Set cell in the part below the X-axis
                cell_count = cell_count - 1;
                continue;
            }

            z_imag[i] = 2.0f * z_real[i] * z_imag[i] + y_start;
            z_real[i] = r2 - i2 + (x_start + i * dx);
        }
    }

    return data;

#if 0
    for (int b_index = 0; b_index < std::ceil((float) height * width / BLOCK_SIZE); ++b_index) {
        int i_index = (b_index * BLOCK_SIZE) / width;
        float imag = y_start + i_index * dy;

        for (int cell_index = 0; cell_index < ; cell_index++) {

        }

        int cell_count = width;
        for (int iteration = 0; iteration < limit; ++iteration) {
            for (int r_index = 0; r_index < width; r_index++) {
                if (data[r_index] != limit) { // Skip already calculated cells
                    continue;
                }

                // Calculate next iteration
                float r2 = z_real[r_index] * z_real[r_index];
                float i2 = z_imag[r_index] * z_imag[r_index];

                if (r2 + i2 > 4.0f) {
                    data[r_index] = iteration; // Set cell in the part above the X-axis
                    data[height * width - r_index - 1] = iteration; // Set cell in the part below the X-axis
                    cell_count = cell_count - 1;
                    continue;
                }

                z_imag[r_index] = 2.0f * z_real[r_index] * z_imag[r_index] + y_start;
                z_real[r_index] = r2 - i2 + (x_start + r_index * dx);
            }
            if (!cell_count) break;
        }

    }
#endif
}