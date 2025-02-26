#include <ncurses.h>
#include "aliens.h"
#include <stdlib.h> // For srand and rand
#include <time.h>   // For time
#include <zmq.h>
#include <pthread.h>
#include "remote-char.h"


/*
 * Function: populate_aliens
 * --------------------------
 * Randomly places aliens (`*`) on the grid within a restricted area (orange part) and updates the display.
 *
 * Arguments:
 * - WINDOW* grid_win: The ncurses window representing the grid where aliens are displayed.
 * - int grid[GRID_SIZE][GRID_SIZE]: A 2D array representing the grid state, where 1 indicates an alien.
 *
 * Behavior:
 * - Defines an inner area (orange part) where aliens can be placed:
 *     - Boundaries are from row/column 2 to GRID_SIZE - 3.
 * - Calculates the total number of aliens to be placed as 1/3 of the total grid cells.
 * - Uses a random number generator (seeded with `8`) to select positions for aliens.
 * - Ensures no duplicate placements by checking if a position is already occupied.
 * - Marks selected positions in the `grid` array and visually places aliens (`*`) in `grid_win`.
 * - Refreshes the window after all aliens are placed.
 *
 * Output:
 * - Updates the `grid` array with alien positions.
 * - Visually places aliens in the `grid_win` window.
 */

void populate_aliens(WINDOW* grid_win, int grid[GRID_SIZE][GRID_SIZE]) {
    // Define inner area boundaries (orange part)
    int inner_start = 2;                  // Starting row/column of the orange area
    int inner_end = GRID_SIZE - 3;        // Ending row/column of the orange area
/*
    int total_inner_cells = (inner_end - inner_start + 1) * (inner_end - inner_start + 1);
    int total_aliens = total_inner_cells / 3; // 1/3 of the orange area
*/
    int total_cells = GRID_SIZE * GRID_SIZE;
    int total_aliens = total_cells/3; // 1/3 of the grid

    int placed_aliens = 0;

    srand(8); // Initialize random seed

    while (placed_aliens < total_aliens) {

        // Generate random positions within the orange area
        int x = rand() % (inner_end - inner_start + 1) + inner_start; // Rows restricted to orange
        int y = rand() % (inner_end - inner_start + 1) + inner_start; // Columns restricted to orange

        // Check if the position is already occupied
        if (grid[x][y] == 0) {
            grid[x][y] = 1; // Mark the position as occupied by an alien
            mvwaddch(grid_win, x + 1, y + 1, '*'); // Place an alien on the grid
            placed_aliens++;
        }
    }

    wrefresh(grid_win);
}
/*
 * Function: draw_aliens
 * ----------------------
 * Draws aliens (`*`) on the grid window and checks if any aliens remain active.
 * Displays "GAME OVER!" if no aliens are left.
 *
 * Arguments:
 * - WINDOW* grid_win: The ncurses window representing the grid.
 * - int grid[GRID_SIZE][GRID_SIZE]: A 2D array representing the grid state, where 1 indicates an alien.
 *
 * Behavior:
 * - Iterates over a 16x16 area of the grid (from row/column 2 to 17).
 * - For each cell:
 *     - If `grid[i][j]` is 1, places an alien (`*`) in `grid_win` and sets `end` to 1 (aliens are present).
 *     - Otherwise, clears the cell in `grid_win`.
 * - If no aliens are found (`end == 0`):
 *     - Displays the "GAME OVER!" message in the middle of the grid.
 *     - Returns 1 to indicate the game is over.
 * - Refreshes the window after updating the display.
 *
 * Output:
 * - Updates the `grid_win` display to show the current state of the grid.
 * - Returns 1 if no aliens remain (game over), otherwise returns 0.
 */

int draw_aliens(WINDOW* grid_win, int grid[GRID_SIZE][GRID_SIZE]) {
  char gameover[11]="GAME OVER!";
  int end=0;
  for (int i = 2; i < 18; i++) {
      for (int j = 2; j < 18; j++) {
        if (grid[i][j] == 1) {
            end=1;
            mvwaddch(grid_win, i + 1, j + 1, '*'); // Place an alien on the grid
        }else{
          mvwaddch(grid_win, i + 1, j + 1, ' ');
        }
      }
    }
    wrefresh(grid_win);
    if (end==0)
    {
      for(int x=0;x<10;x++)
      {
          mvwaddch(grid_win, 10, x+5, gameover[x]);
      }
      wrefresh(grid_win);
      return 1;
    }else
      return 0;
}
/*
 * Function: fork_aliens
 * ----------------------
 * Manages the communication and movement of aliens by sending periodic messages to a father server.
 *
 * Behavior:
 * - Initializes a ZeroMQ context and a `ZMQ_REQ` socket to communicate with a remote server.
 * - Generates a random initial direction for the alien using `rand()`.
 * - Constructs and sends a `remote_char_t` message to the remote server every second.
 * - Receives responses from the server to update the alien's state.
 *
 * Arguments:
 * - None (hardcoded configurations for ZeroMQ connection and alien behavior).
 *
 * Output:
 * - Continuously sends and receives messages from the server to simulate alien movement.
 */
