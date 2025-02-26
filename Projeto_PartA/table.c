#include <ncurses.h>
#include "table.h"
#include "astronaut.h"
#include <unistd.h>



#define GRID_SIZE 20 // Size of the grid
#define GRID_SIZE_ASTRONAUTS 16 // Size of the grid for the astronauts
#define ASTRONAUTS_WIDTH 2 // Width of the grid for the astronauts
#define SCORE_WIDTH 20 // Width of the score panel


/*
 *Function: draw_grid
 * -------------------
 * Draws the borders of a grid window.
 *
 * Parameters:
 *   grid_win - A pointer to the WINDOW object where the grid borders
 *              will be drawn.
 *
 * Description:
 *   This function draws a box around the given grid window using
 *   ncurses' box() function. It then refreshes the window to
 *   display the borders immediately.
 */
void draw_grid(WINDOW* grid_win) {
    // Draw the grid borders
    box(grid_win, 0, 0);
    wrefresh(grid_win);
}

/**
 * Function: draw_numbers
 * ----------------------
 * Displays column numbers at the top and row numbers on the side of a grid.
 *
 * Parameters:
 *   top_numbers_win  - A pointer to the WINDOW object for the top numbers.
 *   side_numbers_win - A pointer to the WINDOW object for the side numbers.
 *
 * Description:
 *   This function prints a sequence of numbers (1-9, then wraps to 0)
 *   across the top and left side of a grid. The numbers are retrieved
 *   from a predefined array and are printed in the corresponding positions:
 *     - Top numbers are aligned horizontally across the first row of the
 *       top_numbers_win, with a +1 offset for alignment.
 *     - Side numbers are aligned vertically along the first column of the
 *       side_numbers_win, with a +1 offset for alignment.
 *
 *   After adding the numbers, both windows are refreshed to display the
 *   updates immediately.
 */
void draw_numbers(WINDOW* top_numbers_win, WINDOW* side_numbers_win) {
    // Add column numbers at the top of the grid
    int arr[21]={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0};
    for (int col = 0; col < GRID_SIZE+1; col++) {
        mvwprintw(top_numbers_win, 0, col + 1, "%d", arr[col]); // Offset by +1 for alignment
    }

    // Add row numbers to the left of the grid
    for (int row = 0; row < GRID_SIZE+1; row++) {
        mvwprintw(side_numbers_win, row + 1, 0, "%d",arr[row]); // Offset by +1 for alignment
    }

    wrefresh(top_numbers_win);
    wrefresh(side_numbers_win);
}

/**
 * Function: update_score
 * ----------------------
 * Updates and displays the scores of players in a score window.
 *
 * Parameters:
 *   score_win - A pointer to the WINDOW object where the scores will be displayed.
 *   char_data - An array of player structures containing character identifiers and their scores.
 *   n         - The number of players whose scores need to be displayed.
 *
 * Description:
 *   This function clears the score window, draws a border around it, and updates
 *   it with the current scores of players. The function iterates through the array
 *   'char_data', and for each valid character (A-I, based on ASCII values 65 to 72),
 *   it displays the player's character and score. The scores are shown starting
 *   from the second row of the score window to maintain alignment.
 *
 *   The window is then refreshed to display the updates immediately.
 */
void update_score(WINDOW* score_win,player char_data[],int n) {
    // Clear and refresh the score window
    werase(score_win);
    box(score_win, 0, 0);
    mvwprintw(score_win, 1, 1, "SCORE");
    char buffer='A';
    for(int i=0;i<n;i++)
    {
      buffer=char_data[i].character;
      if((64<buffer)&&(buffer<73)){
        mvwprintw(score_win, i+2, 1, "%c - %d",char_data[i].character, char_data[i].score);
      }
    }

    wrefresh(score_win);
}

/**
 * Function: draw_zap_line
 * -----------------------
 * Draws a zap line (vertical or horizontal) on the grid based on the player's character type.
 *
 * Parameters:
 *   grid_win    - A pointer to the WINDOW object representing the grid.
 *   pos_x       - The X-coordinate (row) of the player's position.
 *   pos_y       - The Y-coordinate (column) of the player's position.
 *   player_char - The character identifier for the player initiating the zap.
 *   char_data   - An array of player structures containing character data and status.
 *   grid        - A 2D array representing the grid's state.
 *
 * Description:
 *   This function draws a zap line originating from the player's current position:
 *     - For players of type 'B', 'C', 'H', or 'F', the zap line is vertical ('|')
 *       and moves from the player's position upward to the top of the grid.
 *     - For other player types, the zap line is horizontal ('-') and spans from
 *       left to right across the grid at the player's row.
 *
 *   While drawing the zap line, the function:
 *     - Marks opponents encountered along the line as "stunned" and logs the
 *       start time of the stun.
 *     - Increments the player's score if the zap hits a target ('*') and
 *       clears the target from the grid.
 *
 *   After displaying the zap line briefly (0.5 seconds), it erases the line
 *   from the grid to restore its original state. The grid window is refreshed
 *   after each operation to reflect changes visually.
 */
