#ifndef ALIENS_H
#define ALIENS_H

#include <ncurses.h>
#include "remote-char.h" // For direction_t
#define GRID_SIZE 20 // Size of the grid
#define GRID_SIZE_ASTRONAUTS 16 // Size of the grid for the astronauts
#define ASTRONAUTS_WIDTH 2 // Width of the grid for the astronauts
#define SCORE_WIDTH 20 // Width of the score panel


typedef struct alien{
    int x_alien;
    int y_alien;
}alien;

// Function prototypes
void populate_aliens(WINDOW* grid_win, int grid[GRID_SIZE][GRID_SIZE]);
direction_t random_direction();
void move_aliens(int grid[20][20],direction_t );
int draw_aliens(WINDOW* grid_win, int grid[GRID_SIZE][GRID_SIZE]);
void fork_aliens();

#endif // ALIENS_H
