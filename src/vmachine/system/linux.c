/* This file is a part of NXVM project. */

#include "unistd.h"
#include "curses.h"
#include "pthread.h"

#include "../vapi.h"
#include "linux.h"

#define GetMin(x, y) ((x) < (y) ? (x) : (y))

void linuxSleep(int milisec)
{
	usleep((milisec) * 1000);
}

#define COLOR_GRAY         0x08
#define COLOR_LIGHTBLUE    0x09
#define COLOR_LIGHTGREEN   0x0a
#define COLOR_LIGHTCYAN    0x0b
#define COLOR_LIGHTRED     0x0c
#define COLOR_LIGHTMAGENTA 0x0d
#define COLOR_BROWN        0x0e
#define COLOR_LIGHTGRAY    0x0f

void linuxDisplayInit()
{
	int i,j;
	initscr();
	raw();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	noecho();
	start_color();
	for (i = 0;i < 8;++i) {
		for(j = 0;j < 8;++j) {
			init_pair(i * 8 + j, i, j);
		}
	}
}
void linuxDisplayFinal()
{
	noraw();
	nodelay(stdscr, FALSE);
	keypad(stdscr, FALSE);
	endwin();
}
void linuxDisplaySetScreen()
{
	/* there's no way to set screen */
}

static unsigned char CharProp2Color(unsigned char value)
{
	value &= 0x07;
	switch (value) {
	case 0x00: return COLOR_BLACK;       break;
	case 0x01: return COLOR_BLUE;        break;
	case 0x02: return COLOR_GREEN;       break;
	case 0x03: return COLOR_CYAN;        break;
	case 0x04: return COLOR_RED;         break;
	case 0x05: return COLOR_MAGENTA;     break;
	case 0x06: return COLOR_YELLOW;      break;
	case 0x07: return COLOR_WHITE;       break;
/*	case 0x08: return COLOR_GRAY;        break;
	case 0x09: return COLOR_LIGHTBLUE;   break;
	case 0x0a: return COLOR_LIGHTGREEN;  break;
	case 0x0b: return COLOR_LIGHTCYAN;   break;
	case 0x0c: return COLOR_LIGHTRED;    break;
	case 0x0d: return COLOR_LIGHTMAGENTA;break;
	case 0x0e: return COLOR_YELLOW;      break;
	case 0x0f: return COLOR_WHITE;       break;*/
	default:   return COLOR_BLACK;       break;
	}
}
static unsigned char GetColorFromProp(unsigned char prop)
{
	unsigned char fore,back;
	fore = prop & 0x0f;
	back = ((prop & 0x70) >> 4);
	fore = CharProp2Color(fore);
	back = CharProp2Color(back);
	return (fore * 8 + back);
}
void linuxDisplayPaint(unsigned char force)
{
	unsigned char ref,p,c;
	int i, j, sizeRow, sizeCol, curX, curY;
	sizeRow = GetMin(COLS, vapiCallBackDisplayGetRowSize());
	sizeCol = GetMin(LINES, vapiCallBackDisplayGetColSize());
	ref = 0x00;
	if (force || vapiCallBackDisplayGetBufferChange()) {
		clear();
		for(i = 0;i < sizeCol;++i) {
			for(j = 0;j < sizeRow;++j) {
				c = vapiCallBackDisplayGetCurrentChar(i, j);
				p = vapiCallBackDisplayGetCurrentCharProp(i, j);
				if (!c) c = ' ';
				move(i, j);
				addch(c | COLOR_PAIR(GetColorFromProp(p)));
			}
		}
		ref = 0x01;
	}
	if (force || vapiCallBackDisplayGetCursorChange()) {
		curX = vapiCallBackDisplayGetCurrentCursorPosX();
		curY = vapiCallBackDisplayGetCurrentCursorPosY();
		if (curX < sizeCol && curY < sizeRow)
			move(vapiCallBackDisplayGetCurrentCursorPosX(),
				vapiCallBackDisplayGetCurrentCursorPosY());
		else
			move(0, 0);
		ref = 0x01;
	}
	if (ref) refresh();
}

static void *ThreadDisplay(void *arg)
{
	linuxDisplayInit();
	linuxDisplayPaint(0x01);
	while (vapiCallBackMachineGetFlagRun()) {
		linuxDisplayPaint(0x00);
		vapiSleep(100);
	}
	linuxDisplayFinal();
	return 0;
}
static void *ThreadKernel(void *arg)
{
	vapiCallBackMachineRun();
	return 0;
}

void linuxKeyboardMakeStatus() {}

#define ZERO 0x00

