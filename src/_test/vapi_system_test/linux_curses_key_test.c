#include "curses.h"

int main(int argc, char **argv)
{
	int i;
	int x;
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	i = 1;
	do {
		x = getch();
		i = (i+1) % 2;
		if (i) move(0,0);
		printw("key value = %x\n",x);
	} while (x != KEY_F(9));
	noraw();
	keypad(stdscr, FALSE);
	endwin();
	return 0;
}