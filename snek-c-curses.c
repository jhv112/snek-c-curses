/******************
 * CLI Snake Game *
 ******************/

/* Requires: PDCurses
	This is a simple game of Snake.
	The controls are WASD or arrow keys to move.
	The objectives are: to eat food ('o') to grow; and to avoid the edges and
	 avoid biting yourself.

	The game's size can be passed as command-line arguments to the program
	 (minimum and maximum values are currently set to 2 and 80, respectively).
	When no arguments are given, the default behaviour is that the game's size
	 is the CLI's current window size.
*/ 

/* Known bugs:
        Small memory leak after first death.
        Small window size leads to larger-than-expected memory allocation.
        Holding down any key (bar the escape key) makes the snake go faster
         (which not necessarily a bug).
        Clicking on the CLI changes snake behaviour to only moving when
         a key is pressed: this is apparently a bug with minGW mouse support.
    (see: https://lists.gnu.org/archive/html/bug-ncurses/2013-12/msg00001.html )
*/

#include <stdlib.h>
#include <string.h>
#include <curses.h>

#define MAXSCREENWIDTH	80
#define MAXSCREENHEIGHT	80
#define DEFAULTSCREENWIDTH	20
#define DEFAULTSCREENHEIGHT	20
#define REFRESHRATE 50

#define ESC		0x1B

#define LIMIT(a, b, c)	((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))

// Snake head structure, with x-y co-ords, direction of travel and length of
//  snake.
struct Snek{
	int x;
	int y;
	int length;
	char dir;
};

void setxy(int argc, char **argv,	int *boardWidth, int *boardHeight,
									int screenWidth, int screenHeight);

void title(WINDOW *board, int boardWidth, int boardHeight);
void initUnderBoard(int ***board, int boardWidth, int boardHeight);
void initBoard(WINDOW *board, int boardWidth, int boardHeight);
void placeFood(WINDOW *board, int boardWidth, int boardHeight);
void ggnore(WINDOW *board, int **underBoard,
									int boardWidth, int boardHeight,
									chtype input);

void initSnek(WINDOW *board, int **underBoard, struct Snek *snek,
									int boardWidth, int boardHeight);
int moveSnek(WINDOW *board, int **underBoard, struct Snek *snek,
									int boardWidth, int boardHeight);
void changeSnekDirection(struct Snek *snek, chtype input);

int main(int argc, char **argv){
	// Start curses, hiding cursor
	initscr();
	cbreak();
	noecho();
	curs_set(0);

	// Declaring variables for window size, keyboard, internal representation
	//  and the snake's head.
	int boardWidth, boardHeight, screenWidth, screenHeight;
	chtype kb = '0';
	int **underBoard;
	struct Snek snekHead = {0, 0, 0, '\0'};
	struct Snek *snek = &snekHead;

	// Setting game's size
	getmaxyx(stdscr, screenHeight, screenWidth);
	boardWidth = screenWidth;
	boardHeight = screenHeight;
	setxy(argc, argv, &boardWidth, &boardHeight, screenWidth, screenHeight);

	// Initialising game window
	WINDOW *board = newwin(boardHeight, boardWidth,
					(screenHeight - boardHeight) >> 1,
					(screenWidth - boardWidth) >> 1);
	keypad(board, TRUE);

	// Print title
	title(board, boardWidth, boardHeight);

	// Generate board and internal representation; set snake speed
	initBoard(board, boardWidth, boardHeight);
	initUnderBoard(&underBoard, boardWidth, boardHeight);
	wtimeout(board, REFRESHRATE);

	// Game loop (with resets)
	while(TRUE){
		initSnek(board, underBoard, snek, boardWidth, boardHeight);

			// Game loop (without resets)
			while(kb != ESC){
				if((kb = wgetch(board)) != ERR)
					changeSnekDirection(snek, kb);

				// Game logic in here
				if(moveSnek(board, underBoard, snek, boardWidth, boardHeight))
					break;

				placeFood(board, boardWidth, boardHeight);
				wrefresh(board);
			}
		// Game over screen (includes exit)
		ggnore(board, underBoard, boardWidth, boardHeight, kb);
	}
}

