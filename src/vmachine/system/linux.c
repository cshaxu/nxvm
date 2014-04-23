/* This file is a part of NXVM project. */

#include "curses.h"
#include "pthread.h"

#include "../vapi.h"

#if VGLOBAL_PLATFORM == VGLOBAL_VAR_LINUX
#include "linux.h"

void linuxKeyboardMakeStatus() {}
void linuxKeyboardMakeKey(int keyvalue) {}

void linuxDisplaySetScreen() {}
void linuxDisplayPaint() {}

void linuxStartMachine() {}

#endif