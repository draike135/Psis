#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <zmq.h>
#include "aliens.h"
#include "astronaut.h"
#include "table.h"
#include "remote-char.h"
#include "globals.h"

#define GRID_SIZE 20 // Size of the grid
#define GRID_SIZE_ASTRONAUTS 16 // Size of the grid for the astronauts
#define ASTRONAUTS_WIDTH 2 // Width of the grid for the astronauts
#define SCORE_WIDTH 20

typedef struct {
    void *socket;          // ZeroMQ socket
    char assigned_char;    // Character assigned to this astronaut
    int quit_flag;         // Shared flag to signal when to quit
    pthread_mutex_t *quit_mutex; // Mutex for accessing the quit_flag
} thread_args_t;
int int_poss[8][2]={{3,1},{1,3},{2,3},{3,2},{3,20},{20,3},{3,19},{19,3}};
char icons[10]="ABCDEFGH";
void init_variables(player ch[MAX_PLAYERS],int grid[20][20],int act[MAX_PLAYERS],message_display m_disp)
{
  for (int k=0;k<20;k++)
  {
    for(int h=0;h<20;h++)
    {
      grid[k][h]= m_disp.grid_1[k][h];
    }
    if (k<8)
    {
      ch[k]=  m_disp.ch_data[k];
      act[k]=m_disp.act_pl[k];
    }
  }
}


void* astronaut_client_thread(void *args) {
    thread_args_t *data = (thread_args_t *)args;
    void *socket = data->socket;
    char assigned_char = data->assigned_char;

    remote_char_t message;
    message.ch = assigned_char;

    int key;
    while (1) {
        // Check the quit_flag to see if we should exit
        pthread_mutex_lock(data->quit_mutex);
        if (data->quit_flag) {
            pthread_mutex_unlock(data->quit_mutex);
            break;
        }
        pthread_mutex_unlock(data->quit_mutex);

        // Get user input (non-blocking)
        key = getch();
        if (key == ERR) {
            continue; // No key pressed, continue to next iteration
        }

        // Determine the message type and direction based on key press
        if (key == KEY_LEFT) {
            message.direction = LEFT;
            message.msg_type = 1; // Movement message
        } else if (key == KEY_RIGHT) {
            message.direction = RIGHT;
            message.msg_type = 1;
        } else if (key == KEY_UP) {
            message.direction = UP;
            message.msg_type = 1;
        } else if (key == KEY_DOWN) {
            message.direction = DOWN;
            message.msg_type = 1;
        } else if (key == ' ') {
            message.msg_type = 2; // Zap action
        } else if (key == 'q' || key == 'Q') {
            pthread_mutex_lock(data->quit_mutex);
            data->quit_flag = 1; // Signal other threads to quit
            pthread_mutex_unlock(data->quit_mutex);
            message.msg_type = 3; // Quit message
        } else {
            continue; // Ignore other keys
        }

        // Send the message to the server
        zmq_send(socket, &message, sizeof(remote_char_t), 0);


        // Wait for server acknowledgment (optional)
        zmq_recv(socket, &message, sizeof(remote_char_t), 0);

        if (message.msg_type == -1 || message.msg_type == 3) {
            break;
        }
    }

    return NULL;
}

