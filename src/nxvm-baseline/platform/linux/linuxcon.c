/* Copyright 2012-2014 Neko. */

/* LINUXCON provides linux terminal interface. */

#include <curses.h>
#include <pthread.h>

#include "../../utils.h"
#include "../../device/device.h"

#include "linuxcon.h"

#define GetMin(x, y) ((x) < (y) ? (x) : (y))
#define ZERO 0x00

#define COLOR_GRAY         0x08
#define COLOR_LIGHTBLUE    0x09
#define COLOR_LIGHTGREEN   0x0a
#define COLOR_LIGHTCYAN    0x0b
#define COLOR_LIGHTRED     0x0c
#define COLOR_LIGHTMAGENTA 0x0d
#define COLOR_BROWN        0x0e
#define COLOR_LIGHTGRAY    0x0f

static void lnxcdispInit() {
    size_t i, j;
    initscr();
    raw();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    noecho();
    start_color();
    for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j) {
            init_pair(i * 8 + j, i, j);
        }
    }
}

static void lnxcdispFinal() {
    noraw();
    nodelay(stdscr, FALSE);
    keypad(stdscr, FALSE);
    endwin();
}

static uint8_t ReverseColor(uint8_t value) {
    value &= 0x07;
    switch (value) {
    case COLOR_BLACK:
        return COLOR_WHITE;
        break;
    case COLOR_BLUE:
        return COLOR_YELLOW;
        break;
    case COLOR_GREEN:
        return COLOR_RED;
        break;
    case COLOR_CYAN:
        return COLOR_MAGENTA;
        break;
    case COLOR_RED:
        return COLOR_GREEN;
        break;
    case COLOR_MAGENTA:
        return COLOR_CYAN;
        break;
    case COLOR_YELLOW:
        return COLOR_BLUE;
        break;
    case COLOR_WHITE:
        return COLOR_BLACK;
        break;
        /*	case 0x08: return COLOR_GRAY;        break;
        	case 0x09: return COLOR_LIGHTBLUE;   break;
        	case 0x0a: return COLOR_LIGHTGREEN;  break;
        	case 0x0b: return COLOR_LIGHTCYAN;   break;
        	case 0x0c: return COLOR_LIGHTRED;    break;
        	case 0x0d: return COLOR_LIGHTMAGENTA;break;
        	case 0x0e: return COLOR_YELLOW;      break;
        	case 0x0f: return COLOR_WHITE;       break;*/
    }
    return COLOR_BLACK;
}

static uint8_t CharProp2Color(uint8_t value) {
    value &= 0x07;
    switch (value) {
    case 0x00:
        return COLOR_BLACK;
        break;
    case 0x01:
        return COLOR_BLUE;
        break;
    case 0x02:
        return COLOR_GREEN;
        break;
    case 0x03:
        return COLOR_CYAN;
        break;
    case 0x04:
        return COLOR_RED;
        break;
    case 0x05:
        return COLOR_MAGENTA;
        break;
    case 0x06:
        return COLOR_YELLOW;
        break;
    case 0x07:
        return COLOR_WHITE;
        break;
        /*	case 0x08: return COLOR_GRAY;        break;
        	case 0x09: return COLOR_LIGHTBLUE;   break;
        	case 0x0a: return COLOR_LIGHTGREEN;  break;
        	case 0x0b: return COLOR_LIGHTCYAN;   break;
        	case 0x0c: return COLOR_LIGHTRED;    break;
        	case 0x0d: return COLOR_LIGHTMAGENTA;break;
        	case 0x0e: return COLOR_YELLOW;      break;
        	case 0x0f: return COLOR_WHITE;       break;*/
    }
    return COLOR_BLACK;
}

static uint8_t GetColorFromProp(uint8_t prop) {
    uint8_t fore0, back0, fore1, back1;
    fore0 = prop & 0x0f;
    back0 = ((prop & 0x70) >> 4);
    fore1 = CharProp2Color(fore0);
    back1 = CharProp2Color(back0);
    if (fore0 != back0 && fore1 == back1) {
        fore1 = ReverseColor(fore1);
    }
    return (fore1 * 8 + back1);
}