void fork_aliens()
{
  srand(time(NULL));
  int random_dir = rand() % 4;
  void *context2 = zmq_ctx_new();
  void* child_socket = zmq_socket(context2, ZMQ_REQ);
  zmq_connect(child_socket, FORK_IP);
  remote_char_t m_child;
  m_child.msg_type = 4; // Initial message type for connection
  m_child.ch=' ';
  m_child.direction = (direction_t)(random_dir);
  time_t start_time = time(NULL);
  while (1) {
      time_t current_time = time(NULL);
      if (current_time - start_time >= 1) {
          // One second has passed
          zmq_send(child_socket, &m_child, sizeof(remote_char_t), 0);
          zmq_recv(child_socket, &m_child, sizeof(remote_char_t), 0);
          start_time = current_time; // Reset start_time
      }
  }
  zmq_close(child_socket);
  zmq_ctx_destroy(context2);
}

/*
 * Function: move_aliens
 * ----------------------
 * Moves aliens randomly on a 2D grid while ensuring valid positions and avoiding overlaps.
 *
 * Arguments:
 * - int grid[20][20]: A 2D array representing the grid state, where 1 indicates an alien.
 * - direction_t random_dir: A seed value used to randomize the movement direction.
 *
 * Behavior:
 * - Defines direction offsets for UP, DOWN, LEFT, and RIGHT.
 * - Iterates over a restricted 16x16 section of the grid (from row/column 2 to 17).
 * - For each alien found (`grid[i][j] == 1`):
 *     - Generates a random direction using `rand()` seeded by `random_dir`.
 *     - Computes a new position `(new_x, new_y)` based on the random direction.
 *     - Checks if the new position is within bounds and empty (`grid[new_x][new_y] == 0`).
 *     - Moves the alien to the new position and clears the old position.
 *
 * Output:
 * - Updates the `grid` array to reflect new alien positions.
 */

void move_aliens(int grid[20][20],direction_t random_dir) {
    // Initialize random seed
    srand(random_dir);

    // Direction offsets for UP, DOWN, LEFT, RIGHT
    int directions[4][2] = {
        {-1, 0}, // UP
        {1, 0},  // DOWN
        {0, -1}, // LEFT
        {0, 1}   // RIGHT
    };

    for (int i = 2; i < 18; i++) {
        for (int j = 2; j < 18; j++) {
            if (grid[i][j] == 1) { // Alien found at grid[i][j]

                // Generate a random direction
                int random_dir = rand() % 4;
                int new_x = i + directions[random_dir][0];
                int new_y = j + directions[random_dir][1];

                // Check if the new position is within bounds and empty
                if (new_x >= 2 && new_x < 18 && new_y >= 2 && new_y < 18 && grid[new_x][new_y] == 0) {
                    // Move the alien to the new position
                    grid[new_x][new_y] = 1;

                    // Clear the old position
                    grid[i][j] = 0;
                }
            }
        }
    }
}

void* move_aliens_thread(void* args) {
    thread_struct *data = (thread_struct *)args;
    message_display m_disp;
    srand(time(NULL)); // Initialize random seed
    time_t start_time = time(NULL);
    time_t start_time_10 = time(NULL);
    int n_aliens;
    int n_aliens_start=(20*20)/3;
    // Direction offsets for UP, DOWN, LEFT, RIGHT
    int directions[4][2] = {
        {-1, 0}, // UP
        {1, 0},  // DOWN
        {0, -1}, // LEFT
        {0, 1}   // RIGHT
    };
    int new_aliens=0;
    while (1) {
      if (term_flag==1)
      {
        break;
      }

        time_t current_time = time(NULL);
        time_t current_time_10 = time(NULL);

        if(current_time_10 - start_time_10 >= 10){
          n_aliens=0;
          for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 20; j++) {
              if (grid[i][j]==1){
                n_aliens++;
              }
            }
          }
          //printf("%d %d\n",n_aliens,n_aliens_start);

          if(n_aliens>=n_aliens_start){
            new_aliens=(int) (n_aliens*0.1);
            n_aliens_start=n_aliens+new_aliens;
            for (int i = 0; i < 20; i++) {
              for (int j = 0; j < 20; j++) {
                if (grid[i][j]==0 && new_aliens>0){
                  new_aliens--;
                  grid[i][j]=1;

                }
              }
            }
          }
          else{
            n_aliens_start=n_aliens;
          }

          start_time_10 = current_time_10;

        }
        if (current_time - start_time >= 1) {
            pthread_mutex_lock(&grid_lock);
            for (int i = 2; i < 18; i++) {
                for (int j = 2; j < 18; j++) {
                    if (grid[i][j] == 1) { // Alien found at grid[i][j]

                        // Generate a random direction
                        int random_dir = rand() % 4;
                        int new_x = i + directions[random_dir][0];
                        int new_y = j + directions[random_dir][1];

                        // Check if the new position is within bounds and empty
                        if (new_x >= 2 && new_x < 18 && new_y >= 2 && new_y < 18 && grid[new_x][new_y] == 0) {
                            // Move the alien to the new position
                            grid[new_x][new_y] = 1;

                            // Clear the old position
                            grid[i][j] = 0;
                        }
                    }
                }
            }
            draw_aliens(data->grid_win,grid);
            for (int i = 0; i < 20; i++) {
              for (int j = 0; j < 20; j++) {
                m_disp.grid_1[i][j]=grid[i][j];

              }
            }
            //printf("%d\n", data->ch_data[2].score);
            pthread_mutex_lock(&data_lock);
            for (size_t k = 0; k < 8; k++) {
              m_disp.ch_data[k]=data->ch_data[k];
              m_disp.act_pl[k]=data->act_pl[k];
            }
             pthread_mutex_unlock(&data_lock);

            zmq_send(data->pub, &m_disp, sizeof(message_display), 0); //send display message
            pthread_mutex_unlock(&grid_lock);

            start_time = current_time; // Reset the start time after each second
        }
    }
    return NULL;
}
