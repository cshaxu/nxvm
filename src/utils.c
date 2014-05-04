/* Copyright 2012-2014 Neko. */

/* UTILS implements code tracer and all utility functions. */

#include "xasm32/aasm32.h"
#include "xasm32/dasm32.h"
#include "platform/platform.h"

#include "utils.h"

void utilsTracePrintCall(t_utils_trace_call *rtracecall) {
	int i;
	PRINTF("%s", rtracecall->callName);
	for (i = 0;i < rtracecall->blockCount;++i) {
		PRINTF("::%s", rtracecall->blockStack[i]);
	}
	PRINTF("\n");
}
void utilsTracePrintTrace(t_utils_trace *rtrace) {
	int i;
	if (rtrace->callCount) {
		for (i = rtrace->callCount - 1;i >= 0;--i) {
			utilsTracePrintCall(&(rtrace->callStack[i]));
		}
	}
}
void utilsTraceInit(t_utils_trace *rtrace) {
	rtrace->callCount = 0;
	rtrace->flagError = 0;
}
void utilsTraceFinal(t_utils_trace *rtrace) {
	if (!rtrace->flagError && rtrace->callCount) {
		PRINTF("trace_final: call stack is not balanced. (call: %d, block: %d)\n",
			rtrace->callCount, rtrace->callStack[rtrace->callCount].blockCount);
		rtrace->flagError = 1;
	}
	if (rtrace->flagError) {
		utilsTracePrintTrace(rtrace);
	}
	rtrace->callCount = 0;
	rtrace->flagError = 0;
}
void utilsTraceCallBegin(t_utils_trace *rtrace, char *callName) {
	if (rtrace->flagError) {
		return;
	}
	if (rtrace->callCount < UTILS_TRACE_MAX_STACK) {
	#if UTILS_TRACE_DEBUG == 1
		PRINTF("enter call(%d): %s\n", rtrace->callCount, callName);
	#endif
		rtrace->callStack[rtrace->callCount].callName = callName;
		rtrace->callStack[rtrace->callCount].blockCount = 0;
		rtrace->callCount++;
	} else {
		PRINTF("trace_call_begin: call stack is full.\n");
		rtrace->flagError = 1;
	}
}
void utilsTraceCallEnd(t_utils_trace *rtrace) {
	if (rtrace->flagError) {
		return;
	}
	if (rtrace->callCount) {
		rtrace->callCount--;
	#if UTILS_TRACE_DEBUG == 1
		PRINTF("leave call(%d): %s\n", rtrace->callCount,
			rtrace->callStack[rtrace->callCount].callName);
	#endif
		if (rtrace->callStack[rtrace->callCount].blockCount != 0) {
			PRINTF("trace_call_end: call stack is not balanced. (call: %d, block: %d)\n",
				rtrace->callCount, rtrace->callStack[rtrace->callCount].blockCount);
			rtrace->callCount++;
			rtrace->flagError = 1;
		}
	} else {
		PRINTF("trace_call_end: call stack is empty.\n");
		rtrace->flagError = 1;
	}
}
void utilsTraceBlockBegin(t_utils_trace *rtrace, char *blockName) {
	if (rtrace->flagError) {
		return;
	}
	if (rtrace->callStack[rtrace->callCount - 1].blockCount < UTILS_TRACE_MAX_STACK) {
#if UTILS_TRACE_DEBUG == 1
		PRINTF("enter block(%d): %s\n", rtrace->callStack[rtrace->callCount - 1].blockCount, blockName);
#endif
		rtrace->callStack[rtrace->callCount - 1].
			blockStack[rtrace->callStack[rtrace->callCount - 1].blockCount++] = blockName;
	} else {
		PRINTF("trace_block_begin: block stack is full.\n");
		rtrace->flagError = 1;
	}
}
void utilsTraceBlockEnd(t_utils_trace *rtrace) {
	if (rtrace->flagError) {
		return;
	}
	if (rtrace->callStack[rtrace->callCount - 1].blockCount) {
		rtrace->callStack[rtrace->callCount - 1].blockCount--;
#if UTILS_TRACE_DEBUG == 1
		PRINTF("leave block(%d): %s\n",
			rtrace->callStack[rtrace->callCount - 1].blockCount,
			rtrace->callStack[rtrace->callCount - 1].blockStack[rtrace->callStack[rtrace->callCount - 1].blockCount]);
#endif
	} else {
		PRINTF("trace_block_end: block stack is empty.\n");
		rtrace->flagError = 1;
	}
}

