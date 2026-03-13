#include <curses.h>
#include <stdlib.h>
#include <time.h>

typedef struct cell {
	bool hasMine;
	int surroundingMines;
	bool hasFlag;
	bool isCleared;
} cell;

typedef struct coord {
	int y;
	int x;
} coord;

/* game parameters */
#define FIELD_HEIGHT 20
#define FIELD_WIDTH 50
#define MINE_COUNT 99

/* color and pair definitions */
#define COLOR_DGREEN 8
#define COLOR_DDGREEN 9
#define COLOR_LLGREEN 10
#define PAIR_BORDER 1
#define PAIR_LFIELD 2
#define PAIR_DFIELD 3
#define PAIR_CURSOR 4
#define PAIR_FLAG 5
#define PAIR_CLEARED 6
#define PAIR_SAFE 7

/* actual macros */
#define CURSOR_ON(s) wattron(s,COLOR_PAIR(PAIR_CURSOR));wattron(s,A_BOLD)
#define CURSOR_OFF(s) wattroff(s,COLOR_PAIR(PAIR_CURSOR));wattroff(s,A_BOLD)

/* function definitions */
int gameLoop();
void fixCell(int yy, int xx);
char getCellContents(int yy, int xx);
void generateMines();
cell *getCell(int yy, int xx);
void clearZeros(int yy, int xx);

/* global variables */
WINDOW *field;
int cursorY;
int cursorX;
cell fieldState[(FIELD_HEIGHT-2)*(FIELD_WIDTH-2)];
bool firstMove = true;
int location;

