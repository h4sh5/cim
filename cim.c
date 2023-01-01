#include <curses.h>
#include <stdlib.h>

// start mode at 1 to not be confused with null bytes
#define MODE_NORM 1 
#define MODE_TYPING 2

// quit func; clean ups and stuff
void quit(int exitcode) {
	clear();
	endwin();

	exit(exitcode);
}

static int cur_mode; // current mode
static int cur_y, cur_x; // cim current x and y of text (e.g. where the user was typing)


/* LINES and COLS are macros that tell how big the window is; 
to print to the last line / line, use line - 1 and col - 1
*/

int main(int argc, char **argv) {

    initscr();
    cbreak();
    noecho();

    cur_y = 0;
    cur_x = 0;

    cur_mode = MODE_NORM;
    int first_time = 1;


    // mvaddstr(0, 0, msg);
    // mvaddch(0, 0, 'c');
    mvprintw(0, 0, "welcome to cim; type i to start typing; press q to exit LINES:%d COLS:%d\n", LINES, COLS);

    while (1) {
    	char c = getch();

    	mvprintw(LINES - 1, COLS - 5, "c:%02x", c);
    	refresh();
    	
    	if (cur_mode == MODE_NORM) {
    		if (c == 'q') {
    			quit(0);
	    	}
	    	if (c == 'i') {
	    		if (first_time) { // only clear screen on first use, not mode switch
	    			clear();
	    			first_time = 0;
	    		}
	    		mvaddstr(LINES - 1, 0, "-- INSERT --"); // 12 chars
	    		move(cur_y, cur_x);
	    		cur_mode = MODE_TYPING;
	   

	    	}

	    	refresh();

    	} else if (cur_mode == MODE_TYPING) {
    		// TODO implement vim movement and arrow key bindings

    		if (c == 0x1b) { //ESC / escape
    			cur_mode = MODE_NORM; // switch back to normal
    			// clear the INSERT status
    			mvaddstr(LINES - 1, 0, "             "); 
    			move(cur_y, cur_x);
    			continue;
    		}

    		#ifdef 	__APPLE__
    		if (c == 0x7f || c == 0x8) { // backspace on mac keyboard
    		#else
    		if (c == 0x8) { // backspace 
    		#endif
    		
    			// TODO implement line switching
    			mvaddch(cur_y, cur_x - 1, ' ');
    			cur_x--;
    			move(cur_y, cur_x);
    			refresh();
    			continue;

    		}

    		mvaddch(cur_y, cur_x ,c);
    		if (c == '\n') {
    			cur_y++;
    			cur_x = 0; // new line, reset x
    		} else {
    			cur_x++;
    		} 
    		refresh();
    	}

    	

    }
    

}