void draw_zap_line(WINDOW* grid_win, int pos_x, int pos_y, char player_char, player char_data[],int grid[20][20]) {
    if (player_char == 'B' || player_char == 'C' || player_char == 'H' || player_char == 'F') {
        // Draw vertical zap line ('|') from the player's position to the top
        for (int x = 20; x >= 1; x--) { // Minimum row boundary is 3
            wmove(grid_win, x, pos_y);
            char ch = mvwinch(grid_win, x, pos_y) & A_CHARTEXT; // Get character at position
            if (ch == ' ') {
                waddch(grid_win, '|');
            }
            if((ch=='A' ||ch == 'B' || ch == 'C' || ch == 'D'|| ch == 'E'|| ch == 'F'|| ch == 'G'|| ch == 'H') && (ch!=player_char)){
                int loc = find_ch_info(char_data, 8, ch);
                char_data[loc].stun = 1;
                char_data[loc].start_time_stun= time(NULL);
            }
            if (ch == '*') {
                int loc = find_ch_info(char_data, 8, player_char);
                char_data[loc].score++;
                grid[x-1][pos_y-1]=0;
                waddch(grid_win, ' ');
            }
        }
        wrefresh(grid_win);
        // Wait for 0.5 seconds
        time_t start_time = time(NULL);
        time_t current_time = time(NULL);
        while(difftime(current_time, start_time) >= 0.5)
        {
          current_time = time(NULL);
        }

         // Erase the vertical zap line
        for (int x = 20; x >= 1; x--) {
          wmove(grid_win, x, pos_y);
          char ch = mvwinch(grid_win, x, pos_y) & A_CHARTEXT;
          if (ch == '|') {
              waddch(grid_win, ' ');
          }
        }
    } else{
        // Draw horizontal zap line ('-') from left to right across the grid
        for (int y = 1; y <21; y++) {
            wmove(grid_win, pos_x, y);
            char ch = mvwinch(grid_win, pos_x, y) & A_CHARTEXT; // Get character at position
            if (ch == ' ') {
                waddch(grid_win, '-');
            }
            if((ch=='A' ||ch == 'B' || ch == 'C' || ch == 'D'|| ch == 'E'|| ch == 'F'|| ch == 'G'|| ch == 'H') && (ch!=player_char)){
                int loc = find_ch_info(char_data, 8, ch);
                char_data[loc].stun = 1;
                char_data[loc].start_time_stun= time(NULL);

            }
            if (ch == '*') {
                int loc = find_ch_info(char_data, 8, player_char);
                char_data[loc].score++;
                grid[pos_x-1][y-1]=0;
                waddch(grid_win, ' ');

            }
        }
        wrefresh(grid_win);
        // Wait for 0.5 seconds
        time_t start_time = time(NULL);
        time_t current_time = time(NULL);
        while(difftime(current_time, start_time) >= 0.5)
        {
          current_time = time(NULL);
        }

        // Erase the horizontal zap line
        for (int y = 1; y <21; y++) {
          wmove(grid_win, pos_x, y);
          char ch = mvwinch(grid_win, pos_x, y) & A_CHARTEXT;
          if (ch == '-') {
              waddch(grid_win, ' ');
          }
        }
        wrefresh(grid_win);
    }

    // Refresh the grid window to display the zap line
    wrefresh(grid_win);
}

/**
 * Function: prepare_message_display
 * ---------------------------------
 * Prepares a message display structure for communication or rendering purposes.
 *
 * Parameters:
 *   actpl   - An array representing the active players in the game.
 *   grid    - A 2D array representing the current state of the grid.
 *   ch      - An array of player structures containing character information.
 *   m       - A remote_char_t structure containing additional message details.
 *
 * Returns:
 *   A 'message_display' structure populated with the provided game state data.
 *
 * Description:
 *   This function creates and populates 'message_display' structure, which
 *   is used to encapsulate the current state of the game. It copies the grid
 *   data, active player states, and player character data into the structure.
 *   Additionally, it includes a copy of the 'remote_char_t' message for further
 *   use. The function ensures that data integrity is maintained by copying only
 *   the relevant elements.
 */
message_display prepare_message_display(int actpl[MAX_PLAYERS],int grid[GRID_SIZE][GRID_SIZE],player ch[MAX_PLAYERS],remote_char_t m)
{
  message_display m_disp;
  for (int k=0;k<20;k++)
  {
    for(int h=0;h<20;h++)
    {
      m_disp.grid[k][h]=grid[k][h];
    }
    if (k<8)
    {
      m_disp.ch_data[k]=ch[k];
      m_disp.act_pl[k]=actpl[k];
    }
  }
  m_disp.copy_message=m;
  return m_disp;
}

/**
 * Function: draw_players
 * ----------------------
 * Draws active players on the game grid.
 *
 * Parameters:
 *   ch       - An array of player structures containing player data such as position and character.
 *   grid_win - A pointer to the WINDOW object representing the grid.
 *   act      - An array indicating whether each player is active (1 for active, 0 for inactive).
 *
 * Description:
 *   This function iterates through all players and checks if they are active based on the 'act' array.
 *   For each active player, it moves the cursor to the player's position on the grid and draws the
 *   player's character using 'waddch', with the character styled in bold. After all players are drawn,
 *   the grid window is refreshed to display the updates.
 */
void draw_players(player ch[MAX_PLAYERS],WINDOW* grid_win,int act[MAX_PLAYERS])
{

  for (int i = 0; i < MAX_PLAYERS; i++) {
    if(act[i]==1)
    {
      wmove(grid_win, ch[i].position[0], ch[i].position[1]);
      waddch(grid_win, ch[i].character | A_BOLD);
    }
  }
  wrefresh(grid_win);
}
