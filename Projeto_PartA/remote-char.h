#ifndef REMOTE_CHAR_H
#define REMOTE_CHAR_H
#include <time.h>


#define MAX_PLAYERS 8
#define REQ_IP "tcp://*:6025"
#define PUB_IP "tcp://*:6028"
#define FORK_IP "tcp://localhost:6025"
#define GRID_SIZE 20 // Size of the grid
#define GRID_SIZE_ASTRONAUTS 16 // Size of the grid for the astronauts
#define ASTRONAUTS_WIDTH 2 // Width of the grid for the astronauts
#define SCORE_WIDTH 20 // Width of the score panel


typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

//message between req and rep
typedef struct remote_char_t
{
    int msg_type; //0 join   1 - move   2- zapp  3- quit  4- fork
    char ch;
    direction_t direction ;
    char score_data[100];
}remote_char_t;


//player array to store data
typedef struct player{
    int score;
    int position[2];
    int fire; //flag for zapping
    int stun; //flag for stun
    time_t start_time_fire;
    time_t start_time_stun;
    char character;
}player;

//message for pub sub
typedef struct message_display
{
    int msg_type;
    int grid[20][20];
    player ch_data[8];
    int act_pl[9];
    remote_char_t copy_message;

}message_display;

#endif