/* Standard C Library */
struct tm* LOCALTIME(const time_t *_Time) {return localtime(_Time);}

char* STRCAT(char *_Dest, const char *_Source) {return strcat(_Dest, _Source);}
char* STRCPY(char *_Dest, const char *_Source) {return strcpy(_Dest, _Source);}
char* STRTOK(char *_Str, const char *_Delim) {return strtok(_Str, _Delim);}
int   STRCMP(const char *_Str1, const char *_Str2) {return strcmp(_Str1, _Str2);}

int PRINTF(const char *_Format, ...) {
	int nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, _Format);
	nWrittenBytes = vfprintf(stdout, _Format, arg_ptr);
	va_end(arg_ptr);
	fflush(stdout);
	return nWrittenBytes;
}
int FPRINTF(FILE *_File, const char *_Format, ...) {
	int nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, _Format);
	nWrittenBytes = vfprintf(_File, _Format, arg_ptr);
	va_end(arg_ptr);
	return nWrittenBytes;
}
int SPRINTF(char *_Dest, const char *_Format, ...) {
	int nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, _Format);
	nWrittenBytes = vsprintf(_Dest, _Format, arg_ptr);
	va_end(arg_ptr);
	return nWrittenBytes;
}

FILE*  FOPEN(const char *_Filename, const char *_Mode) {return fopen(_Filename, _Mode);}
int    FCLOSE(FILE *_File) {return fclose(_File);}
size_t FREAD(void *_DstBuf, size_t _ElementSize, size_t _Count, FILE *_File) {return fread(_DstBuf, _ElementSize, _Count, _File);}
size_t FWRITE(void *_Str, size_t _Size, size_t _Count, FILE *_File) {return fwrite(_Str, _Size, _Count, _File);}
char*  FGETS(char *_Buf, int _MaxCount, FILE *_File) {return fgets(_Buf, _MaxCount, _File);}

void* MALLOC(size_t _Size) {return malloc(_Size);}
void  FREE(void *_Memory) {free(_Memory);}
void* MEMSET(void *_Dst, int _Val, size_t _Size) {return memset(_Dst, _Val, _Size);}
void* MEMCPY(void *_Dst, const void *_Src, size_t _Size) {return memcpy(_Dst, _Src, _Size);}
int   MEMCMP(const void *_Buf1, const void *_Buf2, size_t _Size) {return memcmp(_Buf1, _Buf2, _Size);}

/* General Functions */
void utilsSleep(unsigned int milisec) {platformSleep(milisec);}
void utilsLowerStr(char *str) {
	int i = 0;
	if (str[0] == '\'') {
		return;
	}
	while(str[i] != '\0') {
		if (str[i] == '\n') {
			str[i] = '\0';
		} else if (str[i] > 0x40 && str[i] < 0x5b) {
			str[i] += 0x20;
		}
		i++;
	}
}

/* NXVM Assembler Library */
unsigned char utilsAasm32(const char *stmt, unsigned char *rcode, unsigned char flag32) {
	return aasm32(stmt, rcode, flag32);
}
unsigned int utilsAasm32x(const char *stmt, unsigned char *rcode, unsigned char flag32) {
	return aasm32x(stmt, rcode, flag32);
}
unsigned char utilsDasm32(char *stmt, unsigned char *rcode, unsigned char flag32) {
	return dasm32(stmt, rcode, flag32);
}
