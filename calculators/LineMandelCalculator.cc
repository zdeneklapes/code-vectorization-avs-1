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
    z_real2 = (float *) (aligned_alloc(64, width * sizeof(float)));
    z_imag2 = (float *) (aligned_alloc(64, width * sizeof(float)));
}

LineMandelCalculator::~LineMandelCalculator() {
    // @TODO cleanup the memory
    free(data);
    free(z_real);
    free(z_imag);
    free(z_real2);
    free(z_imag2);
    data = NULL;
    z_real = NULL;
    z_imag = NULL;
    z_real2 = NULL;
    z_imag2 = NULL;
}

// SIMD = Single Instruction / Multiple Data

int *LineMandelCalculator::calculateMandelbrot() {
    // @TODO implement the calculator & return array of integers
    int half_of_height = height / 2;

    for (int row_index_2d = 0; row_index_2d <= half_of_height; row_index_2d++) {
        int row_index_1d = row_index_2d * width;
        float imag = y_start + row_index_2d * dy;
        int cpRowIndex = (height - row_index_2d - 1) * width;

#if VECTORIZE_MODE == 0
// without vectorization
#elif VECTORIZE_MODE == 1
#pragma omp for simd
#elif VECTORIZE_MODE == 2
#pragma omp for simd simdlen(64)
#endif
        // Calculate real and imaginary values for each pixel in the row
        for (int j = 0; j < width; j++) {
            z_real[j] = x_start + j * dx; // current real value
            z_imag[j] = imag;             // current imaginary value
//            z_real2[j] = z_real[j] * z_real[j];
//            z_imag2[j] = z_imag[j] * z_imag[j];
        }


#if VECTORIZE_MODE == 0
// without vectorization
#elif VECTORIZE_MODE == 1
#pragma omp for simd
#elif VECTORIZE_MODE == 2
#pragma omp for simd simdlen(64)
#endif
        // Calculate mandelbrot values for each pixel in the row
        for (int j = 0; j < width; j++) {
            data[row_index_1d + j] = limit;
        }

        int sum = width;
        for (int iteration = 0; iteration < limit; ++iteration) {
#if VECTORIZE_MODE == 0
// without vectorization
#elif VECTORIZE_MODE == 1
#pragma omp simd
#elif VECTORIZE_MODE == 2
#pragma omp simd reduction(-: sum) simdlen(64)
#elif VECTORIZE_MODE == 3
#pragma omp simd reduction(-: sum) simdlen(64)
#endif
            for (int j = 0; j < width; j++) {
                if (data[row_index_1d + j] == limit) {
                    float r2 = z_real[j] * z_real[j];
                    float i2 = z_imag[j] * z_imag[j];

                    if (z_real2[j] + z_imag2[j] > 4.0f) { // if diverged -> save iteration
                        data[row_index_1d + j] = iteration;
                        sum = sum - 1;
                    } else { // if not diverged -> calculate next iteration
                        z_imag[j] = 2.0f * z_real[j] * z_imag[j] + imag;
                        z_real[j] = z_real2[j] - z_imag2[j] + (x_start + j * dx);
                    }
                }
            }

            // If all pixels in the row have been calculated, break the loop
            if (!sum) break;
        }

#if VECTORIZE_MODE == 0
// without vectorization
#elif VECTORIZE_MODE == 1
#pragma omp simd
#elif VECTORIZE_MODE == 2
#pragma omp simd simdlen(64)
#endif
        // Copy the calculated row to the mirrored row
        for (int j = 0; j < width; j++) {
            data[cpRowIndex + j] = data[row_index_1d + j];
        }
    }

    return data;
}
