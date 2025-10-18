#include <stdio.h>
#include <stdlib.h>
#include "convolution.h"
#include "utils.h"

void sequential_convolution(Matrix* matrix, const ConvKernel* kernel, int iterations) {
    Matrix* temp = create_matrix(matrix->width, matrix->height);
    
    for (int k = 0; k < iterations; k++) {
        convolution(matrix, temp, kernel);
        
        for (int i = 0; i < matrix->height; i++) {
            for (int j = 0; j < matrix->width; j++) {
                matrix->data[i][j] = temp->data[i][j];
            }
        }
    }
    //print_matrix(matrix);
    free_matrix(temp);
}