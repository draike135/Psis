#include <pthread.h>

// Define the global grid and mutex
int grid[20][20];
pthread_mutex_t grid_lock = PTHREAD_MUTEX_INITIALIZER;
int term_flag;
pthread_mutex_t data_lock;
