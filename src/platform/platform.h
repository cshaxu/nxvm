/* Copyright 2012-2014 Neko. */

#ifndef NXVM_PLATFORM_H
#define NXVM_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char flagMode;  /* true if runs in window, otherwise in console */
} t_platform;

extern t_platform platform;

/* Device Operations */
void platformDisplaySetScreen();
void platformDisplayPaint();
void platformSleep(unsigned int milisec);

void platformStart();

void platformInit();
void platformFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
