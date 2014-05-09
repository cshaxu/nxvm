/* This file is a part of NekoConsolePaint project. */

#include "nekomacro.h"
#include "windows.h"
#include "stdio.h"
#include "conio.h"

/*void cls(HANDLE hOut)
{
	DWORD tBuf;
	LPDWORD lpBuf = (LPDWORD)(&tBuf);
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	COORD pos = {0,0};
	WORD att;
	unsigned int size;
	GetConsoleScreenBufferInfo(hOut,&bInfo);
	att = bInfo.wAttributes;
	size = bInfo.dwSize.X * bInfo.dwSize.Y;
	FillConsoleOutputAttribute(hOut,att,size,pos,lpBuf);
	FillConsoleOutputCharacter(hOut,' ',size,pos,lpBuf);
}*/

int main(int argc, char **argv) {
    HANDLE hIn,hOut;
    //WORD att;
    INPUT_RECORD inRec;
    DWORD state = 0,res;
    char ch;
    //SMALL_RECT rc = {10,0,30,20};
    COORD pos = {0,0};
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hIn = GetStdHandle(STD_INPUT_HANDLE);
    /*att = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
    SetConsoleTextAttribute(hOut,att);
    cls(hOut);*/
    while (1) {
        ReadConsoleInput(hIn,&inRec,1,(LPDWORD)(&res));
        SetConsoleCursorPosition(hOut,pos);
        /*if(inRec.Event.KeyEvent.dwControlKeyState != state) {
        	state = inRec.Event.KeyEvent.dwControlKeyState;
        	ShowControlStatus(state);
        }*/
        if (inRec.EventType == MOUSE_EVENT) {
            if (inRec.Event.MouseEvent.dwEventFlags == DOUBLE_CLICK) break;
            pos = inRec.Event.MouseEvent.dwMousePosition;
            if (inRec.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
                FillConsoleOutputCharacter(hOut, 'A', 1, pos, &res);
            else if (inRec.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
                FillConsoleOutputCharacter(hOut, ' ', 1, pos, &res);
        }
    }
    CloseHandle(hOut);
    CloseHandle(hIn);
    getchar();
    return 0;
}
