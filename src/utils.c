/* Copyright 2012-2014 Neko. */

/* UTILS implements code tracer and all utility functions. */

#include "platform/platform.h"
#include "utils.h"

void utilsTracePrintCall(t_utils_trace_call *rtrace, int cid) {
	int i;
	for (i = 0;i < rtrace->callstack[cid].bid;++i) {
		utilsPrint("%s", rtrace->callstack[cid].blockstack[i]);
		if (i != rtrace->callstack[cid].bid - 1) {
			utilsPrint("::");
		}
	}
	utilsPrint("\n");
}
void utilsTracePrintCallStack(t_utils_trace_call *rtrace) {
	int i;
	if (rtrace->cid) {
		for (i = rtrace->cid - 1;i > 0;--i) {
			utilsTracePrintCall(rtrace, i);
		}
		utilsTracePrintCall(rtrace, 0);
	}
}
void utilsTraceInit(t_utils_trace_call *rtrace) {
	rtrace->cid = 0x00;
	rtrace->flagerror = 0;
}
void utilsTraceFinal(t_utils_trace_call *rtrace) {
	if (!rtrace->flagerror && rtrace->cid) {
		utilsPrint("trace_final: call stack is not balanced. (stack: %d, call: %d, block: %d)\n",
			rtrace->cid, rtrace->callstack[rtrace->cid].cid, rtrace->callstack[rtrace->cid].bid);
		rtrace->flagerror = 1;
	}
	if (rtrace->flagerror) {
		utilsTracePrintCallStack(rtrace);
	}
	rtrace->cid = 0x00;
	rtrace->flagerror = 0;
}
void utilsTraceCallBegin(t_utils_trace_call *rtrace, char *s) {
	if (rtrace->flagerror) {
		return;
	}
	if (rtrace->cid < 0xff) {
	#if UTILS_TRACE_DEBUG == 1
		utilsPrint("enter call(%d): %s\n", rtrace->cid, s);
	#endif
		rtrace->callstack[rtrace->cid].blockstack[0] = s;
		rtrace->callstack[rtrace->cid].bid = 1;
		rtrace->callstack[rtrace->cid].cid = rtrace->cid;
		rtrace->cid++;
	} else {
		utilsPrint("trace_call_begin: call stack is full.\n");
		rtrace->flagerror = 1;
	}
}
void utilsTraceCallEnd(t_utils_trace_call *rtrace) {
	if (rtrace->flagerror) {
		return;
	}
	if (rtrace->cid) {
		rtrace->cid--;
	#if UTILS_TRACE_DEBUG == 1
		utilsPrint("leave call(%d): %s\n", rtrace->cid,
			rtrace->callstack[rtrace->cid].blockstack[0]);
	#endif
		if (rtrace->callstack[rtrace->cid].bid != 1 ||
			rtrace->callstack[rtrace->cid].cid != rtrace->cid) {
			utilsPrint("trace_call_end: call stack is not balanced. (stack: %d, call: %d, block: %d)\n",
				rtrace->cid, rtrace->callstack[rtrace->cid].cid, rtrace->callstack[rtrace->cid].bid);
			rtrace->cid++;
			rtrace->flagerror = 1;
		}
	} else {
		utilsPrint("trace_call_end: call stack is empty.\n");
		rtrace->flagerror = 1;
	}
}
void utilsTraceBlockBegin(t_utils_trace_call *rtrace, char *s) {
	if (rtrace->flagerror) {
		return;
	}
	if (rtrace->callstack[rtrace->cid - 1].bid < 0xff) {
#if UTILS_TRACE_DEBUG == 1
		utilsPrint("enter block(%d): %s\n", rtrace->callstack[rtrace->cid - 1].bid, s);
#endif
		rtrace->callstack[rtrace->cid - 1].blockstack[rtrace->callstack[rtrace->cid - 1].bid++] = s;
	} else {
		utilsPrint("trace_block_begin: block stack is full.\n");
		rtrace->flagerror = 1;
	}
}
void utilsTraceBlockEnd(t_utils_trace_call *rtrace) {
	if (rtrace->flagerror) {
		return;
	}
	if (rtrace->callstack[rtrace->cid - 1].bid) {
		rtrace->callstack[rtrace->cid - 1].bid--;
#if UTILS_TRACE_DEBUG == 1
		utilsPrint("leave block(%d): %s\n",
			rtrace->callstack[rtrace->cid - 1].bid,
			rtrace->callstack[rtrace->cid - 1].blockstack[rtrace->callstack[rtrace->cid - 1].bid]);
#endif
	} else {
		utilsPrint("trace_block_end: block stack is empty.\n");
		rtrace->flagerror = 1;
	}
}

/* Standard C Library */
struct tm* LOCALTIME(const time_t *_Time) {return localtime(_Time);}
char* STRCAT(char *_Dest, const char *_Source) {return strcat(_Dest, _Source);}
char* STRCPY(char *_Dest, const char *_Source) {return strcpy(_Dest, _Source);}
char* STRTOK(char *_Str, const char *_Delim) {return strtok(_Str, _Delim);}
int STRCMP(const char *_Str1, const char *_Str2) {return strcmp(_Str1, _Str2);}
int SPRINTF(char *_Dest, const char *_Format, ...) {
	int nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, _Format);
	nWrittenBytes = vsprintf(_Dest, _Format, arg_ptr);
	va_end(arg_ptr);
	return nWrittenBytes;
}
FILE* FOPEN(const char *_Filename, const char *_Mode) {return fopen(_Filename, _Mode);}
char* FGETS(char *_Buf, int _MaxCount, FILE *_File) {return fgets(_Buf, _MaxCount, _File);}

/* General Functions */
void utilsSleep(unsigned int milisec) {platformSleep(milisec);}
void utilsLowerStr(char *s) {
	int i = 0;
	if (s[0] == '\'') {
		return;
	}
	while(s[i] != '\0') {
		if (s[i] == '\n') {
			s[i] = '\0';
		} else if (s[i] > 0x40 && s[i] < 0x5b) {
			s[i] += 0x20;
		}
		i++;
	}
}
int utilsPrint(const char *format, ...) {
	int nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	nWrittenBytes = vfprintf(stdout, format,arg_ptr);
	//nWrittenBytes = vsprintf(stringBuffer,format,arg_ptr);
	va_end(arg_ptr);
	fflush(stdout);
	return nWrittenBytes;
}
