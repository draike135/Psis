#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>

// Declare the global grid and mutex
extern int grid[20][20];
extern pthread_mutex_t grid_lock;
extern int term_flag;
extern pthread_mutex_t data_lock;

#endif // GLOBALS_H
