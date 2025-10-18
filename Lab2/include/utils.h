#ifndef UTILS_H
#define UTILS_H


#include <sys/time.h>
#include "convolution.h"

typedef struct {
    struct timeval start_time;
    struct timeval end_time;
} Timer;

void start_timer(Timer* timer);
void stop_timer(Timer* timer);
double get_spent_time(const Timer* timer);

void fill_matrix_random(Matrix* matrix);

#endif