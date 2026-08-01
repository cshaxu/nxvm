/* Copyright 2012-2014 Neko. */

/* Shared project types and compile-time settings. */

#ifndef NTVDM64_TYPE_H
#define NTVDM64_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#define GLOBAL_VAR_WIN32 0
#define GLOBAL_VAR_LINUX 1

/* COMPILING OPTIONS ******************************* */
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
/* ************************************************* */

/* DEBUGGING OPTIONS ******************************* */
#define UTILS_TRACE_ENABLED 0 /* enable trancer */
#define UTILS_TRACE_DEBUG   0 /* debug tracer itself */
/* ************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <time.h>

/* Legacy C-runtime wrappers are a product-neutral root foundation. */
struct tm* LOCALTIME(const time_t *_Time);
char* STRCAT(char *_Dest, const char *_Source);
char* STRCPY(char *_Dest, const char *_Source);
char* STRTOK(char *_Str, const char *_Delim);
int STRCMP(const char *_Str1, const char *_Str2);
size_t STRLEN(const char *_Str);
int PRINTF(const char *_Format, ...);
int FPRINTF(FILE *_File, const char *_Format, ...);
int SPRINTF(char *_Dest, const char *_Format, ...);
FILE* FOPEN(const char *_Filename, const char *_Mode);
int FCLOSE(FILE *_File);
size_t FREAD(void *_DstBuf, size_t _ElementSize, size_t _Count, FILE *_File);
size_t FWRITE(void *_Str, size_t _Size, size_t _Count, FILE *_File);
char* FGETS(char *_Buf, int _MaxCount, FILE *_File);
void* MALLOC(size_t _Size);
void FREE(void *_Memory);
void* MEMSET(void *_Dst, int _Val, size_t _Size);
void* MEMCPY(void *_Dst, const void *_Src, size_t _Size);
int MEMCMP(const void *_Buf1, const void *_Buf2, size_t _Size);
void utilsLowerStr(char *str);

/* Legacy C-runtime wrappers are a product-neutral root foundation. */
struct tm* LOCALTIME(const time_t *_Time);
char* STRCAT(char *_Dest, const char *_Source);
char* STRCPY(char *_Dest, const char *_Source);
char* STRTOK(char *_Str, const char *_Delim);
int STRCMP(const char *_Str1, const char *_Str2);
size_t STRLEN(const char *_Str);
int PRINTF(const char *_Format, ...);
int FPRINTF(FILE *_File, const char *_Format, ...);
int SPRINTF(char *_Dest, const char *_Format, ...);
FILE* FOPEN(const char *_Filename, const char *_Mode);
int FCLOSE(FILE *_File);
size_t FREAD(void *_DstBuf, size_t _ElementSize, size_t _Count, FILE *_File);
size_t FWRITE(void *_Str, size_t _Size, size_t _Count, FILE *_File);
char* FGETS(char *_Buf, int _MaxCount, FILE *_File);
void* MALLOC(size_t _Size);
void FREE(void *_Memory);
void* MEMSET(void *_Dst, int _Val, size_t _Size);
void* MEMCPY(void *_Dst, const void *_Src, size_t _Size);
int MEMCMP(const void *_Buf1, const void *_Buf2, size_t _Size);
void utilsLowerStr(char *str);

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
