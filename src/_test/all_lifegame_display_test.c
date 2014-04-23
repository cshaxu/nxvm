#define PRODUCT "Neko's Life Game\n\
Copyright (c) Neko 2013. All rights reserved.\n"

#define USAGE "INFO:\tcommand-line usage\n\
lifegame"

/*
http://bbs.chinaunix.net/thread-235738-1-1.html
http://www.verydemo.com/demo_c170_i6350.html
http://www.dreamincode.net/forums/topic/176599-linux-c-num-lock-probdetect-and-toggle/
http://www.cnblogs.com/Xiao_bird/archive/2009/07/21/1527947.html
http://bbs.kechuang.org/read-kc-tid-55903-page-e.html
*/

#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "memory.h"

typedef unsigned char t_bool;
typedef unsigned short t_nubit16;
typedef struct {
	int row,col;
	t_bool *space,*buffer;
	t_bool pause,exit;
} t_world;

#define WIN32 0
#define LINUX 1
#define PLATFORM WIN32

static t_nubit16 sizeRow, sizeCol;

#if PLATFORM == WIN32
#include "windows.h"
static HANDLE hIn, hOut;
static PCHAR_INFO charBuf,charBuf2;
static COORD coordDefaultBufSize, coordBufSize, coordBufStart;
static SMALL_RECT srctWriteRect;
static CONSOLE_CURSOR_INFO defaultCurInfo,curInfo;
static UINT defaultCodePage;
static CONSOLE_SCREEN_BUFFER_INFO defaultBufInfo;
static UCHAR bufComp[0x1000];
void w32cdispInit()
{
	GetConsoleCursorInfo(hOut, (PCONSOLE_CURSOR_INFO)(&defaultCurInfo));
	GetConsoleScreenBufferInfo(hOut, &defaultBufInfo);
	curInfo = defaultCurInfo;
	charBuf = NULL;
	curInfo.bVisible = 0;
	coordBufSize.X = sizeRow; // number of cols
	coordBufSize.Y = sizeCol; // number of rows
	coordBufStart.X = 0; 
	coordBufStart.Y = 0; 
	srctWriteRect.Top = 0;
	srctWriteRect.Bottom = sizeCol - 1;
	srctWriteRect.Left = 0;
	srctWriteRect.Right = sizeRow - 1;
	if (charBuf) free(charBuf);
	charBuf = (PCHAR_INFO)malloc(sizeCol * sizeRow * sizeof(CHAR_INFO));
	SetConsoleCursorInfo(hOut, &curInfo);
	SetConsoleOutputCP(437);
	SetConsoleScreenBufferSize(hOut, coordBufSize);
	SetConsoleMode(hIn, ENABLE_MOUSE_INPUT);
}
void w32cdispPaint(t_world *pworld)
{
	UCHAR ansiChar = ' ';
	WCHAR unicodeChar;
	UCHAR i, j;
	MultiByteToWideChar(437, 0, (LPCSTR)(&ansiChar), 1, (LPWSTR)(&unicodeChar), 1);
	for(i = 0;i < sizeCol;++i) {
		for(j = 0;j < sizeRow;++j) {
			charBuf[i * sizeRow + j].Char.UnicodeChar = unicodeChar;
			charBuf[i * sizeRow + j].Attributes = pworld->space[i * pworld->col + j] ? 0x00 : 0xff;
		}
	}
	WriteConsoleOutput(hOut, charBuf, coordBufSize, coordBufStart, &srctWriteRect);
}
void w32cdispFinal()
{
	if (charBuf) free((void *)charBuf);
	charBuf = NULL;
	SetConsoleCursorInfo(hOut, &defaultCurInfo);
	SetConsoleOutputCP(defaultCodePage);
	SetConsoleScreenBufferSize(hOut, defaultBufInfo.dwSize);
	hOut = INVALID_HANDLE_VALUE;
}
void w32ckeybProcess(t_world *pworld)
{
	INPUT_RECORD inRec;
	DWORD res;
	COORD pos;
	
	GetNumberOfConsoleInputEvents(hIn, &res);
	if (!res) return;
	ReadConsoleInput(hIn,&inRec,1,&res);
	if (inRec.EventType == MOUSE_EVENT) {
		if (inRec.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
			pos = inRec.Event.MouseEvent.dwMousePosition;
			charBuf[pos.Y * sizeRow + pos.X].Attributes = 0x00;
			pworld->space[pos.Y * pworld->col + pos.X] = 0x01;
		}
		if (inRec.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED) {
			pos = inRec.Event.MouseEvent.dwMousePosition;
			charBuf[pos.Y * sizeRow + pos.X].Attributes = 0xff;
			pworld->space[pos.Y * pworld->col + pos.X] = 0x00;
		}
	}
	if (inRec.EventType == KEY_EVENT)
		if (inRec.Event.KeyEvent.wVirtualKeyCode == VK_F9)
			pworld->exit = TRUE;
	if (GetKeyState(VK_CAPITAL) & 0x0001) pworld->pause = FALSE;
	else pworld->pause = TRUE;
}
#else
#include "unistd.h"
#include "curses.h"
#define GetMin(x, y) ((x) < (y) ? (x) : (y))
void lnxcdispInit()
{
	initscr();
	raw();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	noecho();
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_WHITE);
	init_pair(2, COLOR_BLACK, COLOR_BLACK);
}
void lnxcdispPaint(t_world *pworld)
{
	int i, j, c;
	sizeRow = GetMin(COLS, pworld->col);
	sizeCol = GetMin(LINES, pworld->row);
	for(i = 0;i < sizeCol;++i) {
		for(j = 0;j < sizeRow;++j) {
			move(i, j);
			c = ' ';
			if (pworld->space[i * pworld->col + j])
				c |= COLOR_PAIR(2);
			else
				c |= COLOR_PAIR(1);
			addch(c);
		}
	}
	refresh();
}
void lnxcdispFinal()
{
	endwin();
}
void lnxckeybProcess(t_world *pworld)
{
	if (getch() == KEY_F(9)) pworld->exit = 0x01;
}
#endif