int main() {
	/* window setup */
	initscr();
	if (!has_colors()) {
		printf("No color support :3\n");
		endwin();
		return 1;
	}
	raw();
	start_color();
	use_default_colors();
	init_color(COLOR_DGREEN, 0, 525, 0);
	init_color(COLOR_DDGREEN, 0 , 350, 0);
	init_color(COLOR_GREEN, 0, 700, 0);
	init_color(COLOR_WHITE, 750, 750, 750);
	init_color(COLOR_LLGREEN, 0, 850, 0);

	init_pair(PAIR_BORDER, COLOR_BLACK, COLOR_WHITE);
	init_pair(PAIR_LFIELD, COLOR_WHITE, COLOR_GREEN);
	init_pair(PAIR_DFIELD, COLOR_WHITE, COLOR_DGREEN);
	init_pair(PAIR_CURSOR, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(PAIR_FLAG, COLOR_YELLOW, COLOR_BLUE);
	init_pair(PAIR_CLEARED, COLOR_RED, COLOR_DDGREEN);
	init_pair(PAIR_SAFE, COLOR_WHITE, COLOR_LLGREEN);
	
	noecho();
	curs_set(0);
	keypad(stdscr,true);
	int max_y, max_x;
	getmaxyx(stdscr, max_y, max_x);
	refresh();
	field = newwin(FIELD_HEIGHT, FIELD_WIDTH,
			       (max_y/2)-(FIELD_HEIGHT/2), (max_x/2)-(FIELD_WIDTH/2));
	wattron(field,COLOR_PAIR(PAIR_BORDER));
	wborder(field,' ',' ',' ',' ',' ',' ',' ',' ');
	mvwprintw(field,0,1,"mines: %i",MINE_COUNT);
	for (int yy = 1; yy <= FIELD_HEIGHT-2; yy++) {
		for (int xx = 1; xx <= FIELD_WIDTH-2; xx++) {
			if ((xx % 2) ^ (yy % 2)) {
				wattron(field,COLOR_PAIR(PAIR_LFIELD));
			} else {
				wattron(field,COLOR_PAIR(PAIR_DFIELD));
			}
			mvwaddch(field,yy,xx,' ');
		}
	}
	cursorX = 1;
	cursorY = 1;
	CURSOR_ON(field);
	mvwaddch(field,cursorY,cursorX,' ');
	CURSOR_OFF(field);
	int fieldLength = (FIELD_HEIGHT-2)*(FIELD_WIDTH-2);
	for (int ii = 1; ii < fieldLength; ii++) {
		fieldState[ii].hasMine = false;
		fieldState[ii].surroundingMines = 0;
		fieldState[ii].hasFlag = false;
		fieldState[ii].isCleared = false;
	}
	wrefresh(field);
	int win = gameLoop();
	delwin(field);
	if (win == 0) {
		mvprintw(0,max_x/2,"you lose bozo");
		getch();
	} else if (win == 1) {
		mvprintw(0,max_x/2,"you win lmao");
		getch();
	}
	endwin();
	return 0;
}

int gameLoop() {
	int keyPress;
	while (true) {
		wrefresh(field);
		int location = (cursorY-1)+((FIELD_HEIGHT-2)*(cursorX-1));
		keyPress = getch();
		switch (keyPress) {
			case KEY_UP:
				if (cursorY > 1) {
					fixCell(cursorY-1,cursorX-1);
					cursorY--;
					CURSOR_ON(field);
					mvwaddch(field,cursorY,cursorX,getCellContents(cursorY,cursorX));
					CURSOR_OFF(field);
				}
				break;
			case KEY_DOWN:
				if (cursorY < (FIELD_HEIGHT-2)) {
					fixCell(cursorY-1,cursorX-1);
					cursorY++;
					CURSOR_ON(field);
					mvwaddch(field,cursorY,cursorX,getCellContents(cursorY,cursorX));
					CURSOR_OFF(field);
				}
				break;
			case KEY_LEFT:
				if (cursorX > 1) {
					fixCell(cursorY-1,cursorX-1);
					cursorX--;
					CURSOR_ON(field);
					mvwaddch(field,cursorY,cursorX,getCellContents(cursorY,cursorX));
					CURSOR_OFF(field);
				}
				break;
			case KEY_RIGHT:
				if (cursorX < (FIELD_WIDTH-2)) {
					fixCell(cursorY-1,cursorX-1);
					cursorX++;
					CURSOR_ON(field);
					mvwaddch(field,cursorY,cursorX,getCellContents(cursorY,cursorX));
					CURSOR_OFF(field);
				}
				break;
			case 'c':
			case 'C':
				if (!(fieldState[location].isCleared) && !(fieldState[location].hasFlag)) {
					if (firstMove) {
						generateMines();
						firstMove = false;
					}
					fieldState[location].isCleared = true;
					CURSOR_ON(field);
					mvwaddch(field,cursorY,cursorX,getCellContents(cursorY,cursorX));
					CURSOR_OFF(field);
					if (fieldState[location].surroundingMines == 0) {
						clearZeros(cursorY-1,cursorX-1);
						CURSOR_ON(field);
						mvwaddch(field,cursorY,cursorX,getCellContents(cursorY,cursorX));
					}
					CURSOR_OFF(field);
					if (fieldState[location].hasMine) {
						return 0;
					}
				}
				break;
			case 'x':
			case 'X':
				if (!(fieldState[location].isCleared)) {
					fieldState[location].hasFlag = !(fieldState[location].hasFlag);
					CURSOR_ON(field);
					mvwaddch(field,cursorY,cursorX,getCellContents(cursorY,cursorX));
					CURSOR_OFF(field);
					wattron(field,COLOR_PAIR(PAIR_BORDER));
					mvwprintw(field, 0, 8,"  ");
					int flagCount = 0;
					for (int ii = 0; ii < ((FIELD_HEIGHT-1)*(FIELD_WIDTH-2)); ii++) {
						if (fieldState[ii].hasFlag) {
							flagCount++;
						}
					}
					mvwprintw(field, 0, 8,"%d",MINE_COUNT-flagCount);
				}
				break;
			case 'q':
			case 'Q':
				return 2;
		}
	}
}

void fixCell(int yy, int xx) {
	if (getCell(yy,xx) == NULL) return;
	char cellContents;
	int location = (yy)+((FIELD_HEIGHT-2)*(xx));
	wattron(field,A_BOLD);
	if (fieldState[location].hasFlag) {
		wattron(field,COLOR_PAIR(PAIR_FLAG));
		cellContents = 'F';
	} else if (fieldState[location].isCleared) {
		if (fieldState[location].surroundingMines == 0) {
			wattron(field,COLOR_PAIR(PAIR_SAFE));
			cellContents = ' ';
		} else {
			wattron(field,COLOR_PAIR(PAIR_CLEARED));
			cellContents = '0' + fieldState[location].surroundingMines;
		}
	} else {
		if ((xx % 2) ^ (yy % 2)) {
			wattron(field,COLOR_PAIR(PAIR_LFIELD));
		} else {
			wattron(field,COLOR_PAIR(PAIR_DFIELD));
		} 
		cellContents = ' ';
	}
	mvwaddch(field,yy+1,xx+1,cellContents);
	wattroff(field,A_BOLD);
	return;
}

char getCellContents(int yy, int xx) {
	char cellContents;
	int location = (yy-1)+((FIELD_HEIGHT-2)*(xx-1));
	if (fieldState[location].hasFlag) {
		cellContents = 'F';
	} else if (fieldState[location].isCleared) {
		if (fieldState[location].surroundingMines == 0) {
			cellContents = ' ';
		} else {
			cellContents = '0' + fieldState[location].surroundingMines;
		}
	} else {
		cellContents = ' ';
	}
	return cellContents;
}

void generateMines() {
	int fieldLength = (FIELD_HEIGHT-2)*(FIELD_WIDTH-2);
	bool isValid;
	int poke;
	srand(time(NULL));
	for (int ii = 0; ii < MINE_COUNT; ii++) { //generate mines randomly across the field
		do {
			poke = rand() % (fieldLength);
			if (fieldState[poke].hasMine) {
				isValid = false;
			} else {
				isValid = true;
			}
		} while (!isValid);
		fieldState[poke].hasMine = true;
	}
	coord checkList[8] = {{-1,-1},{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0}};
	cell *stab;
	for (int ii = 0; ii < (FIELD_WIDTH-2); ii++) { //sum all surrounding mines for each cell
		for (int jj = 0; jj < (FIELD_HEIGHT-2); jj++) {
			for (int dir = 0; dir < 8; dir++) {
				if ((abs(checkList[dir].y + jj - cursorY-1) <= 1 ) && (abs(checkList[dir].x + ii - cursorX-1) <= 1)) {
					continue;
				}
				stab = getCell(checkList[dir].y + jj, checkList[dir].x + ii);
				if ((stab != NULL) && (stab->hasMine)) {
					getCell(jj,ii)->surroundingMines++;
				}
			}
		}
	}
}

void clearZeros(int yy, int xx) {
	coord checkList[8] = {{-1,-1},{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0}};
	cell *stab;
	getCell(yy,xx)->isCleared = true;
	for (int dir = 0; dir < 8; dir++) {
		int checkY = checkList[dir].y + yy;
		int checkX = checkList[dir].x + xx;
		stab = getCell(checkY, checkX);
		if (stab == NULL) continue;
		if ((stab->surroundingMines == 0) && !(stab->isCleared)) {
			clearZeros(checkY, checkX);	
		}
		stab->isCleared = true;
		fixCell(checkY, checkX);
	}
}

cell *getCell(int yy, int xx) {
	if (yy > (FIELD_HEIGHT-3)) { //inside bottom bound
		return NULL;
	}
	if (yy < 0) { //inside top bound
		return NULL;
	}
	if (xx > (FIELD_WIDTH-3)) { //inside right bound
		return NULL;
	}
	if (xx < 0) { //inside left bound
		return NULL;
	}
	return &(fieldState[yy+((FIELD_HEIGHT-2)*xx)]); //return cell
}