static uint8_t Ascii2Print[][2] = {
    {0x00, ' ' }, {0x01, '*' }, {0x02, '*' }, {0x03, '*' },
    {0x04, '*' }, {0x05, '*' }, {0x06, '*' }, {0x07, 0x07},
    {0x08, 0x08}, {0x09, 0x09}, {0x0a, 0x0a}, {0x0b, 0x0b},
    {0x0c, 0x0c}, {0x0d, 0x0d}, {0x0e, '*' }, {0x0f, '*' },
    {0x10, '>' }, {0x11, '<' }, {0x12, '|' }, {0x13, '!' },
    {0x14, 'T' }, {0x15, '$' }, {0x16, '_' }, {0x17, '|' },
    {0x18, '^' }, {0x19, 'v' }, {0x1a, '>' }, {0x1b, '<' },
    {0x1c, 'L' }, {0x1d, '=' }, {0x1e, '^' }, {0x1f, 'v' },
    {0x20, ' '}, {0x21, '!'}, {0x22,'\"'}, {0x23, '#'},
    {0x24, '$'}, {0x25, '%'}, {0x26, '&'}, {0x27,'\''},
    {0x28, '('}, {0x29, ')'}, {0x2a, '*'}, {0x2b, '+'},
    {0x2c, ','}, {0x2d, '-'}, {0x2e, '.'}, {0x2f, '/'},
    {0x30, '0'}, {0x31, '1'}, {0x32, '2'}, {0x33, '3'},
    {0x34, '4'}, {0x35, '5'}, {0x36, '6'}, {0x37, '7'},
    {0x38, '8'}, {0x39, '9'}, {0x3a, ':'}, {0x3b, ';'},
    {0x3c, '<'}, {0x3d, '='}, {0x3e, '>'}, {0x3f, '?'},
    {0x40, '@'}, {0x41, 'A'}, {0x42, 'B'}, {0x43, 'C'},
    {0x44, 'D'}, {0x45, 'E'}, {0x46, 'F'}, {0x47, 'G'},
    {0x48, 'H'}, {0x49, 'I'}, {0x4a, 'J'}, {0x4b, 'K'},
    {0x4c, 'L'}, {0x4d, 'M'}, {0x4e, 'N'}, {0x4f, 'O'},
    {0x50, 'P'}, {0x51, 'Q'}, {0x52, 'R'}, {0x53, 'S'},
    {0x54, 'T'}, {0x55, 'U'}, {0x56, 'V'}, {0x57, 'W'},
    {0x58, 'X'}, {0x59, 'Y'}, {0x5a, 'Z'}, {0x5b, '['},
    {0x5c,'\\'}, {0x5d, ']'}, {0x5e, '^'}, {0x5f, '_'},
    {0x60, '`'}, {0x61, 'a'}, {0x62, 'b'}, {0x63, 'c'},
    {0x64, 'd'}, {0x65, 'e'}, {0x66, 'f'}, {0x67, 'g'},
    {0x68, 'h'}, {0x69, 'i'}, {0x6a, 'j'}, {0x6b, 'k'},
    {0x6c, 'l'}, {0x6d, 'm'}, {0x6e, 'n'}, {0x6f, 'o'},
    {0x70, 'p'}, {0x71, 'q'}, {0x72, 'r'}, {0x73, 's'},
    {0x74, 't'}, {0x75, 'u'}, {0x76, 'v'}, {0x77, 'w'},
    {0x78, 'x'}, {0x79, 'y'}, {0x7a, 'z'}, {0x7b, '{'},
    {0x7c, '|'}, {0x7d, '}'}, {0x7e, '~'}, {0x7f,0x7f},
    {0x80, 'C'}, {0x81, 'u'}, {0x82, 'e'}, {0x83, 'a'},
    {0x84, 'a'}, {0x85, 'a'}, {0x86, 'a'}, {0x87, 'c'},
    {0x88, 'e'}, {0x89, 'e'}, {0x8a, 'e'}, {0x8b, 'i'},
    {0x8c, 'i'}, {0x8d, 'i'}, {0x8e, 'A'}, {0x8f, 'A'},
    {0x90, 'E'}, {0x91, 'a'}, {0x92, 'A'}, {0x93, 'o'},
    {0x94, 'o'}, {0x95, 'o'}, {0x96, 'u'}, {0x97, 'u'},
    {0x98, 'y'}, {0x99, 'O'}, {0x9a, 'U'}, {0x9b, 'C'},
    {0x9c, 'L'}, {0x9d, 'Y'}, {0x9e, 'P'}, {0x9f, 'f'},
    {0xa0, 'a'}, {0xa1, 'i'}, {0xa2, 'o'}, {0xa3, 'u'},
    {0xa4, 'n'}, {0xa5, 'N'}, {0xa6, 'a'}, {0xa7, 'o'},
    {0xa8, '?'}, {0xa9, '+'}, {0xaa, '+'}, {0xab, '/'},
    {0xac, '/'}, {0xad, 'i'}, {0xae, '<'}, {0xaf, '>'},
    {0xb0, '*'}, {0xb1, '*'}, {0xb2, '*'}, {0xb3, '|'},
    {0xb4, '|'}, {0xb5, '|'}, {0xb6, '|'}, {0xb7, '+'},
    {0xb8, '+'}, {0xb9, '|'}, {0xba, '|'}, {0xbb, '+'},
    {0xbc, '+'}, {0xbd, '+'}, {0xbe, '+'}, {0xbf, '+'},
    {0xc0, '+'}, {0xc1, '+'}, {0xc2, '+'}, {0xc3, '|'},
    {0xc4, '-'}, {0xc5, '+'}, {0xc6, '|'}, {0xc7, '|'},
    {0xc8, '+'}, {0xc9, '+'}, {0xca, '+'}, {0xcb, '+'},
    {0xcc, '|'}, {0xcd, '='}, {0xce, '+'}, {0xcf, '+'},
    {0xd0, '+'}, {0xd1, '+'}, {0xd2, '+'}, {0xd3, '+'},
    {0xd4, '+'}, {0xd5, '+'}, {0xd6, '+'}, {0xd7, '+'},
    {0xd8, '+'}, {0xd9, '+'}, {0xda, '+'}, {0xdb, '#'},
    {0xdc, '#'}, {0xdd, '#'}, {0xde, '#'}, {0xdf, '#'},
    {0xe0, 'a'}, {0xe1, 'b'}, {0xe2, 'r'}, {0xe3, '*'},
    {0xe4, 'S'}, {0xe5, 's'}, {0xe6, 'u'}, {0xe7, 'T'},
    {0xe8, '*'}, {0xe9, '*'}, {0xea, '*'}, {0xeb, 'd'},
    {0xec, '*'}, {0xed, '*'}, {0xee, '*'}, {0xef, '*'},
    {0xf0, '='}, {0xf1, '*'}, {0xf2, '*'}, {0xf3, '*'},
    {0xf4, '*'}, {0xf5, '*'}, {0xf6, '*'}, {0xf7, '~'},
    {0xf8, '.'}, {0xf9, '.'}, {0xfa, '.'}, {0xfb, '*'},
    {0xfc, 'n'}, {0xfd, '2'}, {0xfe, '#'}, {0xff, ' '}
};

