/* Copyright 2012-2014 Neko. */

/* PLATFORM is the hub that connects all platform-specific modules */

#include "core/product/utils.h"

#include "core/platform/display_frame.h"
#include "vm/platform/platform.h"

t_platform platform;

#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32
#include "vm/platform/win32/win32.h"
void platformDisplaySetScreen() {
    win32DisplaySetScreen(platform.flagMode);
}
void platformDisplayPaint() {
    win32DisplayPaint(platform.flagMode);
}
void platformStart() {
    win32StartMachine(platform.flagMode);
}
#elif GLOBAL_PLATFORM == GLOBAL_VAR_LINUX
#include "vm/platform/linux/linux.h"
void platformDisplaySetScreen() {
    linuxDisplaySetScreen(platform.flagMode);
}
void platformDisplayPaint() {
    linuxDisplayPaint(platform.flagMode);
}
void platformStart() {
    linuxStartMachine(platform.flagMode);
}
#endif

void platformInit() {
    MEMSET((void *)(&platform), 0x00, sizeof(t_platform));
    platform.flagMode = 0;
    core_platform_display_initialize();
}

void platformFinal() {}
