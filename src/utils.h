/* Copyright 2012-2014 Neko. */

#ifndef NXVM_UTILS_H
#define NXVM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"

/* DEBUGGING OPTIONS ******************************* */
#define UTILS_TRACE_ENABLED 0 /* enable trancer */
#define UTILS_TRACE_DEBUG   0 /* debug tracer itself */
/* ************************************************* */

/* Code Tracer */
#define UTILS_TRACE_MAX_STACK 0x100
typedef struct {
	char* blockStack[UTILS_TRACE_MAX_STACK];
	int blockCount;
	char *callName;
} t_utils_trace_call;
typedef struct {
	t_utils_trace_call callStack[UTILS_TRACE_MAX_STACK];
	int callCount;
	int flagError;
} t_utils_trace;

void utilsTracePrintCall(t_utils_trace_call *rtracecall);
void utilsTracePrintTrace(t_utils_trace *rtrace);
void utilsTraceInit(t_utils_trace *rtrace);
void utilsTraceFinal(t_utils_trace *rtrace);
void utilsTraceCallBegin(t_utils_trace *rtrace, char *callName);
void utilsTraceCallEnd(t_utils_trace *rtrace);
void utilsTraceBlockBegin(t_utils_trace *rtrace, char *blockName);
void utilsTraceBlockEnd(t_utils_trace *rtrace);

/* #define UTILS_TRACE_VAR    tracer variable */
/* #define UTILS_TRACE_ERROR  error condition */
/* #define UTILS_TRACE_SETERR set error statement */

#if UTILS_TRACE_ENABLED == 1
#define _cb(callName)  utilsTraceCallBegin(&(UTILS_TRACE_VAR), (callName))
#define _bb(blockName) utilsTraceBlockBegin(&(UTILS_TRACE_VAR), (blockName))
#define _ce utilsTraceCallEnd(&(UTILS_TRACE_VAR))
#define _be utilsTraceBlockEnd(&(UTILS_TRACE_VAR))
#define _chb(n) \
	if (1) { \
		(n); \
		if (UTILS_TRACE_ERROR) { \
			(UTILS_TRACE_VAR).flagError = 1; \
			utilsTraceFinal(&(UTILS_TRACE_VAR)); \
			break; \
		} \
	} else
#define _chr(n) \
	do { \
		(n); \
		if (UTILS_TRACE_ERROR) { \
			(UTILS_TRACE_VAR).flagError = 1; \
			utilsTraceFinal(&(UTILS_TRACE_VAR)); \
			return; \
		} \
	} while (0)
#define _chrz(n) \
	do { \
		(n); \
		if (UTILS_TRACE_ERROR) { \
			(UTILS_TRACE_VAR).flagError = 1; \
			utilsTraceFinal(&(UTILS_TRACE_VAR)); \
			return 0; \
		} \
	} while (0)
#else
#define _cb(callName)
#define _bb(blockName)
#define _ce
#define _be
#define _chb(n) \
	if (1) { \
		(n); \
		if (UTILS_TRACE_ERROR) { \
			break; \
		} \
	} else
#define _chr(n) \
	do { \
		(n); \
		if (UTILS_TRACE_ERROR) { \
			return; \
		} \
	} while (0)
#define _chrz(n) \
	do { \
		(n); \
		if (UTILS_TRACE_ERROR) { \
			return 0; \
		} \
	} while (0)
#endif

#define _impossible_b_  _chb(UTILS_TRACE_SETERR);
#define _impossible_r_  _chr(UTILS_TRACE_SETERR);
#define _impossible_rz_ _chrz(UTILS_TRACE_SETERR);

/* Standard C Library */
struct tm* LOCALTIME(const time_t *_Time);

char* STRCAT(char *_Dest, const char *_Source);
char* STRCPY(char *_Dest, const char *_Source);
char* STRTOK(char *_Str, const char *_Delim);
int   STRCMP(const char *_Str1, const char *_Str2);

int   PRINTF(const char *_Format, ...);
int   FPRINTF(FILE *_File, const char *_Format, ...);
int   SPRINTF(char *_Dest, const char *_Format, ...);

FILE*  FOPEN(const char *_Filename, const char *_Mode);
int    FCLOSE(FILE *_File);
size_t FREAD(void *_DstBuf, size_t _ElementSize, size_t _Count, FILE *_File);
size_t FWRITE(void *_Str, size_t _Size, size_t _Count, FILE *_File);
char*  FGETS(char *_Buf, int _MaxCount, FILE *_File);

void* MALLOC(size_t _Size);
void  FREE(void *_Memory);
void* MEMSET(void *_Dst, int _Val, size_t _Size);
void* MEMCPY(void *_Dst, const void *_Src, size_t _Size);
int   MEMCMP(const void *_Buf1, const void *_Buf2, size_t _Size);

/* NXVM Library */
void utilsSleep(unsigned int milisec);
void utilsLowerStr(char *str);

/* NXVM Assembler Library */
unsigned char utilsAasm32(const char *stmt, unsigned char *rcode, unsigned char flag32);
unsigned int utilsAasm32x(const char *stmt, unsigned char *rcode, unsigned char flag32);
unsigned char utilsDasm32(char *stmt, unsigned char *rcode, unsigned char flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
