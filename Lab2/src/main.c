#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "convolution.h"
#include "utils.h"

void sequential_convolution(Matrix* matrix, const ConvKernel* kernel, int iterations);
void parallel_convolution(Matrix* matrix, const ConvKernel* kernel, int iterations, int num_threads);

int main(int argc, char* argv[]) {
    pid_t pid = getpid();
    size_t n_threads = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Логических ядер: %ld\n", n_threads);
    printf("%d\n", pid);

    int width = 20, height = 20, iterations = 20, kernel_size = 5, max_threads = 4;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            width = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            height = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            kernel_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            max_threads = atoi(argv[++i]);
        }
    }
    
    Matrix* matrix = create_matrix(width, height);
    fill_matrix_random(matrix);
    //print_matrix(matrix);
    //printf("\n");
    
    ConvKernel* kernel = create_kernel(kernel_size);
    
    Timer timer;
    double sequential_time, parallel_time;
    
    // Последовательная версия
    Matrix* seq_matrix = copy_matrix(matrix);
    
    start_timer(&timer);
    sequential_convolution(seq_matrix, kernel, iterations);
    stop_timer(&timer);

    sequential_time = get_spent_time(&timer);
    printf("Время выполнения: %.6f секунд\n", sequential_time);
    
    // Параллельная версия
    for (int num_threads = 1; num_threads <= max_threads; num_threads++) {
        Matrix* par_matrix = copy_matrix(matrix);
        
        start_timer(&timer);
        parallel_convolution(par_matrix, kernel, iterations, num_threads);
        stop_timer(&timer);
        
        parallel_time = get_spent_time(&timer);
        double speedup = sequential_time / parallel_time;
        double efficiency = speedup / num_threads;
        
        printf("Потоков: %d, Время: %.6f сек, Ускорение: %.2fx, Эффективность: %.3f\n",
               num_threads, parallel_time, speedup, efficiency);
        
        free_matrix(par_matrix);
    }
    
    free_matrix(matrix);
    free_matrix(seq_matrix);
    free_kernel(kernel);
    
    return 0;
}