/* Sets game size to screen size.
   Allows for user-defined game size, given as arguments to the program.
   Restricts game size between window size and 2.
*/
void setxy(int argc, char **argv, int *x, int *y, int sx, int sy){
	if(argc == 2)
		*y = atoi(*(argv+1));
	if(argc > 2){
		*x = atoi(*(argv+1));
		*y = atoi(*(argv+2));
	}

	*x = LIMIT(2, *x, sx);
	*y = LIMIT(2, *y, sy);
}

/* Ends game, curses and exits. */
void end(WINDOW *board){
	delwin(board);
	endwin();
	exit(0);
}

/* Prints the title screen and waits for key press.
   If the game is too small to print the big title, "SNEK" is printed instead.
   If both are too large (when game width is less than 4), neither are printed.
   If key press is the escape key, program exits.
*/
void title(WINDOW *board, int x, int y){
	if(x<4)
		return;
	else if(x<29||y<8)
		mvwaddstr(board, ((y-1)>>1), (x-4)>>1, "SNEK");
	else{
		char *t[8] ={	" O---O  ,_  ,-, ,---, ,-, __",
						"/ ___ \\ | \\ | | | __/ | |/ /",
						"\\ \'-_\\/ |  \\| | |  \\  | \' / ",
						" \'-_  \\ | |\\  | | _/  | , \\ ",
						"/\\_-\' / | | \\ | |  \'\\ | |\\ \\",
						"\\____/  \'-\'  \\| \'---\' \'-\' \\_\\",
						"   WASD/ARROW KEYS TO MOVE",
						"         ESC TO QUIT"
		};

		int j;
		for(j = 0; j < 6; j++)
			mvwaddstr(board, ((y-9)>>1) + j, (x-30)>>1, t[j]);

		mvwaddstr(board, ((y-9)>>1) + ++j, (x-30)>>1, t[j-1]);
		mvwaddstr(board, ((y-9)>>1) + ++j, (x-30)>>1, t[j-1]);
	}
	wrefresh(board);
	wtimeout(board, -1);
	if(wgetch(board) == ESC)
		end(board);
	wtimeout(board, REFRESHRATE);
}

/* Initialises internal representation of the game. */
void initUnderBoard(int ***board, int x, int y){
	*board = (int **) malloc(y * sizeof(int *));

	int i, j;
	for(j = 0; j < y; j++)
		(*board)[j] = (int *) calloc(x, sizeof(int));
}

/* Resets internal representation of the game. */
void resetUnderBoard(int **board, int x, int y){
	int i, j;
	for(j = 0; j < y; j++)
		for(i = 0; i < x; i++)
			board[j][i] = 0;
}

/* Initialises game onto screen. */
void initBoard(WINDOW *board, int x, int y){
	char *line = (char *) malloc((x+1) * sizeof(char));
	memset(line , '.', x);
	line[x] = '\0';

	int j;
	for(j = 0; j < y; j++)
		mvwaddnstr(board, j, 0, line, x);
	free(line);
	wrefresh(board);
}

/* Random number generator between 0 and num-1.
   Uses stdlib's rand(), which can be replaced if desired.
*/
int numGen(int num){
	if (num <= 0)
		return num ? -1 : 0;

	int random, limit = (RAND_MAX/num)*num;

	if(limit == 0)
		return 0;

	while((random = rand()) > limit)
		;
	return random % num;
}

/* Checks for a free spot in the game to place food for the snake. */
void placeFood(WINDOW *board, int x, int y){
	int i, j;
	for(j = 0; j < y; j++)
		for(i = 0; i < x; i++)
			if(mvwinch(board, j, i) == 'o')
				return;

	while(mvwinch(board, j = numGen(y-1), i = numGen(x-1)) != '.')
		;

	mvwaddch(board, j, i, 'o');
}

