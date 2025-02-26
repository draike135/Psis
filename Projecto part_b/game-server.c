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
#include "score.pb-c.h"

typedef struct {
    void *context;
    void *req_socket;
    void *pub_socket;
    WINDOW *grid_win;
    WINDOW *score_win;
    player *ch_data;
    int *act_pl;
} communication_struct;

void *communication_thread(void *arg) {
    communication_struct *comm_data = (communication_struct *)arg;
    void *context= comm_data->context;
    void *req_socket = comm_data->req_socket;
    void *pub_socket = comm_data->pub_socket;
    WINDOW* grid_win=comm_data->grid_win;
    WINDOW* score_win=comm_data->score_win;
    player *char_data=comm_data->ch_data; //array that stores player data
    int *act_player=comm_data->act_pl;


    //void *context2 = zmq_ctx_new();
    void *socket_2 = zmq_socket(context, ZMQ_PUB);
    // Connect to the Python server
    zmq_connect(socket_2, "tcp://localhost:5555");


    int int_poss[MAX_PLAYERS][2]={{3,1},{1,3},{2,3},{3,2},{3,20},{20,3},{3,19},{19,3}}; //start position for players
    char icons[MAX_PLAYERS+1]="ABCDEFGH"; //available players
    init_pl(char_data,act_player); //initialize arrays
    int n_chars = 0;

    remote_char_t m;
    message_display m_disp;
    update_score(score_win, char_data, 8);






    while (1) {
        // Receive a message from the player (or fork)
        zmq_recv(req_socket, &m, sizeof(remote_char_t), 0); //message from players or fork
        pthread_mutex_lock(&data_lock);
        if(m.msg_type==9)
        {
          //printf("yepsx2\n");
          break;
        }
        m_disp=prepare_message_display(act_player,grid,char_data,m); //initialize message for display
        zmq_send(pub_socket, &m_disp, sizeof(message_display), 0); //send display message
        // Process the message
        if (m.msg_type == 0 && n_chars<=7){ //connection message
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
        } else if (m.msg_type == 1) {//movement message
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

        } else if (m.msg_type == 2) { //Zapp Message
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
          int scores_send[9];
          for (size_t i = 0; i < 8; i++) {
            scores_send[i]=char_data[i].score;
          }
          ScoreUpdate score_update = SCORE_UPDATE__INIT;
          score_update.n_astronaut_scores = 8;
          score_update.astronaut_scores = act_player;
          score_update.n_space_mission_scores = 8;
          score_update.space_mission_scores = scores_send;

          // Serialize the message using Protocol Buffers
          size_t message_size = score_update__get_packed_size(&score_update);
          void *buffer = malloc(message_size);
          score_update__pack(&score_update, buffer);
          zmq_send(socket_2, buffer, message_size, 0);
          free(buffer);

        } else if (m.msg_type == 3) { //disconect message
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
            werase(score_win);
            update_score(score_win, char_data, MAX_PLAYERS+1);
            n_chars--;
            wrefresh(grid_win);
            zmq_send(req_socket, &m, sizeof(remote_char_t), 0);
          }

    }
    //printf("yepsx3\n");
     pthread_mutex_unlock(&data_lock);
  }
  //printf("yepsx332\n");

  zmq_close(socket_2);

  return NULL;
}

int main() {

  pthread_t alien_thread, qthr_thread,msg_thread;
  pthread_mutex_init(&grid_lock, NULL);
  pthread_mutex_init(&data_lock, NULL);


  void* context = zmq_ctx_new();
  void* req_socket = zmq_socket(context, ZMQ_REP);
  zmq_bind(req_socket,REQ_IP);

  void* pub_socket = zmq_socket(context, ZMQ_PUB);
  zmq_bind(pub_socket, PUB_IP);
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
  //int grid[GRID_SIZE][GRID_SIZE] = {0}; // 0 means empty, 1 means occupied by an alien
  for (int i = 0; i < GRID_SIZE; i++) {
    for (int j = 0; j < GRID_SIZE; j++) {
        grid[i][j] = 0;
    }
  }

  // Populate 1/3 of the grid with aliens
  populate_aliens(grid_win, grid);
  player char_data[MAX_PLAYERS]; //array that stores player data
  int act_player[MAX_PLAYERS]; //array that stores active players

  thread_struct th_message;
  th_message.pub=pub_socket;
  th_message.grid_win=grid_win;
  th_message.ch_data=char_data;
  th_message.act_pl=act_player;

  termina termina_1;
  termina_1.pub=pub_socket;
  termina_1.context=context;

  communication_struct comm_data = {context, req_socket, pub_socket,grid_win,score_win,char_data,act_player};

  pthread_create(&msg_thread, NULL, communication_thread, &comm_data);
  pthread_create(&alien_thread, NULL, move_aliens_thread,&th_message);
  pthread_create(&qthr_thread, NULL, keyboard_listener,&termina_1);

  // Wait for the thread to finish (it never will in this case)
  pthread_join(qthr_thread, NULL);
  pthread_join(msg_thread, NULL);
  term_flag=1;
  pthread_join(alien_thread, NULL);
  pthread_mutex_destroy(&data_lock);
  pthread_mutex_destroy(&grid_lock);
  zmq_close(req_socket);
  zmq_close(pub_socket);
  zmq_ctx_shutdown(context);
  //sleep(2);
  endwin();

  return 0;
}
