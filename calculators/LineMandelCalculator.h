/**
 * @file LineMandelCalculator.h
 * @author Zdenek Lapes <xlapes02@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date 3.11.2023
 */

#include <BaseMandelCalculator.h>

#define VECTORIZE_MODE 1
#define DEBUG_LITE  1   // DEBUG_PRINT_LITE
#define DEBUG_PRINT_LITE(fmt, ...) \
            do { if (DEBUG_LITE) fprintf(stderr, fmt, __VA_ARGS__); } while (0)


constexpr int SIMD_WIDTH = 64;

class LineMandelCalculator : public BaseMandelCalculator
{
public:
    LineMandelCalculator(unsigned matrixBaseSize, unsigned limit);
    ~LineMandelCalculator();
    int *calculateMandelbrot();

private:
    // @TODO add all internal parameters
    int *data;
    float *z_real;
    float *z_imag;
};