static void lnxcdispPaint(uint8_t force) {
    int ref;
    uint8_t p, c;
    int i, j, sizeRow, sizeCol, curX, curY;
    sizeRow = GetMin(COLS, deviceConnectDisplayGetRowSize());
    sizeCol = GetMin(LINES, deviceConnectDisplayGetColSize());
    ref = 0;
    if (force || deviceConnectDisplayGetBufferChange()) {
        clear();
        for (i = 0; i < sizeCol; ++i) {
            for (j = 0; j < sizeRow; ++j) {
                c = deviceConnectDisplayGetCurrentChar(i, j);
                p = deviceConnectDisplayGetCurrentCharProp(i, j) & 0x7f;
                c = Ascii2Print[c][1]; /* curses cannot print ext ascii */
                move(i, j);
                addch(c | COLOR_PAIR(GetColorFromProp(p)));
            }
        }
        ref = 1;
    }
    if (force || deviceConnectDisplayGetCursorChange()) {
        curX = deviceConnectDisplayGetCurrentCursorPosX();
        curY = deviceConnectDisplayGetCurrentCursorPosY();
        if (curX < sizeCol && curY < sizeRow) {
            move(deviceConnectDisplayGetCurrentCursorPosX(),
                 deviceConnectDisplayGetCurrentCursorPosY());
        } else {
            move(0, 0);
        }
        ref = 1;
    }
    if (ref) {
        refresh();
    }
}

