#include <ncurses.h>
#include "astronaut.h"


#define GRID_SIZE 20 // Size of the grid
#define GRID_SIZE_ASTRONAUTS 16 // Size of the grid for the astronauts
#define ASTRONAUTS_WIDTH 2 // Width of the grid for the astronauts
#define SCORE_WIDTH 20 // Width of the score panel

// Function to find the index of a character in a player array
// Arguments:
// - player char_data[]: An array of player structures (assumes a `character` field in the structure)
// - int n_char: The number of players (size of the `char_data` array)
// - int ch: The character to search for within the `character` field of each player
//
// Output:
// - Returns the index of the player whose `character` matches `ch`.
// - If no match is found, returns -1 to indicate failure.
int find_ch_info(player char_data[], int n_char, int ch) {
    for (int i = 0; i < n_char; i++) {
        if (ch == char_data[i].character) {
            return i;
        }
    }
    return -1;
}

/*
 * Function: new_position
 * -----------------------
 * Updates the position (x, y) based on a given letter and direction.
 *
 * Arguments:
 * - int* x: Pointer to the x-coordinate of the current position.
 * - int* y: Pointer to the y-coordinate of the current position.
 * - char letter: The letter determining the movement constraints.
 *                - 'A', 'E', 'D', 'G': Can move up or down only.
 *                - 'C', 'H', 'F', 'B': Can move left or right only.
 * - direction_t direction: The direction of movement.
 *                - UP: Move up (decrease x).
 *                - DOWN: Move down (increase x).
 *                - LEFT: Move left (decrease y).
 *                - RIGHT: Move right (increase y).
 *
 * Behavior:
 * - Ensures the position stays within a predefined "green area":
 *     - Minimum x: 3, Maximum x: 18
 *     - Minimum y: 3, Maximum y: 18
 * - Adjusts the position only in the allowed directions based on the letter.
 *
 * Output:
 * - The updated x and y values are written back via the pointers provided.
 */
void new_position(int* x, int* y, char letter, direction_t direction) {
    switch (letter) {
        // Letters A, E, C, G: Move up or down only
        case 'A':
        case 'E':
        case 'D':
        case 'G':
            if (direction == UP) {
                (*x)--; // Move up
                if (*x < 3) *x = 3; // Stay within the green area (minimum row = 3)
            } else if (direction == DOWN) {
                (*x)++; // Move down
                if (*x > 18) *x = 18; // Stay within the green area (maximum row = 19)
            }
            break;

        // Letters D, H, F, B: Move left or right only
        case 'C':
        case 'H':
        case 'F':
        case 'B':
            if (direction == LEFT) {
                (*y)--; // Move left
                if (*y < 3) *y = 3; // Stay within the green area (minimum column = 3)
            } else if (direction == RIGHT) {
                (*y)++; // Move right
                if (*y > 18) *y = 18; // Stay within the green area (maximum column = 19)
            }
            break;
    }
}

/*
 * Function: seconds_passed
 * -------------------------
 * Checks if a specified amount of time (in seconds) has passed since a given start time.
 *
 * Arguments:
 * - time_t start_time: The start time to measure from.
 * - int max_time: The maximum time in seconds to check against.
 *
 * Behavior:
 * - Retrieves the current time using `time(NULL)`.
 * - Compares the difference between the current time and the start time to `max_time`.
 *
 * Output:
 * - Returns 1: If `max_time` seconds or more have passed since `start_time`.
 * - Returns 0: If less than `max_time` seconds have passed since `start_time`.
 */
int seconds_passed(time_t start_time, int max_time) {
    time_t current_time = time(NULL); // Get the current time
    if (difftime(current_time, start_time) >= max_time) {

        return 1; // 3 seconds have passed
    }
    return 0; // Less than 3 seconds have passed
}

/*
 * Function: init_pl
 * ------------------
 * Initializes player data and active player arrays
 *
 * Arguments:
 * - player char_data[]: An array of player structures to be initialized.
 * - int act_player[]: An array indicating active player states to be initialized.
 *
 * Behavior:
 * - Iterates through all players (up to `MAX_PLAYERS`).
 * - Sets default values for each player's fields:
 *     - fire: 0 (no firing activity)
 *     - stun: 0 (not stunned)
 *     - position: [0, 0] (initial position at origin)
 *     - score: 0 (no score)
 * - Sets all active player states to 0 (inactive).
 *
 * Output:
 * - Modifies `char_data` and `act_player` arrays in-place with initial values.
 */
void init_pl(player char_data[],int act_player[])
{
  for (int i = 0; i < MAX_PLAYERS; i++) {
    // Initialize arrays
    char_data[i].fire = 0;
    act_player[i]=0;
    char_data[i].stun = 0;
    char_data[i].position[0] = 0;
    char_data[i].position[1] = 0;
    char_data[i].score=0;
  }
}
