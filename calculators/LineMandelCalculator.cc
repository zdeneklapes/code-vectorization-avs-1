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


#include "LineMandelCalculator.h"

constexpr int SIMD_WIDTH = 64;

LineMandelCalculator::LineMandelCalculator(unsigned matrixBaseSize, unsigned limit) :
        BaseMandelCalculator(matrixBaseSize, limit, "LineMandelCalculator") {
    // @TODO allocate & prefill memory
    data = (int *) (aligned_alloc(64, height * width * sizeof(int)));
    zReal = (float *) (aligned_alloc(64, width * sizeof(float)));
    zImag = (float *) (aligned_alloc(64, width * sizeof(float)));
}

LineMandelCalculator::~LineMandelCalculator() {
    // @TODO cleanup the memory
    free(data);
    free(zReal);
    free(zImag);
    data = NULL;
    zReal = NULL;
    zImag = NULL;
}

int *LineMandelCalculator::calculateMandelbrot() {
    // @TODO implement the calculator & return array of integers
    int height_div_2 = height / 2;

    for (int row_index_2d = 0; row_index_2d <= height_div_2; row_index_2d++) {
        int row_index_1d = row_index_2d * width;
        float imag = y_start + row_index_2d * dy;
        int cpRowIndex = (height - row_index_2d - 1) * width;

        //init values
        // SIMD = Single Instruction / Multiple Data
#pragma omp for simd simdlen(64)
        for (int j = 0; j < width; j++) {
            zReal[j] = x_start + j * dx; // current real value
            zImag[j] = imag;             // current imaginary value
        }

        //fill empty
#pragma omp for simd simdlen(64)
        for (int j = 0; j < width; j++) {
            data[row_index_1d + j] = limit;
        }

        int sum = width;
        for (int iteration = 0; iteration < limit; ++iteration) {
#pragma omp simd reduction(-: sum) simdlen(64)
            for (int j = 0; j < width; j++) {
                if (data[row_index_1d + j] == limit) {
                    float r2 = zReal[j] * zReal[j];
                    float i2 = zImag[j] * zImag[j];

                    if (r2 + i2 > 4.0f) {
                        data[row_index_1d + j] = iteration;
                        sum = sum - 1;
                    } else {
                        zImag[j] = 2.0f * zReal[j] * zImag[j] + imag;
                        zReal[j] = r2 - i2 + (x_start + j * dx);
                    }
                }
            }

            if (!sum) break;
        }

        //copy
#pragma omp simd simdlen(64)
        for (int j = 0; j < width; j++) {
            data[cpRowIndex + j] = data[row_index_1d + j];
        }

    }
    return data;
}
