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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <time.h>

/* COMPATIBILITY DEFINITIONS *********************************************** */
#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32
typedef unsigned char    uint8_t;
typedef unsigned short   uint16_t;
typedef unsigned int     uint32_t;
typedef unsigned __int64 uint64_t;
typedef   signed char    int8_t;
typedef   signed short   int16_t;
typedef   signed int     int32_t;
typedef   signed __int64 int64_t;
#else
#include <stdint.h>
#endif
/* ************************************************************************* */

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