// Function for the display thread
void* display_thread(void *args) {
    thread_args_t *data = (thread_args_t *)args;
    int grid_os1[20][20];
    int n_chars = 0;
    void *context = zmq_ctx_new();
    void *sub_socket = zmq_socket(context, ZMQ_SUB);
    zmq_connect(sub_socket, "tcp://localhost:6028");
    zmq_setsockopt(sub_socket, ZMQ_SUBSCRIBE, "", 0);

    player char_data[8];
    int act_player[9];


    remote_char_t m;
    message_display m_disp;

    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    // Create windows
    WINDOW* top_numbers_win = newwin(1, GRID_SIZE+1 , 0, 2); // Top numbers for columns
    WINDOW* side_numbers_win = newwin(GRID_SIZE+1 , 1, 1, 0); // Side numbers for rows
    WINDOW* grid_win = newwin(GRID_SIZE+2, GRID_SIZE+2, 1, 2);     // Grid window (inside numbers)
    WINDOW* score_win = newwin(GRID_SIZE+2 , SCORE_WIDTH+2, 1, GRID_SIZE + 5); // Score window
    refresh();

    // Draw the grid and numbers
    draw_grid(grid_win);
    draw_numbers(top_numbers_win, side_numbers_win);


    // Create a grid representation to track alien positions
     // 0 means empty, 1 means occupied by an alien

    // Populate 1/3 of the grid with aliens

    draw_aliens(grid_win,grid);
    update_score(score_win, char_data, 8);

    // Create a debug window below the main grid
    /*
    WINDOW* debug_win = newwin(10, 50, GRID_SIZE + 3, 2); // Debug window
    box(debug_win, 0, 0);
    mvwprintw(debug_win, 0, 1, "DEBUG");
    wrefresh(debug_win);
    */

    while (1) {

        //printf(" test teste %c \n", char_data[0].character);

        pthread_mutex_lock(data->quit_mutex);
        if (data->quit_flag) {
            pthread_mutex_unlock(data->quit_mutex);
            break;
        }
        pthread_mutex_unlock(data->quit_mutex);

        zmq_recv(sub_socket, &m_disp, sizeof(m_disp), 0);

        init_variables(char_data,grid,act_player,m_disp);
        draw_aliens(grid_win,m_disp.grid_1);
        update_score(score_win, m_disp.ch_data, 8);
        draw_players( m_disp.ch_data,grid_win,m_disp.act_pl);
        m=m_disp.copy_message;
        /*
        werase(debug_win); // Clear debug window
        box(debug_win, 0, 0); // Redraw the border
        mvwprintw(debug_win, 0, 1, "DEBUG");
        mvwprintw(debug_win, 1, 1, "Received message: msg_type=%d, char=%c", m.msg_type, m.ch);
        wrefresh(debug_win);
        */
        if (m.msg_type == 9) {
          pthread_mutex_lock(data->quit_mutex);
          data->quit_flag = 1; // Signal other threads to quit
          pthread_mutex_unlock(data->quit_mutex);
          break;
        }

        if (m.msg_type == 0) {
          for(int c=0;c<8;c++)
          {
            if (act_player[c]==5)
            {
            char_data[c].position[0] = int_poss[c][0];
            char_data[c].position[1] = int_poss[c][1];
            char_data[c].character = icons[c];
            m.ch = icons[c];
            char_data[c].score = 0;

            grid_os1[char_data[c].position[0]][char_data[c].position[1]] = 1; // Mark grid as occupied
            update_score(score_win, char_data, 8);
            update_score(score_win, m_disp.ch_data, 8);
            //draw_players( m_disp.ch_data,grid_win,m_disp.act_pl);
            wmove(grid_win, char_data[c].position[0], char_data[c].position[1]);
            waddch(grid_win, char_data[c].character | A_BOLD);
            wrefresh(grid_win);

            n_chars++; // Increment player count
            act_player[c]=1;
            break;
            }
          }
        }else if (m.msg_type == 1) {
          int ch_pos = find_ch_info(char_data,8, m.ch);
          if(char_data[ch_pos].stun == 0){
            if (ch_pos != -1) {
                int pos_x = char_data[ch_pos].position[0];
                int pos_y = char_data[ch_pos].position[1];
                int ch = char_data[ch_pos].character;

                wmove(grid_win, pos_x, pos_y);
                waddch(grid_win, ' ');

                new_position(&pos_x, &pos_y, ch, m.direction);
                char_data[ch_pos].position[0] = pos_x;
                char_data[ch_pos].position[1] = pos_y;

                wmove(grid_win, pos_x, pos_y);
                waddch(grid_win, ch | A_BOLD);
                wrefresh(grid_win);

              }
          }else if (seconds_passed(char_data[ch_pos].start_time_stun, 10)) {
                int pos_x = char_data[ch_pos].position[0];
                int pos_y = char_data[ch_pos].position[1];
                int ch = char_data[ch_pos].character;

                wmove(grid_win, pos_x, pos_y);
                waddch(grid_win, ' ');

                new_position(&pos_x, &pos_y, ch, m.direction);
                char_data[ch_pos].position[0] = pos_x;
                char_data[ch_pos].position[1] = pos_y;

                wmove(grid_win, pos_x, pos_y);
                waddch(grid_win, ch | A_BOLD);
                wrefresh(grid_win);
        }
        }else if (m.msg_type == 2) {
        // Find the character's position in the char_data array
        int ch_pos = find_ch_info(char_data, 8, m.ch);

        // Ensure the character is valid
        if (ch_pos == -1) {
            // Character not found, return early
            //mvwprintw(debug_win, 1, 1, "no character");
            //wrefresh(debug_win);
        }

        // Get the character's position
        int pos_x = char_data[ch_pos].position[0];
        int pos_y = char_data[ch_pos].position[1];

        // Check if the character can fire
        if (char_data[ch_pos].fire == 0) {
            // Fire for the first time
            char_data[ch_pos].start_time_fire = time(NULL);
            char_data[ch_pos].fire = 1;
            draw_zap_line(grid_win, pos_x, pos_y, m.ch,char_data,grid);
            update_score(score_win, char_data,8);
        } else if (seconds_passed(char_data[ch_pos].start_time_fire, 3)) {
            // Allow firing again after 3 seconds
            char_data[ch_pos].start_time_fire = time(NULL);
            draw_zap_line(grid_win, pos_x, pos_y, m.ch,char_data,grid);
            update_score(score_win, char_data, 8);
          }
        }else if (m.msg_type == 3) {
              // Message from disconect
                int ch_pos = find_ch_info(char_data, 8, m.ch);
                act_player[ch_pos]=0;
                wmove(grid_win, char_data[ch_pos].position[0], char_data[ch_pos].position[1]);
                waddch(grid_win, ' ');
                char_data[ch_pos].score=0;
                char_data[ch_pos].character=' ';
                werase(score_win);
                update_score(score_win, char_data, 8 + 1);
                n_chars--;
                wrefresh(grid_win);

      }else if (m.msg_type == 4) {
                // Message from child
                //move_aliens(grid_,m.direction);
                if(draw_aliens(grid_win,grid_os1)) //Game Over (no more aliens)
                {
                  char wins[20];
                  int max_points=0;

                  int ch_win;
                  m.msg_type=3;
                  for(int a=0;a<MAX_PLAYERS;a++) //search for higher score
                  {
                    if (char_data[a].score>max_points)
                        {
                          ch_win = a;
                          max_points=char_data[a].score;
                        }
                  }
                  sprintf(wins,"%c Wins!",icons[ch_win]);
                  for(int b=0;b<6;b++)
                  {
                      mvwaddch(grid_win, 11, b+8, wins[b]);
                  }
                  wrefresh(grid_win);
                  break;
                }
                //mvwprintw(debug_win, 2, 1, "Message from child received");
                //wrefresh(debug_win);
          }
      }

    return NULL;
}

