#include <stdio.h>
#include <stdlib.h>
#include "convolution.h"

Matrix* create_matrix(int width, int height) {
    Matrix* matrix = (Matrix*)malloc(sizeof(Matrix));
    matrix->width = width;
    matrix->height = height;
    
    matrix->data = (double**)malloc(height * sizeof(double*));
    for (int i = 0; i < height; i++) {
        matrix->data[i] = (double*)malloc(width * sizeof(double));
    }
    
    return matrix;
}

void free_matrix(Matrix* matrix) {
    if (matrix) {
        for (int i = 0; i < matrix->height; i++) {
            free(matrix->data[i]);
        }
        free(matrix->data);
        free(matrix);
    }
}

Matrix* copy_matrix(const Matrix* src) {
    Matrix* dst = create_matrix(src->width, src->height);
    
    for (int i = 0; i < src->height; i++) {
        for (int j = 0; j < src->width; j++) {
            dst->data[i][j] = src->data[i][j];
        }
    }
    
    return dst;
}

void print_matrix(const Matrix* matrix) {
    for (int i = 0; i < matrix->height; i++) {
        for (int j = 0; j < matrix->width; j++) {
            printf("%8.3f ", matrix->data[i][j]);
        }
        printf("\n");
    }
}

ConvKernel* create_kernel(int size) {
    ConvKernel* kernel = (ConvKernel*)malloc(sizeof(ConvKernel));
    kernel->size = size;
    
    kernel->kernel = (double**)malloc(size * sizeof(double*));
    for (int i = 0; i < size; i++) {
        kernel->kernel[i] = (double*)malloc(size * sizeof(double));
    }
    
    double value = 1.0 / (size * size);
    
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            kernel->kernel[x][y] = value;
        }
    }
    
    return kernel;
}

void free_kernel(ConvKernel* kernel) {
    if (kernel) {
        for (int i = 0; i < kernel->size; i++) {
            free(kernel->kernel[i]);
        }
        free(kernel->kernel);
        free(kernel);
    }
}

double convolve_pixel(const Matrix* src, const ConvKernel* kernel, int x, int y) {
    int kernel_radius = kernel->size / 2;
    double result = 0.0;
    
    for (int i = -kernel_radius; i <= kernel_radius; i++) {
        for (int j = -kernel_radius; j <= kernel_radius; j++) {
            int src_x = x + i;
            int src_y = y + j;
            
            if (src_x < 0) src_x = -src_x;
            if (src_y < 0) src_y = -src_y;
            if (src_x >= src->height) src_x = 2 * src->height - src_x - 1;
            if (src_y >= src->width) src_y = 2 * src->width - src_y - 1;
            
            double pixel_value = src->data[src_x][src_y];
            double kernel_value = kernel->kernel[i + kernel_radius][j + kernel_radius];
            
            result += pixel_value * kernel_value;
        }
    }
    
    return result;
}

void convolution(const Matrix* src, Matrix* dst, const ConvKernel* kernel) {
    for (int i = 0; i < src->height; i++) {
        for (int j = 0; j < src->width; j++) {
            dst->data[i][j] = convolve_pixel(src, kernel, i, j);
        }
    }
}