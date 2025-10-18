#ifndef CONVOLUTION_H
#define CONVOLUTION_H

typedef struct {
    int width;
    int height;
    double** data;
} Matrix;

typedef struct {
    int size;
    double** kernel;
} ConvKernel;

Matrix* create_matrix(int width, int height);
void free_matrix(Matrix* matrix);
Matrix* copy_matrix(const Matrix* src);
void print_matrix(const Matrix* matrix);

ConvKernel* create_kernel(int size);
void free_kernel(ConvKernel* kernel);

void convolution(const Matrix* src, Matrix* dst, const ConvKernel* kernel);
double convolve_pixel(const Matrix* src, const ConvKernel* kernel, int x, int y);

#endif