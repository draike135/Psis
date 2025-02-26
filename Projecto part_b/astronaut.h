#ifndef ASTRONAUT_H
#define ASTRONAUT_H

#include <ncurses.h>
#include "remote-char.h"
// Function prototypes
int find_ch_info(player char_data[], int n_char, int ch);
void new_position(int* x, int* y, char letter, direction_t direction); // Placeholder function (adjust based on actual implementation)
int seconds_passed(time_t start_time, int max_time);
void init_pl(player char_data[],int act_player[]);

#endif // ASTRONAUT_H