// Main program
int main() {
    pthread_t input_thread, display_thread_id;
    thread_args_t thread_data;
    pthread_mutex_t quit_mutex = PTHREAD_MUTEX_INITIALIZER;

    // Initialize ZeroMQ context and socket
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(socket, "tcp://localhost:6025");
    remote_char_t m;
    m.msg_type = 0; // Initial message type for connection
    m.direction = UP; // Default direction
    int ch;

    // Send initial connection message (msg_type=0)
    zmq_send(socket, &m, sizeof(remote_char_t), 0);
    zmq_recv(socket, &m, sizeof(remote_char_t), 0); // Receive the assigned character from the server
    ch = m.ch; // Store the assigned character
    // Initialize thread data
    thread_data.socket = socket;
    thread_data.assigned_char = ch; // Example assigned character
    thread_data.quit_flag = 0;
    thread_data.quit_mutex = &quit_mutex;

    // Initialize ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    //timeout(100); // Non-blocking getch()
    pthread_create(&display_thread_id, NULL, display_thread, &thread_data);
    // Start the astronaut client thread
    pthread_create(&input_thread, NULL, astronaut_client_thread, &thread_data);

    // Start the display thread


    // Wait for both threads to finish
    pthread_join(input_thread, NULL);
    pthread_join(display_thread_id, NULL);

    // Clean up
    endwin();
    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}
