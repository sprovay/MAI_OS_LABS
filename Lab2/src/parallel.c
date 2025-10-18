#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "convolution.h"
#include "utils.h"

typedef struct {
    Matrix* src;
    Matrix* dst;
    const ConvKernel* kernel;
    int start_row;
    int end_row;
} ThreadData;

void* process_rows(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 0; j < data->src->width; j++) {
            data->dst->data[i][j] = convolve_pixel(data->src, data->kernel, i, j);
        }
    }
    
    pthread_exit(NULL);
}

void parallel_convolution(Matrix* matrix, const ConvKernel* kernel, int iterations, int num_threads) {
    Matrix* temp = create_matrix(matrix->width, matrix->height);
    Matrix* current_src = matrix;
    Matrix* current_dst = temp;
    
    for (int k = 0; k < iterations; k++) {
        pthread_t threads[num_threads];
        ThreadData thread_data[num_threads];
        
        int rows_per_thread = matrix->height / num_threads;
        int remaining_rows = matrix->height % num_threads;
        
        int current_row = 0;
        for (int t = 0; t < num_threads; t++) {
            thread_data[t].src = current_src;
            thread_data[t].dst = current_dst;
            thread_data[t].kernel = kernel;
            thread_data[t].start_row = current_row;
            
            int thread_rows = rows_per_thread + (t < remaining_rows ? 1 : 0);
            thread_data[t].end_row = current_row + thread_rows;
            current_row += thread_rows;
            
            pthread_create(&threads[t], NULL, process_rows, &thread_data[t]);
        }
        
        for (int t = 0; t < num_threads; t++) {
            pthread_join(threads[t], NULL);
        }
        
        Matrix* swap_temp = current_src;
        current_src = current_dst;
        current_dst = swap_temp;
    }
    
    if (current_src == temp) {
        for (int i = 0; i < matrix->height; i++) {
            for (int j = 0; j < matrix->width; j++) {
                matrix->data[i][j] = temp->data[i][j];
            }
        }
    }
    
    free_matrix(temp);
}