#ifndef TABLE_H
#define TABLE_H

#include <ncurses.h>
#include "remote-char.h"
// Function prototypes
void draw_grid(WINDOW* grid_win);
void draw_numbers(WINDOW* top_numbers_win, WINDOW* side_numbers_win);
void update_score(WINDOW* score_win,player char_data[],int n);
void draw_zap_line(WINDOW* grid_win, int pos_x, int pos_y, char player_char, player char_data[],int grid[20][20]);
message_display prepare_message_display(int actpl[MAX_PLAYERS],int grid[GRID_SIZE][GRID_SIZE],player ch[MAX_PLAYERS],remote_char_t m);
void draw_players(player ch[MAX_PLAYERS],WINDOW* grid_win,int act[MAX_PLAYERS]);
void* keyboard_listener(void* arg);
#endif // TABLE_H