#define value(pw, x, y) ((pw)->space[(x) * (pw)->col + (y)])

void apiInit(t_world *pworld)
{
#if PLATFORM == WIN32
	hIn = GetStdHandle(STD_INPUT_HANDLE);
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	sizeRow = pworld->col;
	sizeCol = pworld->row;
	w32cdispInit();
	w32cdispPaint(pworld);
#else
	lnxcdispInit();
	lnxcdispPaint(pworld);
#endif
}
void apiFinal(t_world *pworld)
{
#if PLATFORM == WIN32
	w32cdispFinal();
#else
	lnxcdispFinal();
#endif
}
void apiInput(t_world *pworld)
{
#if PLATFORM == WIN32
	w32ckeybProcess(pworld);
#else
	lnxckeybProcess(pworld);
#endif
}
void apiOutput(t_world *pworld)
{
#if PLATFORM == WIN32
	w32cdispPaint(pworld);
#else
	lnxcdispPaint(pworld);
#endif
}
void apiSleep(int milisec)
{
#if PLATFORM == WIN32
	Sleep(milisec);
#else
	usleep(milisec * 1000);
#endif
}

void worldInit(t_world *pworld)
{
	pworld->row = pworld->col = 0;
	pworld->space = NULL;
	pworld->pause = pworld->exit = 0x00;
}
void worldAlloc(t_world *pworld, int row, int col)
{
	int i,j;
	pworld->row = row;
	pworld->col = col;
	pworld->space = (t_bool *)malloc(row * col * sizeof(t_bool));
	pworld->buffer = (t_bool *)malloc(row * col * sizeof(t_bool));
	memset((void *)pworld->space, 0x00, row * col * sizeof(t_bool));
	srand(time(0));
	for (i = 0;i < row;++i) {
		for (j = 0;j < col;++j) {
			value(pworld, i, j) = rand() % 2;
		}
	}
}
void worldRefresh(t_world *pworld)
{
	int i, j, c;
	for (i = 0;i < pworld->row;++i) {
		for (j = 0;j < pworld->col;++j) {
			c = 0;
			c += (i == 0) ? 0 : value(pworld, i-1, j);
			c += (j == 0) ? 0 : value(pworld, i, j-1);
			c += (i == pworld->row - 1) ? 0 : value(pworld, i+1, j);
			c += (j == pworld->col - 1) ? 0 : value(pworld, i, j+1);
			c += (i == 0 || j == 0) ? 0 : value(pworld, i-1, j-1);
			c += (i == pworld->row - 1 || j == pworld->col - 1) ? 0 : value(pworld, i+1, j+1);
			c += (i == 0 || j == pworld->col - 1) ? 0 : value(pworld, i-1, j+1);
			c += (i == pworld->row - 1 || j == 0) ? 0 : value(pworld, i+1, j-1);
			switch (c) {
			case 2: pworld->buffer[i * pworld->col + j] = value(pworld, i, j);break;
			case 3: pworld->buffer[i * pworld->col + j] = 0x01;break;
			default:pworld->buffer[i * pworld->col + j] = 0x00;break;
			}
		}
	}
	memcpy((void *)(pworld->space), (void *)(pworld->buffer), pworld->row * pworld->col * sizeof(t_bool));
}
void worldFinal(t_world *pworld)
{
	if (pworld->space) free((void *)pworld->space);
	if (pworld->buffer) free((void *)pworld->buffer);
	pworld->row = pworld->col = 0;
	pworld->space = NULL;
	pworld->pause = pworld->exit = 0x00;
}

int main(int argc, char **argv)
{
	t_world world;
	fprintf(stdout,"%s\n",PRODUCT);
	worldInit(&world);
	worldAlloc(&world, 25, 80);
	apiInit(&world);
	while (!world.exit) {
		apiInput(&world);
		apiOutput(&world);
		apiSleep(50);
		if (!world.pause) worldRefresh(&world);
	}
	apiFinal(&world);
	worldFinal(&world);
	return 0;
}