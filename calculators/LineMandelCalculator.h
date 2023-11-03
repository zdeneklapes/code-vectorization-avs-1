/**
 * @file LineMandelCalculator.h
 * @author FULL NAME <xlogin00@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date DATE
 */

#include <BaseMandelCalculator.h>

#define VECTORIZE_MODE 2
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
    float *z_real2;
    float *z_imag2;
};