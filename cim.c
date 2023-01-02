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

int cur_mode; // current mode
unsigned int cur_y, cur_x; // cim current x and y of text (e.g. where the user was typing)
int cur_line = 0;


char *buffer;
unsigned long buffersize = 10240; // by default initiate with that much buffer
unsigned long buffer_len_real = 0; // how many bytes are really in here?
unsigned long bufindex = 0;
char *filepath = NULL;

/* the idea behind text_line_lens is an instant access array to tell how long 
each line is, and use that to calculate buffer indexes when the user moves 
around with the cursor using y,x coordinates 

e.g. #line text
#0 this\n             --> line length 5 ("this" + "\n")
#1 is\n               --> line length 3 
#2 an example        --> line length 11
      ^
x->0123456789
      
      the 'e' marked with ^ is on y = 2 (line = 2), and x = 3
      and its buffer location is 5 + 3 + 3 = 11

so the buffer index of a given y,x coord is the sum of line lengths before the
current line number PLUS the x coordinate
*/
unsigned long *text_line_lens; // a index-mapped dynamic array of how long each line is
unsigned long text_lines_len_items; // number of lines (items) allocated in text_line_lens

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

void init_line_lengths() {
	text_line_lens = malloc(sizeof(unsigned long) * 1);
	text_lines_len_items = 1;
}

void check_expand_line_lengths(int line) {
	if (line >= text_lines_len_items) {
		text_line_lens = realloc(text_line_lens, sizeof(unsigned long) * text_lines_len_items*2);
		if (text_line_lens == NULL) {
			mvprintw(LINES - 1, 0, "Error: out of mem: attempted realloc() with %d bytes\n", text_lines_len_items*2);
		} else {
			text_lines_len_items *= 2;
		}
	}
}
/* update the length of a line in the line length array */
void update_line_length(int line, unsigned long length) {
	check_expand_line_lengths(line);
	text_line_lens[line] = length;
}

void inc_line_length(int line) {
	check_expand_line_lengths(line);
	text_line_lens[line]++;
}

// get buffer index by line number
unsigned long lookup_buf_index(int line, int x) {
	unsigned long bufindex = 0;
	for (int i = 0; i < line; ++i)
	{
		/* add lengths of all lines before this one */
		bufindex += text_line_lens[i];
	}
	// now for current line
	bufindex += x;
	return bufindex;
}

/* err msg on the bottom right corner */
void errmsg(char* msg) { 
	getmaxyx(stdscr, LINES, COLS); // update LINES and COLS to adapt to changing screen sizes;
	mvaddstr(LINES - 1, COLS - strlen(msg), msg);
	refresh();
}

/** report debug info / status in the corner 
 * input_c is the result of getch()
 **/
void status_report_corner(int input_c) {
	getmaxyx(stdscr, LINES, COLS); // update LINES and COLS to adapt to changing screen sizes
	char msg[COLS];
	snprintf(msg, COLS, "y/x %d/%d line:%d c:%02x buf_i:%lu", cur_y,cur_x, cur_line, input_c, lookup_buf_index(cur_line, cur_x));
	mvaddstr(LINES - 1, COLS - strlen(msg), msg);
	refresh();
}



void load_file() {
	FILE *fp = fopen(filepath, "r");
	int done = 0;
	int rlen = 0;
	if (fp != NULL) {
		while (!done) {
			rlen += fread(buffer+rlen, 1, buffersize, fp);
			if (rlen == 0 || feof(fp)) {
				done = 1;
			}
			// if bigger
			if (rlen >= buffersize) {
				increase_buffer();
			}
		}
	} else {
		errmsg("cannot open file; does it exist?");
	}

	// display buffer on screen
	clear();
	mvprintw(0,0,"%s", buffer);
	refresh();

	
}

/* LINES and COLS are macros that tell how big the window is; 
to print to the last line / line, use line - 1 and col - 1
*/

int main(int argc, char **argv) {

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, 1);//enable things like arrow keys

    init_line_lengths();

    buffer = malloc(buffersize);

    
    cur_y = 0;
    cur_x = 0;

    cur_mode = MODE_NORM;
    int first_time;

    // load file if there is one
    if (argc > 1) {
    	filepath = argv[1];
    	first_time = 0;
    	load_file();
    } else {
    	first_time = 1;
		mvprintw(0, 0, "welcome to cim; type i to start typing; press q to exit LINES:%d COLS:%d\n", LINES, COLS);
    }


    while (1) {
    	int c = getch();
    	status_report_corner(c);
    	
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
	    	if (c == 's' || c == 'w') { // maybe get rid of s, idk
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
    		
    			// TODO implement deleting pass end of line, need to move cursor up
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
    			inc_line_length(cur_line); // \n counts as 1 char in buffer
    			cur_y++;
    			cur_line++;
    			cur_x = 0; // new line, reset x
    		} else {
    			cur_x++;
    			inc_line_length(cur_line);
    		} 
    		refresh();
    	}

    	

    }
    

}