/* Copyright 2012-2014 Neko. */

/* Global Settings */

#ifndef NXVM_GLOBAL_H
#define NXVM_GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif

#define GLOBAL_VAR_WIN32 0
#define GLOBAL_VAR_LINUX 1

/* COMPILING OPTIONS ******************************************************* */
#if 1
#define GLOBAL_SIZE_INTEGER 64
#else
#define GLOBAL_SIZE_INTEGER 32
#endif
#ifdef _WIN32
#define GLOBAL_PLATFORM GLOBAL_VAR_WIN32
#else
#define GLOBAL_PLATFORM GLOBAL_VAR_LINUX
#endif
/* ************************************************************************* */

/* TESTING OPTIONS ********************************************************* */
#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32
//#define GLOBAL_BOCHS
#endif
/* ************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
