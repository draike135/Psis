#include <zmq.h>
#include <ncurses.h>
#include "remote-char.h"
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
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

    // Initialize ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    int key;
    do {
        key = getch();
        if (key == KEY_LEFT) {
            m.direction = LEFT;
            m.msg_type = 1; // Movement message
        } else if (key == KEY_RIGHT) {
            m.direction = RIGHT;
            m.msg_type = 1; // Movement message
        } else if (key == KEY_DOWN) {
            m.direction = DOWN;
            m.msg_type = 1; // Movement message
        } else if (key == KEY_UP) {
            m.direction = UP;
            m.msg_type = 1; // Movement message
        } else if (key == ' ') {
            m.msg_type = 2; // Zap action
        } else if (key == 'q' || key == 'Q') {
            m.msg_type = 3; // Quit message
        } else {
            continue; // Ignore other keys
        }

        // Set character for the message
        m.ch = ch;

        // Send the message to the server
        zmq_send(socket, &m, sizeof(remote_char_t), 0);

        // Wait for server response
        zmq_recv(socket, &m, sizeof(remote_char_t), 0);
        mvprintw(0,0,"%s",m.score_data);
        // If the user presses 'q' or 'Q', exit the loop
        if (m.msg_type == 3) {
            break;
        }

    } while (key != 27); // Escape key to exit the loop

    // Clean up ncurses and ZeroMQ
    endwin();
    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}
