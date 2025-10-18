#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "utils.h"

void start_timer(Timer* timer) {
    gettimeofday(&timer->start_time, NULL);
}

void stop_timer(Timer* timer) {
    gettimeofday(&timer->end_time, NULL);
}

double get_spent_time(const Timer* timer) {
    double start = timer->start_time.tv_sec + timer->start_time.tv_usec / 1000000.0;
    double end = timer->end_time.tv_sec + timer->end_time.tv_usec / 1000000.0;
    return end - start;
}

void fill_matrix_random(Matrix* matrix) {
    srand(time(NULL));
    for (int i = 0; i < matrix->height; i++) {
        for (int j = 0; j < matrix->width; j++) {
            matrix->data[i][j] = (double)rand() / RAND_MAX * 100.0;
        }
    }
}