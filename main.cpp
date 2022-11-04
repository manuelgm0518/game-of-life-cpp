#include <stdio.h>
#include <conio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define clear_console() SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });
#else
#define clear_console() system("clear");
#endif


/* CONFIGURATION CONSTANTS */
const int BOARD_WIDTH = 30;
const int BOARD_HEIGHT = 20;
const int GENERATION_DELAY_MS = 200;

const int ACTIVE_CELL_CHAR = 254; //219
const int INACTIVE_CELL_CHAR = 32;
const int ACTIVE_CURSOR_CHAR = 2;
const int INACTIVE_CURSOR_CHAR = 1;

const int FINISH_KEY = 27; // Esc
const int PAUSE_KEY = 80; // P
const int RESET_KEY = 82; // R
const int NEXT_KEY = 78; // N

const int MOVE_UP_KEY = 87; // W
const int MOVE_LEFT_KEY = 65; // A
const int MOVE_DOWN_KEY = 83; // S
const int MOVE_RIGHT_KEY = 68; // D
const int INTERACT_KEY = 32; // Space


/* GLOBAL VARIABLES */
bool paused = true;
bool finish = false;
int cursor_x = 0;
int cursor_y = 0;
bool* board = new bool[BOARD_WIDTH * BOARD_HEIGHT];


/* UTIL FUNCTIONS */
void move_cursor(int x, int y) {
	if (cursor_x + x >= 0 && cursor_x + x < BOARD_WIDTH)
		cursor_x += x;
	if (cursor_y + y >= 0 && cursor_y + y < BOARD_HEIGHT)
		cursor_y += y;
}

bool get_board_value(int x, int y) {
	if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT)
		return board[x + y * BOARD_WIDTH];
	return 0;
}

void set_board_value(int x, int y, bool value) {
	if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT)
		board[x + y * BOARD_WIDTH] = value;
}

void interact() {
	bool value = get_board_value(cursor_x, cursor_y) == 1;
	set_board_value(cursor_x, cursor_y, !value);
}

int count_neighbors(int x, int y) {
	int neighbors = 0, i, row, col;
	for (i = 0; i < 9; i++) {
		row = -1 + i / 3, col = -1 + i % 3;
		if ((row != 0 || col != 0) && get_board_value(x + col, y + row) == 1)
			neighbors++;
	}
	return neighbors;
}

void ingame_delay() {
	clock_t start_time = clock();
	while (clock() < start_time + GENERATION_DELAY_MS && !paused);
}

void print_border(char start, char middle, char end, int width) {
	int i;
	printf_s("%c", start);
	for (i = 0; i <width; i++)
		printf_s("%c", middle);
	printf_s("%c\n", end);
}



/* MAIN FUNCTIONS */
void generation() {
	bool* board_copy = new bool[BOARD_WIDTH * BOARD_HEIGHT];
	int x, y, cell, neighbors;
	for (y = 0; y < BOARD_HEIGHT; y++) {
		for (x = 0; x < BOARD_WIDTH; x++) {
			cell = get_board_value(x, y);
			neighbors = count_neighbors(x, y);
			if (cell == 1 && (neighbors == 2 || neighbors == 3))
				board_copy[x + y * BOARD_WIDTH] = 1;
			else if (cell != 1 && neighbors == 3)
				board_copy[x + y * BOARD_WIDTH] = 1;
			else board_copy[x + y * BOARD_WIDTH] = 0;
		}
	}
	for (y = 0; y < BOARD_HEIGHT; y++) 
		for (x = 0; x < BOARD_WIDTH; x++) 
			set_board_value(x, y, board_copy[x + y * BOARD_WIDTH]);
	delete[] board_copy;
}

void command_reader(char pressed_key) {	
	switch (int(pressed_key)) {
	case PAUSE_KEY: paused = !paused; break;
	case FINISH_KEY: finish = true; break;
	case NEXT_KEY: generation(); break;
	case RESET_KEY: 
		paused = true;
		board = new bool[BOARD_WIDTH * BOARD_HEIGHT];
		break;
	}
}

void movement_reader(char pressed_key) {
	switch (int(pressed_key)) {
	case MOVE_UP_KEY: move_cursor(0, -1); break;
	case MOVE_LEFT_KEY: move_cursor(-1, 0); break;
	case MOVE_DOWN_KEY: move_cursor(0, 1); break;
	case MOVE_RIGHT_KEY: move_cursor(1, 0); break;
	case INTERACT_KEY: interact(); break;
	}
}

void print_commands() {
	int padding = (BOARD_WIDTH * 2 - 34) / 2;
	print_border(201, 205, 187, BOARD_WIDTH * 2 + 1);
	printf_s("%-*c %3s: Exit  %c %3s: Interact %c Move: %*c\n", padding, 186,  "Esc", 179, "Spc", 179, padding + 1, 186);
	printf_s("%-*c %3c: %-5s %c %3c: Next     %c   %c   %*c\n", padding, 186, PAUSE_KEY, paused?"Play": "Pause", 179, NEXT_KEY, 179, MOVE_UP_KEY, padding + 1, 186);
	printf_s("%-*c %3c: Reset %c               %c %c %c %c %*c\n", padding, 186, RESET_KEY, 179, 179, MOVE_LEFT_KEY, MOVE_DOWN_KEY, MOVE_RIGHT_KEY, padding + 1, 186);
	print_border(200, 205, 188, BOARD_WIDTH * 2 + 1);
}

void print_board() {
	int x, y;
	print_border(218, 196, 191, BOARD_WIDTH * 2 + 1);
	for (y = 0; y < BOARD_HEIGHT; y++) {
		printf_s("%c", 179);
		for (x = 0; x < BOARD_WIDTH; x++) {
			bool value = get_board_value(x, y);
			bool show_cursor = x == cursor_x && cursor_y == y && paused;
			int neighbors = count_neighbors(x, y);
			char icon = value == 1 ? ACTIVE_CELL_CHAR : INACTIVE_CELL_CHAR;
			if (show_cursor) icon = value == 1 ? ACTIVE_CURSOR_CHAR : INACTIVE_CURSOR_CHAR;
			printf_s(" %c", icon);
		}
		printf_s(" %c\n", 179);
	}
	print_border(192, 196, 217, BOARD_WIDTH * 2 + 1);
}

void print_screen() {
	clear_console();
	print_commands();
	print_board();
}

int main() {
	char pressed_key;	
	while (true) {
		if (_kbhit()) {
			pressed_key = toupper(_getch());
			command_reader(pressed_key);
			if (paused) movement_reader(pressed_key);
			print_screen();
		}
		if (finish) break;
		if (!paused) {
			generation();
			ingame_delay();
			print_screen();
		}
	}
	delete[] board;
	return 0;
}
