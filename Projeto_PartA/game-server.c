#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <zmq.h>
#include "aliens.h"
#include "astronaut.h"
#include "table.h"
#include "remote-char.h"

int main() {

  int int_poss[MAX_PLAYERS][2]={{3,1},{1,3},{2,3},{3,2},{3,20},{20,3},{3,19},{19,3}}; //start position for players
  char icons[MAX_PLAYERS+1]="ABCDEFGH"; //available players
  player char_data[MAX_PLAYERS]; //array that stores player data
  int act_player[MAX_PLAYERS]; //array that stores active players
  init_pl(char_data,act_player); //initialize arrays

  int n_chars = 0;
  message_display m_disp; //message for remote screen
  //create zmq sockets
  void* context = zmq_ctx_new();
  void* req_socket = zmq_socket(context, ZMQ_REP);
  zmq_bind(req_socket,REQ_IP);

  void* pub_socket = zmq_socket(context, ZMQ_PUB);
  zmq_bind(pub_socket, PUB_IP);


  // Initialize ncurses
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
  int grid[GRID_SIZE][GRID_SIZE] = {0}; // 0 means empty, 1 means occupied by an alien

  // Populate 1/3 of the grid with aliens
  populate_aliens(grid_win, grid);

  update_score(score_win, char_data, 8);

  remote_char_t m; //mesage for players and fork

  // Create a debug window below the main grid
  //WINDOW* debug_win = newwin(10, 50, GRID_SIZE + 3, 2); // Debug window
  //box(debug_win, 0, 0);
  //mvwprintw(debug_win, 0, 1, "DEBUG");
  //wrefresh(debug_win);

  // Fork to create a child process
  pid_t pid = fork();
  if (pid == 0) {
    fork_aliens(); //fork function
    exit(0); // Ensure child process exits cleanly
  }else if (pid > 0) {
    while (1) {
      zmq_recv(req_socket, &m, sizeof(remote_char_t), 0); //message from players or fork
      m_disp=prepare_message_display(act_player,grid,char_data,m); //initialize message for display
      zmq_send(pub_socket, &m_disp, sizeof(message_display), 0); //send display message
      //werase(debug_win); // Clear debug window
      //box(debug_win, 0, 0); // Redraw the border
      //mvwprintw(debug_win, 0, 1, "DEBUG");
      //mvwprintw(debug_win, 1, 1, "Received message: msg_type=%d, char=%c", m.msg_type, m.ch);
      //wrefresh(debug_win);

      if (m.msg_type == 0 && n_chars<=7) { //connection message
        for(int c=0;c<MAX_PLAYERS;c++) //find desactivated slot to give to player
        {
          if (act_player[c]==0) //if player is inactive connect
          {
            char_data[c].position[0] = int_poss[c][0];
            char_data[c].position[1] = int_poss[c][1];
            char_data[c].character = icons[c];
            m.ch = icons[c];
            char_data[c].score = 0;

            grid[char_data[c].position[0]][char_data[c].position[1]] = 1; // Mark grid as occupied
            update_score(score_win, char_data, MAX_PLAYERS);

            wmove(grid_win, char_data[c].position[0], char_data[c].position[1]);
            waddch(grid_win, char_data[c].character | A_BOLD);
            wrefresh(grid_win);

            n_chars++; // Increment player count
            act_player[c]=1; //activate slot
            break;
          }
        }
        zmq_send(req_socket, &m, sizeof(remote_char_t), 0);
      }else if (m.msg_type == 1) { //movement message
        int ch_pos = find_ch_info(char_data,MAX_PLAYERS, m.ch); //find character location in array
        if (ch_pos == -1) { //player that sent message doesn't exit
          m.msg_type = -1;
          zmq_send(req_socket, &m, sizeof(remote_char_t), 0);
        }else if(char_data[ch_pos].stun == 0){ //if player is not stunned
            sprintf(m.score_data,"O teu score e %d",char_data[ch_pos].score);

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
            zmq_send(req_socket, &m, sizeof(remote_char_t), 0);
        }else if (seconds_passed(char_data[ch_pos].start_time_stun, 10)) { //if 10 seconds have passed
          sprintf(m.score_data,"O teu score e %d",char_data[ch_pos].score);
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
          zmq_send(req_socket, &m, sizeof(remote_char_t), 0);
        }else{
          sprintf(m.score_data,"O teu score e %d",char_data[ch_pos].score);
          zmq_send(req_socket, &m, sizeof(remote_char_t), 0); //if 10 seconds haven't passed
        }

      }else if (m.msg_type == 2) { //Zapp Message
        // Find the character's position in the char_data array
        int ch_pos = find_ch_info(char_data, MAX_PLAYERS, m.ch);

        if (ch_pos == -1) { //if player doens't exist
          m.msg_type = -1;
          zmq_send(req_socket, &m, sizeof(remote_char_t), 0);
        }else if (char_data[ch_pos].fire == 0) { //if character can fire
          // Get the character's position
          int pos_x = char_data[ch_pos].position[0];
          int pos_y = char_data[ch_pos].position[1];
          // Fire for the first time
          char_data[ch_pos].start_time_fire = time(NULL);
          char_data[ch_pos].fire = 1;
          draw_zap_line(grid_win, pos_x, pos_y, m.ch,char_data,grid);
          update_score(score_win, char_data,8);
          zmq_send(req_socket, &m, sizeof(remote_char_t), 0); // Send after firing
        } else if (seconds_passed(char_data[ch_pos].start_time_fire, 3)) { //if 3 seconds have passed
          // Get the character's position
          int pos_x = char_data[ch_pos].position[0];
          int pos_y = char_data[ch_pos].position[1];
          // Allow firing again after 3 seconds
          char_data[ch_pos].start_time_fire = time(NULL);
          draw_zap_line(grid_win, pos_x, pos_y, m.ch,char_data,grid);
          update_score(score_win, char_data, 8);
          zmq_send(req_socket, &m, sizeof(remote_char_t), 0); // Send after firing
        } else{
          zmq_send(req_socket, &m, sizeof(remote_char_t), 0); //if 3 seconds haven't passed
        }

      }else if (m.msg_type == 3) { //disconect message
        // Message from disconect
        int ch_pos = find_ch_info(char_data,MAX_PLAYERS, m.ch);
        if (ch_pos == -1) { //if player doesn't exist
          m.msg_type = -1;
          zmq_send(req_socket, &m, sizeof(remote_char_t), 0);
        }else{
          act_player[ch_pos]=0; //deactivate player
          wmove(grid_win, char_data[ch_pos].position[0], char_data[ch_pos].position[1]);
          waddch(grid_win, ' ');
          char_data[ch_pos].score=0;
          char_data[ch_pos].character=' ';
          update_score(score_win, char_data, MAX_PLAYERS+1);
          n_chars--;
          wrefresh(grid_win);
          zmq_send(req_socket, &m, sizeof(remote_char_t), 0);
        }

      }else if (m.msg_type == 4) { //message form fork child to move aliens
        move_aliens(grid,m.direction);
        if(draw_aliens(grid_win,grid)) //Game Over (no more aliens)
        {
          char wins[20];
          int max_points=0;

          int ch_win;
          m.msg_type=3; //gameover message equals to quit
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
              mvwaddch(grid_win, 11, b+8, wins[b]); //shows winner
          }
          wrefresh(grid_win);
          zmq_send(req_socket, &m, sizeof(remote_char_t), 0);
          break;
        }
        zmq_send(req_socket, &m, sizeof(remote_char_t), 0);
      }


    }
    zmq_close(req_socket);
    zmq_close(pub_socket);
    zmq_ctx_destroy(context);
    sleep(5);
    endwin();
  }
  else {
    perror("Fork failed");
    exit(EXIT_FAILURE);
  }

  return 0;
}
