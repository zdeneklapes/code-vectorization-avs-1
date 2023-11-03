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
    // @TODO implement the calculator & return array of integers
    const int h2 = (float) height / 2;

    const int mainGroupXBlockStart = std::ceil((float) std::ceil(mainCluterRStart * width) / BLOCK_SIZE) + 1;
    const int mainGroupXBlockEnd = std::floor((float) std::floor(mainCluterREnd * width) / BLOCK_SIZE) - 1;
    const int mainGroupYStart = std::ceil(mainCluterIStart * height);
    const int mainGroupYEnd = std::floor(mainCluterIEnd * height);

    //#pragma omp parallel for
    for (int i = 0; i <= h2; i++) {

        const int rowIndex = i * width;
        const float imag = y_start + i * dy;

        const int cpRowIndex = (height - i - 1) * width;

        //init values
#pragma omp simd simdlen(64)
        for (int j = 0; j < width; j++) {
            z_real[j] = x_start + j * dx; // current real value
            z_imag[j] = imag;             // current imaginary value
        }

        //fill empty
#pragma omp simd simdlen(64)
        for (int j = 0; j < width; j++) {
            data[rowIndex + j] = limit;
        }

        for (int block = 0; block < std::ceil(((float) width) / BLOCK_SIZE); block++) {

            //drop it is in bandelbrot set
            if (block >= mainGroupXBlockStart && block <= mainGroupXBlockEnd && i >= mainGroupYStart &&
                i <= mainGroupYEnd) {
                continue;
            }

            const int blockStart = BLOCK_SIZE * block;
            int blockEnd = BLOCK_SIZE * block + BLOCK_SIZE;
            int sum = BLOCK_SIZE;

            //last iteratin
            if (blockEnd > width) {
                blockEnd = width;
            }

            for (int iteration = 0; iteration < limit; ++iteration) {

#pragma omp simd reduction(-: sum) simdlen(64)
                for (int j = blockStart; j < blockEnd; j++) {

                    if (data[rowIndex + j] == limit) {
                        float r2 = z_real[j] * z_real[j];
                        float i2 = z_imag[j] * z_imag[j];

                        if (r2 + i2 > 4.0f) {
                            data[rowIndex + j] = iteration;
                            sum = sum - 1;
                        } else {
                            z_imag[j] = 2.0f * z_real[j] * z_imag[j] + imag;
                            z_real[j] = r2 - i2 + (x_start + j * dx);
                        }
                    }
                }

                if (!sum) break;
            }
        }

        //copy
#pragma omp simd simdlen(64)
        for (int j = 0; j < width; j++) {
            data[cpRowIndex + j] = data[rowIndex + j];
        }
    }

    return data;
}