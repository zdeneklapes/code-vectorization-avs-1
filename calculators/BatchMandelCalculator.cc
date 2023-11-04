/**
 * @file BatchMandelCalculator.cc
 * @author Zdenek Lapes <xlapes02@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over small batches
 * @date 3.11.2023
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
    data = (int *) (aligned_alloc(64, height * width * sizeof(int)));
    z_real = (float *) (aligned_alloc(64, BATCH_SIZE * sizeof(float)));
    z_imag = (float *) (aligned_alloc(64, BATCH_SIZE * sizeof(float)));
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
    #pragma omp simd simdlen(64)
    for (int i = 0; i < width * height; ++i) {
        data[i] = limit;
    }

    int i_count = height / 2;
//    #pragma omp simd simdlen(64)
    for (int i_index = 0; i_index < i_count; i_index++) {
        int i_index_from_top = i_index * width;
        int i_index_from_bottom = (height - i_index - 1) * width;
        float imag = y_start + i_index * dy;

        int batch_count = std::ceil((float) width / BATCH_SIZE);
//        #pragma omp simd simdlen(64)
        for (int batch_index = 0; batch_index < batch_count; batch_index++) {
            int r_index_start = batch_index * BATCH_SIZE;

            #pragma omp simd simdlen(64)
            for (int b_index = 0; b_index < BATCH_SIZE; b_index++) {
                int r_index = r_index_start + b_index;
                z_real[b_index] = x_start + r_index * dx;
                z_imag[b_index] = imag;
            }

            int cell_count = BATCH_SIZE;
            for (int iteration = 0; iteration < limit; iteration++) {
                #pragma omp simd reduction(-:cell_count) simdlen(64)
                for (int b_index = 0; b_index < BATCH_SIZE; b_index++) {
                    int r_index = r_index_start + b_index;

                    if (data[i_index_from_top + r_index] != limit) {
                        // Skip already calculated cells
                        continue;
                    }

                    // Calculate next iteration
                    float r2 = z_real[b_index] * z_real[b_index];
                    float i2 = z_imag[b_index] * z_imag[b_index];

                    if (r2 + i2 > 4.0f) {
                        // Set cell in the part above the X-axis
                        data[i_index_from_top + r_index] = iteration;
                        // Set cell in the part below the X-axis
                        data[i_index_from_bottom + r_index] = iteration;
                        cell_count = cell_count - 1;
                        continue;
                    }

                    z_imag[b_index] = 2.0f * z_real[b_index] * z_imag[b_index] + imag;
                    z_real[b_index] = r2 - i2 + (x_start + r_index * dx);
                }
                if (!cell_count) break;
            }
        }
    }

    return data;
}
