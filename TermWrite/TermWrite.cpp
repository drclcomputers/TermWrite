#pragma warning (disable : 4996)

#include<iostream>
#include<fstream>
#include<cstring>
#include<string>
#include<vector>

#ifdef _WIN32
#define CLEAR_SCREEN "cls"
#include<conio.h>
#define NOMINMAX
#include<windows.h>
char key() {
	return _getch();
}
int getcolumns() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	return columns;
}
int getrows() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int rows;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return rows;
}
#else
#define CLEAR_SCREEN "clear"
#include <sys/ioctl.h>
#include <stdio.h>
#include<termios.h>
#include <unistd.h>
void setTerminalToRaw() {
	struct termios newt;
	tcgetattr(STDIN_FILENO, &newt);
	newt.c_lflag &= ~(ICANON | ECHO);  // Disable canonical mode and echo
	newt.c_cc[VMIN] = 1;              // Minimum number of characters to read
	newt.c_cc[VTIME] = 0;             // Timeout for read (0 for no timeout)
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void resetTerminal() {
	struct termios newt;
	tcgetattr(STDIN_FILENO, &newt);
	newt.c_lflag |= (ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

char key() {
	setTerminalToRaw();
	char ch;
	read(STDIN_FILENO, &ch, 1);
	resetTerminal();
	return ch;
}
int getcolumns() {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	return w.ws_col;
}
int getrows() {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	return w.ws_row;
}
#endif

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define WHITE "\033[37m"
#define BLUE "\033[34m"
#define GRAY "\033[90m"
#define BLACK "\033[30m"

#define REDBG "\033[41m"
#define GREENBG "\033[42m"
#define YELLOWBG "\033[43m"
#define WHITEBG "\033[47m"
#define BLUEBG "\033[44m"
#define GRAYBG "\033[100m"
#define BLACKBG "\033[40m"

std::vector <std::string> lines;
int term_height = getrows(), term_width = getcolumns();
int start_row = 1, end_row=term_height-2, start_column=1, end_column=term_width-3;
bool saved = 0;

int nrdig(int n) {
	int s = 0;
	do {
		n /= 10;
		s++;
	} while (n);
	return s;
}

void move_cursor(int row, int column) {
	std::cout << "\033[" << row << ';' << column+nrdig(row)+1 << 'H'; //\033[%d;%dH
}

void render(int row, int column, bool movemode) {
	system(CLEAR_SCREEN);
	move_cursor(1, -2);
	for (int i = start_row; i <= end_row && i < lines.size(); i++) {
		if (lines[i].length()) {
			std::string aux;
			int line_length = lines[i].length();
			int start = std::min(start_column - 1, line_length);
			int length = std::max(0, std::min(end_column - start - 2, line_length - start));

			aux = lines[i].substr(start, length);
			
			if (i == lines.size() - 1) std::cout << i << ' ' << aux;
			else std::cout << i << ' ' << aux << '\n';
		}
		else {
			if (i == lines.size() - 1) std::cout << i << ' ' << " ";
			else std::cout << i << ' ' << " " << '\n';
		}
	}

	move_cursor(term_height, -2);
	std::cout << "                                                 ";
	move_cursor(term_height, -2);
	std::cout << GRAYBG;
	if (!movemode) std::cout << "Write Mode ";
	else std::cout << "Move Mode ";
	std::cout << row << ' ' << column << "    ";
	if (!saved) std::cout << "*not saved*";
	else std::cout << "*saved*";
	std::cout << BLACKBG;
}

bool open_file(char* filename) {
	std::ifstream f(filename);
	if (!f) return 0;

	std::string aux;
	lines.push_back("");
	while (std::getline(f, aux))
		lines.push_back(aux);
	f.close();

	return 1;
}

void save_file(char* filename) {
	std::ofstream f(filename);
	for (int i = 2; i <= lines.size(); i++) {
		if (i == lines.size()) f << lines[i - 1];
		else f << lines[i - 1] << '\n';
	}
	f.close();
}

void save_gui() {
	system(CLEAR_SCREEN);
	move_cursor(1, -2);
	std::cout << "Do you wish to save it (y/n): ";
	char r = key();
	if (r == 'y') {
		saved = 1;
		move_cursor(1, -2);
		std::cout << "                                                     ";
		move_cursor(1, -2);
		std::cout << "Input filename (max 500 characters): ";
		char filename[501] = ""; std::cin.getline(filename, 500);
		save_file(filename);
	}
}

int edit_text() {
	int row = 1, column = 1;
	bool move_mode = 1;
	for (int i = lines.size(); i <= row; i++)
		lines.push_back("");
	for (int i = lines[row].length(); i <= column; i++)
		lines[row].push_back(' ');
	render(row, column, move_mode);
	while (true) {
		term_height = getrows(), term_width = getcolumns();

		if (row >= term_height - 2 && column >= term_width - 3) move_cursor(term_height - 2, term_width - 4);
		else if (row >= term_height - 2)  move_cursor(term_height - 2, column);
		else if (column >= term_width - 3) move_cursor(row, term_width - 4);
		else move_cursor(row, column);

		char keycap = key();

		if (move_mode) {
			if ((keycap == 119 || keycap == 87) && row > 1) //up
				if (row > term_height - 2 && start_row > 1) start_row--, end_row--, row--;
				else --row;
			else if (keycap == 115 || keycap == 83) //down
				if (row >= term_height - 2) start_row++, end_row++, row++;
				else row++;
			else if ((keycap == 97 || keycap == 65) && column > 1) //left
				if (column > term_width - 3 && start_column > 1) start_column--, end_column--, column--;
				else --column;
			else if (keycap == 100 || keycap == 68) //right
				if (column >= term_width - 3) start_column++, end_column++, column++;
				else ++column;
			else if (keycap == 27) //change mode
				move_mode = 0;
			else if (keycap == 113 || keycap == 81) { //quit
				if (!saved)
					save_gui();
				return 0;
			}
			else if (keycap == 88 || keycap == 120)
				save_gui();
		}
		else {
			if (keycap == 27) //change mode
				move_mode = 1;
			else if (keycap >= 32 && keycap <= 126) { //type
				if (column == 1) lines[row].insert(lines[row].begin(), keycap);
				else lines[row].insert(lines[row].begin() + column - 1, keycap);
				column++;
				if (column >= term_width - 3) start_column++, end_column++;
				if (saved) saved = 0;
			}
			else if (keycap == 8 || keycap == 127) { //backspace
				if (column > 1) {
					lines[row].erase(lines[row].begin() + column - 2);
					--column;
					if (column >= term_width - 4) start_column--, end_column--;
				}
				else if (row > 1) {
					if (!lines[row].empty()) lines[row - 1].append(lines[row]);
					column = lines[row - 1].length() - 1;
					if (column > term_width - 3) start_column=column-term_width+3, end_column = column;
					lines.erase(lines.begin() + row);
					--row;
					if (row >= term_height - 2) start_row--, end_row--;
				}

				if (saved) saved = 0;
			}
			else if (keycap == 127) {
				
			}
			else if (keycap == 13) { //newline
				if (column > term_width - 3) start_column = 1, end_column = term_width - 3;
				std::string rest;
				rest = lines[row].substr(column-1, lines[row].length());
				lines[row].erase(column-1, lines[row].length());
				lines.insert(lines.begin() + row + 1, rest);
				column = 1;
				++row;
				if (row > term_height - 2) start_row++, end_row--;
				if (saved) saved = 0;
			}
			else if (keycap == 9) { //tab
				lines[row].insert(column - 1, "    ");
				column += 4;
				if (column > term_width - 3) start_column += 4, end_column += 4;
				if (saved) saved = 0;
			}
		}
		for (int i = lines.size(); i <= row; i++)
			lines.push_back("");
		for (int i = lines[row].length(); i <= column; i++)
			lines[row].push_back(' ');

		render(row, column, move_mode);
	}
	return 1;
}

int main(int argc, char* argv[]) {
	//std::cout << "\033[2J";
	system(CLEAR_SCREEN);
	if (argc == 1) {
		edit_text();
	}
	else if (argc == 2) {
		if (!open_file(argv[1])) {
			std::cout << "File doesn't exist!";
			return 0;
		}
		saved = 1;
		edit_text();
	}
	else std::cout << "Too many arguments passed!\nType 'termwrite' to create a new file\nType 'termwrite <filename>' to open a document";
	
	system(CLEAR_SCREEN);
	
	return 0;
}