static void *ThreadDisplay(void *arg) {
    lnxcdispInit();
    lnxcdispPaint(1);
    while (device.flagRun) {
        lnxcdispPaint(0);
        utilsSleep(100);
    }
    lnxcdispFinal();
    return 0;
}

static void *ThreadKernel(void *arg) {
    deviceStart();
    return 0;
}

static uint8_t Ascii2ScanCode[][2] = {
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

#define send(n) deviceConnectKeyboardRecvKeyPress(n)
static void lnxckeybMakeKey(int keyvalue) {
    if (keyvalue == KEY_F(9)) {
        deviceStop();
    }
    if (keyvalue < 0x001b) {
        switch (keyvalue) {
        case 0x000a:
            /* ENTER */
            send(0x1c0d);
            break;
        default:
            /* CTRL + LETTER */
            send(0x1d00);
            send((Ascii2ScanCode[keyvalue + 0x60][1] << 8) | keyvalue);
            break;
        }
    } else if (keyvalue < 0x0020) {
        switch (keyvalue) {
        case 0x001b:
            keyvalue = getch();
            if (keyvalue == ERR) {
                /* ESCAPE*/
                send(0x011b);
            } else {
                /* ALT */
                send(0x3800);
                send(Ascii2ScanCode[keyvalue][1] << 8);
            }
            break;
        default:
            /* CTRL + NUMBER */
            break;
        }
    } else if (keyvalue < 0x0100) {
        switch (keyvalue) {
        default:
            send((Ascii2ScanCode[keyvalue][1] << 8) | keyvalue);
            break;
        }
    } else if (keyvalue > KEY_F0 && keyvalue <= KEY_F(12)) {
        switch (keyvalue) {
        default:
            send((keyvalue - KEY_F0 + 0x3a) << 8);
            break;
        }
    } else {
        /* get special keys */
        switch (keyvalue) {
        case KEY_DOWN:
            send(0x5000);
            break;
        case KEY_UP:
            send(0x4800);
            break;
        case KEY_LEFT:
            send(0x4b00);
            break;
        case KEY_RIGHT:
            send(0x4d00);
            break;
        case KEY_HOME:
            send(0x4700);
            break;
        case KEY_BACKSPACE:
            send(0x0e08);
            break;
        case KEY_ENTER:
            send(0x1c0d);
            break;
        case KEY_NPAGE:
            send(0x5100);
            break;
        case KEY_PPAGE:
            send(0x4900);
            break;
        case KEY_END:
            send(0x4f00);
            break;
        case 0x014a:
            /* DELETE */
            send(0x5300);
            break;
        case 0x014b:
            /* INSERT */
            send(0x5200);
            break;
        default:
            return;
        }
    }
}

static void lnxckeybProcess() {
    int keyvalue = getch();
    if (keyvalue != ERR) {
        lnxckeybMakeKey(keyvalue);
    }
}

void lnxcDisplaySetScreen() {}

void lnxcDisplayPaint() {
    lnxcdispPaint(1);
}

void lnxcStartMachine() {
    pthread_t ThreadIdDisplay;
    pthread_t ThreadIdKernel;
    pthread_attr_t attr;
    int oldDeviceFlip = device.flagFlip;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    pthread_create(&ThreadIdKernel,  &attr, ThreadKernel, NULL);
    while (oldDeviceFlip == device.flagFlip) {
        utilsSleep(100);
    }
    pthread_create(&ThreadIdDisplay, &attr, ThreadDisplay, NULL);
    while (device.flagRun) {
        utilsSleep(20);
        lnxckeybProcess();
    }
    pthread_attr_destroy(&attr);
}