static unsigned char Ascii2ScanCode[][2] = {
	{0x00, ZERO}, {0x01, ZERO}, {0x02, ZERO}, {0x03, ZERO},
	{0x04, ZERO}, {0x05, ZERO}, {0x06, ZERO}, {0x07, ZERO},
	{0x08, 0x0e}, {0x09, 0x0f}, {0x0a, ZERO}, {0x0b, ZERO},
	{0x0c, ZERO}, {0x0d, ZERO}, {0x0e, ZERO}, {0x0f, ZERO},
	{0x10, ZERO}, {0x11, ZERO}, {0x12, ZERO}, {0x13, ZERO},
	{0x14, ZERO}, {0x15, ZERO}, {0x16, ZERO}, {0x17, ZERO},
	{0x18, ZERO}, {0x19, ZERO}, {0x1a, ZERO}, {0x1b, ZERO},
	{0x1c, ZERO}, {0x1d, ZERO}, {0x1e, ZERO}, {0x1f, ZERO},
	{0x20, 0x39}, {0x21, 0x02}, {0x22, 0x28}, {0x23, 0x04},
	{0x24, 0x05}, {0x25, 0x06}, {0x26, 0x08}, {0x27, 0x28},
	{0x28, 0x0a}, {0x29, 0x0b}, {0x2a, 0x09}, {0x2b, 0x0d},
	{0x2c, 0x33}, {0x2d, 0x0c}, {0x2e, 0x34}, {0x2f, 0x35},
	{0x30, 0x0b}, {0x31, 0x02}, {0x32, 0x03}, {0x33, 0x04},
	{0x34, 0x05}, {0x35, 0x06}, {0x36, 0x07}, {0x37, 0x08},
	{0x38, 0x09}, {0x39, 0x0a}, {0x3a, 0x27}, {0x3b, 0x27},
	{0x3c, 0x33}, {0x3d, 0x0d}, {0x3e, 0x34}, {0x3f, 0x35},
	{0x40, 0x03}, {0x41, 0x1e}, {0x42, 0x30}, {0x43, 0x2e},
	{0x44, 0x20}, {0x45, 0x12}, {0x46, 0x21}, {0x47, 0x22},
	{0x48, 0x23}, {0x49, 0x17}, {0x4a, 0x24}, {0x4b, 0x25},
	{0x4c, 0x26}, {0x4d, 0x32}, {0x4e, 0x31}, {0x4f, 0x18},
	{0x50, 0x19}, {0x51, 0x10}, {0x52, 0x13}, {0x53, 0x1f},
	{0x54, 0x14}, {0x55, 0x16}, {0x56, 0x2f}, {0x57, 0x11},
	{0x58, 0x2d}, {0x59, 0x15}, {0x5a, 0x2c}, {0x5b, 0x1a},
	{0x5c, 0x2b}, {0x5d, 0x1b}, {0x5e, 0x07}, {0x5f, 0x0c},
	{0x60, 0x29}, {0x61, 0x1e}, {0x62, 0x30}, {0x63, 0x2e},
	{0x64, 0x20}, {0x65, 0x12}, {0x66, 0x21}, {0x67, 0x22},
	{0x68, 0x23}, {0x69, 0x17}, {0x6a, 0x24}, {0x6b, 0x25},
	{0x6c, 0x26}, {0x6d, 0x32}, {0x6e, 0x31}, {0x6f, 0x18},
	{0x70, 0x19}, {0x71, 0x10}, {0x72, 0x13}, {0x73, 0x1f},
	{0x74, 0x14}, {0x75, 0x16}, {0x76, 0x2f}, {0x77, 0x11},
	{0x78, 0x2d}, {0x79, 0x15}, {0x7a, 0x2c}, {0x7b, 0x1a},
	{0x7c, 0x2b}, {0x7d, 0x1b}, {0x7e, 0x29}, {0x7f, ZERO},
	{0x80, ZERO}, {0x81, ZERO}, {0x82, ZERO}, {0x83, ZERO},
	{0x84, ZERO}, {0x85, ZERO}, {0x86, ZERO}, {0x87, ZERO},
	{0x88, ZERO}, {0x89, ZERO}, {0x8a, ZERO}, {0x8b, ZERO},
	{0x8c, ZERO}, {0x8d, ZERO}, {0x8e, ZERO}, {0x8f, ZERO},
	{0x90, ZERO}, {0x91, ZERO}, {0x92, ZERO}, {0x93, ZERO},
	{0x94, ZERO}, {0x95, ZERO}, {0x96, ZERO}, {0x97, ZERO},
	{0x98, ZERO}, {0x99, ZERO}, {0x9a, ZERO}, {0x9b, ZERO},
	{0x9c, ZERO}, {0x9d, ZERO}, {0x9e, ZERO}, {0x9f, ZERO},
	{0xa0, ZERO}, {0xa1, ZERO}, {0xa2, ZERO}, {0xa3, ZERO},
	{0xa4, ZERO}, {0xa5, ZERO}, {0xa6, ZERO}, {0xa7, ZERO},
	{0xa8, ZERO}, {0xa9, ZERO}, {0xaa, ZERO}, {0xab, ZERO},
	{0xac, ZERO}, {0xad, ZERO}, {0xae, ZERO}, {0xaf, ZERO},
	{0xb0, ZERO}, {0xb1, ZERO}, {0xb2, ZERO}, {0xb3, ZERO},
	{0xb4, ZERO}, {0xb5, ZERO}, {0xb6, ZERO}, {0xb7, ZERO},
	{0xb8, ZERO}, {0xb9, ZERO}, {0xba, ZERO}, {0xbb, ZERO},
	{0xbc, ZERO}, {0xbd, ZERO}, {0xbe, ZERO}, {0xbf, ZERO},
	{0xc0, ZERO}, {0xc1, ZERO}, {0xc2, ZERO}, {0xc3, ZERO},
	{0xc4, ZERO}, {0xc5, ZERO}, {0xc6, ZERO}, {0xc7, ZERO},
	{0xc8, ZERO}, {0xc9, ZERO}, {0xca, ZERO}, {0xcb, ZERO},
	{0xcc, ZERO}, {0xcd, ZERO}, {0xce, ZERO}, {0xcf, ZERO},
	{0xd0, ZERO}, {0xd1, ZERO}, {0xd2, ZERO}, {0xd3, ZERO},
	{0xd4, ZERO}, {0xd5, ZERO}, {0xd6, ZERO}, {0xd7, ZERO},
	{0xd8, ZERO}, {0xd9, ZERO}, {0xda, ZERO}, {0xdb, ZERO},
	{0xdc, ZERO}, {0xdd, ZERO}, {0xde, ZERO}, {0xdf, ZERO},
	{0xe0, ZERO}, {0xe1, ZERO}, {0xe2, ZERO}, {0xe3, ZERO},
	{0xe4, ZERO}, {0xe5, ZERO}, {0xe6, ZERO}, {0xe7, ZERO},
	{0xe8, ZERO}, {0xe9, ZERO}, {0xea, ZERO}, {0xeb, ZERO},
	{0xec, ZERO}, {0xed, ZERO}, {0xee, ZERO}, {0xef, ZERO},
	{0xf0, ZERO}, {0xf1, ZERO}, {0xf2, ZERO}, {0xf3, ZERO},
	{0xf4, ZERO}, {0xf5, ZERO}, {0xf6, ZERO}, {0xf7, ZERO},
	{0xf8, ZERO}, {0xf9, ZERO}, {0xfa, ZERO}, {0xfb, ZERO},
	{0xfc, ZERO}, {0xfd, ZERO}, {0xfe, ZERO}, {0xff, ZERO}
};
#define mc(n) if (1) {code = (n); break;} else
void linuxKeyboardMakeKey(int keyvalue)
{
	unsigned short code;
	if (keyvalue == KEY_F(9)) vapiCallBackMachineStop();
	if (keyvalue < 0x0100) {
		switch (keyvalue) {
		case 0x0a:	/* ENTER */
			mc(0x1c0d);
		default:
			code = (Ascii2ScanCode[keyvalue][1] << 8) | keyvalue;
			break;
		}
	} else if (keyvalue > KEY_F0 && keyvalue <= KEY_F(12)) {
		code = ((keyvalue - KEY_F0 + 0x3a) << 8);
	} else {
		/* get special keys */
		switch (keyvalue) {
		case KEY_DOWN:
			mc(0x5000);
		case KEY_UP:
			mc(0x4800);
		case KEY_LEFT:
			mc(0x4b00);
		case KEY_RIGHT:
			mc(0x4d00);
		case KEY_HOME:
			mc(0x4700);
		case KEY_BACKSPACE:
			mc(0x0e08);
		case KEY_ENTER:
			mc(0x1c0d);
		case KEY_NPAGE:
			mc(0x5100);
		case KEY_PPAGE:
			mc(0x4900);
		case KEY_END:
			mc(0x4f00);
		default:
			return;
		}
	}
	/*printw("key = %d\n",keyvalue);*/
	vapiCallBackKeyboardRecvKeyPress(code);
}
void linuxKeyboardProcess()
{
	int keyvalue = getch();
	if (keyvalue != ERR)
		linuxKeyboardMakeKey(keyvalue);
}

void linuxStartMachine()
{
	pthread_t ThreadIdDisplay;
	pthread_t ThreadIdKernel;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_create(&ThreadIdDisplay, &attr, ThreadDisplay, NULL);
	pthread_create(&ThreadIdKernel,  &attr, ThreadKernel,  NULL);
/*	linuxDisplayInit();*/
	while (vapiCallBackMachineGetFlagRun()) {
		vapiSleep(20);
		linuxKeyboardProcess();
	}
/*	linuxDisplayFinal();*/
	pthread_attr_destroy(&attr);
}
