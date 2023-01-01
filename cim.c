#include <curses.h>
#include <stdlib.h>
#include <string.h>

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

char *buffer;
unsigned long buffersize = 10240; // by default initiate with that much buffer
unsigned long buffer_len_real = 0; // how many bytes are really in here?
unsigned long bufindex = 0;
char *filepath = NULL;

int screen_rows,screen_cols; // to store the size of screen

void init_buffer() {
	buffer = malloc(buffersize);
	if (buffer == NULL) {
		mvprintw(LINES - 1, 0, "Error: out of mem! attempted malloc() with %d bytes\n", buffersize);
	}
}

/* atm just double it*/
void increase_buffer() {
	
	buffer = realloc(buffer, buffersize * 2);
	if (buffer == NULL) { // out of mem
		mvprintw(LINES - 1, 0, "Error: out of mem! attempted realloc() with %d bytes\n", buffersize*2);
	} else {
		buffersize *= 2;
	}

}

/* add a char to the immediate index */
void buf_add_char(char c) {
	if (bufindex >= buffersize - 1) {
		increase_buffer();
	}
	buffer[bufindex] = c;
	bufindex ++;
	buffer_len_real ++;
}

/* remove a char from the immediate index by setting it to NULL */
void buf_remove_char() {
	buffer[bufindex] = 0;

	if (bufindex > 0) { // no underflowing
		bufindex --;
	}
	buffer_len_real --;
	
}

/* LINES and COLS are macros that tell how big the window is; 
to print to the last line / line, use line - 1 and col - 1
*/

int main(int argc, char **argv) {

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, 1);//enable things like arrow keys

    buffer = malloc(buffersize);

    cur_y = 0;
    cur_x = 0;

    cur_mode = MODE_NORM;
    int first_time = 1;


    // mvaddstr(0, 0, msg);
    // mvaddch(0, 0, 'c');
    mvprintw(0, 0, "welcome to cim; type i to start typing; press q to exit LINES:%d COLS:%d\n", LINES, COLS);

    while (1) {
    	int c = getch();

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
	    	if (c == 's') {
	    		if (filepath == NULL) {
	    			// popup window in the middle
	    			char msg[] = "enter filepath to save: ";
	    			
	    			getmaxyx(stdscr,screen_rows,screen_cols); //  get the number of rows and columns
	    			mvprintw(screen_rows/2,(screen_cols - strlen(msg))/2, "%s", msg); // center of screen
	    			filepath = malloc(1024);
	    			echo(); // enable echo so users can see what they're typing
	    			getstr(filepath);
	    			noecho(); // turn it back off
	    			// make prompt disappear
	    			for (int i = 0; i < strlen(msg) + strlen(filepath); ++i) {
	    				mvaddch(screen_rows/2,(screen_cols - strlen(msg))/2+i, ' ');
	    			}
	    			

	    		}

	    		// now flush the buffer to file
	    		FILE *fp = fopen(filepath, "w+");
    			fwrite(buffer, buffer_len_real, 1, fp);
	    		fclose(fp);
	    		char msg[strlen("saved to: ") + strlen(filepath) +  1];
	    		sprintf(msg, "saved to: %s", filepath);
	    		getmaxyx(stdscr,screen_rows,screen_cols);
	    		mvprintw(screen_rows-1, screen_cols - strlen(msg) - 1, "%s", msg);
	    		// move cursor back 
		    	move(cur_y, cur_x);

	    	}
	    	// arrow key and vim binding movements
	    	if (c == 'h' || c == KEY_LEFT) {
	    		cur_x --;
	    		move(cur_y, cur_x);
	    	}
	    	if (c == 'j' || c == KEY_DOWN) {
	    		cur_y ++;
	    		move(cur_y, cur_x);
	    	}
	    	if (c == 'k' || c == KEY_UP) {
	    		cur_y --;
	    		move(cur_y, cur_x);
	    	}
	    	if (c == 'l' || c == KEY_RIGHT) {
	    		cur_x ++;
	    		move(cur_y, cur_x);
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
    		if (c == 0x8) { // backspace, could also use KEY_BACKSPACE
    		#endif
    		
    			// TODO implement line switching
    			mvaddch(cur_y, cur_x - 1, ' ');
    			buf_remove_char();
    			cur_x--;
    			move(cur_y, cur_x);
    			refresh();
    			continue;

    		}

    		// arrow key and vim binding movements
	    	if (c == KEY_LEFT) {
	    		cur_x --;
	    		move(cur_y, cur_x);
	    		continue;
	    	}
	    	if (c == KEY_DOWN) {
	    		cur_y ++;
	    		move(cur_y, cur_x);
	    		continue;
	    	}
	    	if (c == KEY_UP) {
	    		cur_y --;
	    		move(cur_y, cur_x);
	    		continue;
	    	}
	    	if (c == KEY_RIGHT) {
	    		cur_x ++;
	    		move(cur_y, cur_x);
	    		continue;
	    	}

    		mvaddch(cur_y, cur_x ,c);
    		buf_add_char(c);
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