/* Game over screen; includes exit. */
void ggnore(WINDOW *board, int **underBoard, int x, int y, chtype c){
	if(x > 8)
		mvwprintw(board, y>>1, (x-9)>>1, "GAME OVER");
	wrefresh(board);
	wtimeout(board, -1);
	if(wgetch(board) == ESC || c == ESC)
		end(board);
	initBoard(board, x, y);
	resetUnderBoard(underBoard, x, y);
	wtimeout(board, REFRESHRATE);
}

/* Initialises snake, placing it in the middle of the screen, moving upwards. */
void initSnek(WINDOW *board, int **underBoard, struct Snek *snek, int x, int y){
	snek->x = x>>1;
	snek->y = y>>1;
	snek->length = 1;
	snek->dir = 'U';

	mvwaddch(board, snek->y, snek->x, '#');
	underBoard[snek->y][snek->x] = snek->length;
}

/* The snake is represented internally as integers that are bigger than 0;
    therefore, this function increments the snakes length, including each
    integer in the internal representation that is bigger than 0.
*/
void growSnek(WINDOW *board, int **underBoard, struct Snek *snek, int x, int y){
	(snek->length)++;
	int i, j;
	for(j = 0; j < y; j++)
		for(i = 0; i < x; i++)
			if(underBoard[j][i] > 0)
				underBoard[j][i]++;
}

/* Decrements each integer in the internal representation that is bigger than 0;
    any integer that formerly represented the snake returns to being part of the
    screen.
*/
void shrinkSnek
(WINDOW *board, int **underBoard, struct Snek *snek, int x, int y){
	int i, j;
	for(j = 0; j < y; j++)
		for(i = 0; i < x; i++){
			if(underBoard[j][i] > 0)
				underBoard[j][i]--;

			if(mvwinch(board, j, i) == '#' && underBoard[j][i] < 1)
				mvwaddch(board, j, i, '.');
		}
}

/* Moves the snake.
   Includes food, bite and out-of-bounds detection.
*/
int moveSnek(WINDOW *board, int **underBoard, struct Snek *snek, int x, int y){
	// New snake head co-ords are first calculated
	switch(snek->dir){
		case 'U':
			--(snek->y);
			break;

		case 'D':
			++(snek->y);
			break;

		case 'L':
			--(snek->x);
			break;

		case 'R':
			++(snek->x);
			break;
	}

	// Out-of-bounds
	if(snek->y < 0 || snek->x < 0 || snek->y > y-1 || snek->x > x-1)
		return -1;

	// Bite
	if(mvwinch(board, snek->y, snek->x) == '#')
		return -1;

	shrinkSnek(board, underBoard, snek, x, y);

	// Food
	if(mvwinch(board, snek->y, snek->x) == 'o')
		growSnek(board, underBoard, snek, x, y);

	// Snake head is moved within game and internal representation
	underBoard[snek->y][snek->x] = snek->length;
	mvwaddch(board, snek->y, snek->x, '#');
	return 0;
}

/* Changes the snakes direction if suitable arrow or WASD key is pressed. */
void changeSnekDirection(struct Snek *snek, chtype c){
	switch(c){
		case KEY_UP: case 'W': case 'w':
			if(snek->dir == 'L' || snek->dir == 'R')
				snek->dir = 'U';
			break;

		case KEY_DOWN: case 'S': case 's':
			if(snek->dir == 'L' || snek->dir == 'R')
				snek->dir = 'D';
			break;

		case KEY_LEFT: case 'A': case 'a':
			if(snek->dir == 'U' || snek->dir == 'D')
				snek->dir = 'L';
			break;

		case KEY_RIGHT: case 'D': case 'd':
			if(snek->dir == 'U' || snek->dir == 'D')
				snek->dir = 'R';
			break;
	}
}