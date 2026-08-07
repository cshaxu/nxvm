/* Copyright 2012-2014 Neko. */

#include "type.h"

struct tm* STD_LOCALTIME(const STD_TIME_T *_Time) { return localtime(_Time); }
C_CHAR* STD_STRCAT(C_CHAR *_Dest, const C_CHAR *_Source) { return strcat(_Dest, _Source); }
C_CHAR* STD_STRCPY(C_CHAR *_Dest, const C_CHAR *_Source) { return strcpy(_Dest, _Source); }
C_CHAR* STD_STRTOK(C_CHAR *_Str, const C_CHAR *_Delim) { return strtok(_Str, _Delim); }
C_INT STD_STRCMP(const C_CHAR *_Str1, const C_CHAR *_Str2) { return strcmp(_Str1, _Str2); }
STD_SIZE_T STD_STRLEN(const C_CHAR *_Str) { return strlen(_Str); }

C_INT STD_PRINTF(const C_CHAR *_Format, ...) {
    C_INT nWrittenBytes;
    STD_VA_LIST arg_ptr;
    va_start(arg_ptr, _Format);
    nWrittenBytes = vfprintf(STD_STDOUT, _Format, arg_ptr);
    va_end(arg_ptr);
    fflush(STD_STDOUT);
    return nWrittenBytes;
}
C_INT STD_FPRINTF(STD_FILE *_File, const C_CHAR *_Format, ...) {
    C_INT nWrittenBytes;
    STD_VA_LIST arg_ptr;
    va_start(arg_ptr, _Format);
    nWrittenBytes = vfprintf(_File, _Format, arg_ptr);
    va_end(arg_ptr);
    return nWrittenBytes;
}
C_INT STD_SNPRINTF(C_CHAR *_Dest, STD_SIZE_T _Size, const C_CHAR *_Format, ...) {
    C_INT nWrittenBytes;
    STD_VA_LIST arg_ptr;
    if (_Format == STD_NULL || (_Size != 0u && _Dest == STD_NULL)) {
        if (_Dest != STD_NULL && _Size != 0u) {
            _Dest[0] = '\0';
        }
        return -1;
    }
    va_start(arg_ptr, _Format);
    nWrittenBytes = vsnprintf(_Dest, _Size, _Format, arg_ptr);
    va_end(arg_ptr);
    if (_Size != 0u) {
        _Dest[_Size - 1u] = '\0';
        if (nWrittenBytes < 0) {
            _Dest[0] = '\0';
        }
    }
    return nWrittenBytes;
}

C_INT STD_SNPRINTF_APPEND(C_CHAR **_Cursor, STD_SIZE_T *_Remaining,
    const C_CHAR *_Format, ...) {
    C_INT nWrittenBytes;
    STD_VA_LIST arg_ptr;
    if (_Cursor == STD_NULL || _Remaining == STD_NULL || _Format == STD_NULL ||
        (*_Remaining != 0u && *_Cursor == STD_NULL)) {
        if (_Cursor != STD_NULL && _Remaining != STD_NULL && *_Cursor != STD_NULL &&
            *_Remaining != 0u) {
            (*_Cursor)[0] = '\0';
        }
        return -1;
    }
    va_start(arg_ptr, _Format);
    nWrittenBytes = vsnprintf(*_Cursor, *_Remaining, _Format, arg_ptr);
    va_end(arg_ptr);
    if (*_Remaining != 0u) {
        (*_Cursor)[*_Remaining - 1u] = '\0';
        if (nWrittenBytes < 0) {
            (*_Cursor)[0] = '\0';
        }
    }
    if (nWrittenBytes < 0 || (STD_SIZE_T)nWrittenBytes >= *_Remaining) {
        return nWrittenBytes;
    }
    *_Cursor += nWrittenBytes;
    *_Remaining -= (STD_SIZE_T)nWrittenBytes;
    return nWrittenBytes;
}

STD_FILE* STD_FOPEN(const C_CHAR *_Filename, const C_CHAR *_Mode) { return fopen(_Filename, _Mode); }
C_INT STD_FCLOSE(STD_FILE *_File) { return fclose(_File); }
STD_SIZE_T STD_FREAD(C_VOID *_DstBuf, STD_SIZE_T _ElementSize, STD_SIZE_T _Count, STD_FILE *_File) { return fread(_DstBuf, _ElementSize, _Count, _File); }
STD_SIZE_T STD_FWRITE(const C_VOID *_Buffer, STD_SIZE_T _Size, STD_SIZE_T _Count, STD_FILE *_File) { return fwrite(_Buffer, _Size, _Count, _File); }
C_CHAR* STD_FGETS(C_CHAR *_Buf, C_INT _MaxCount, STD_FILE *_File) { return fgets(_Buf, _MaxCount, _File); }
C_INT STD_FSEEK(STD_FILE *_File, C_LONG _Offset, C_INT _Origin) { return fseek(_File, _Offset, _Origin); }
C_LONG STD_FTELL(STD_FILE *_File) { return ftell(_File); }
C_INT STD_FSEEK_64(STD_FILE *_File, int64_t _Offset, C_INT _Origin) {
#ifdef _WIN32
    return _fseeki64(_File, _Offset, _Origin);
#else
    return fseeko(_File, (off_t)_Offset, _Origin);
#endif
}
int64_t STD_FTELL_64(STD_FILE *_File) {
#ifdef _WIN32
    return _ftelli64(_File);
#else
    return (int64_t)ftello(_File);
#endif
}
C_INT STD_FGETC(STD_FILE *_File) { return fgetc(_File); }
C_INT STD_FPUTC(C_INT _Character, STD_FILE *_File) { return fputc(_Character, _File); }
C_INT STD_FPUTS(const C_CHAR *_String, STD_FILE *_File) { return fputs(_String, _File); }
C_INT STD_FEOF(STD_FILE *_File) { return feof(_File); }
C_INT STD_REMOVE(const C_CHAR *_Filename) { return remove(_Filename); }
C_INT STD_ATOI(const C_CHAR *_String) { return atoi(_String); }
STD_TIME_T STD_TIME(STD_TIME_T *_Time) { return time(_Time); }
C_VOID* STD_CALLOC(STD_SIZE_T _Count, STD_SIZE_T _Size) { return calloc(_Count, _Size); }
C_VOID* STD_MALLOC(STD_SIZE_T _Size) { return malloc(_Size); }
C_VOID STD_FREE(C_VOID *_Memory) { free(_Memory); }
C_VOID* STD_MEMSET(C_VOID *_Dst, C_INT _Val, STD_SIZE_T _Size) { return memset(_Dst, _Val, _Size); }
C_VOID* STD_MEMCPY(C_VOID *_Dst, const C_VOID *_Src, STD_SIZE_T _Size) { return memcpy(_Dst, _Src, _Size); }
C_VOID* STD_MEMMOVE(C_VOID *_Dst, const C_VOID *_Src, STD_SIZE_T _Size) { return memmove(_Dst, _Src, _Size); }
C_INT STD_MEMCMP(const C_VOID *_Buf1, const C_VOID *_Buf2, STD_SIZE_T _Size) { return memcmp(_Buf1, _Buf2, _Size); }

C_VOID type_string_lower(C_CHAR *str) {
    STD_SIZE_T i = 0;
    if (str[0] == '\'') {
        return;
    }
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            str[i] = '\0';
        } else if (str[i] > 0x40 && str[i] < 0x5b) {
            str[i] += 0x20;
        }
        i++;
    }
}

static C_VOID printTraceCall(type_trace_call *rtracecall) {
    STD_SIZE_T i;
    STD_PRINTF("%s", rtracecall->callName);
    for (i = 0; i < rtracecall->blockCount; ++i) {
        STD_PRINTF("::%s", rtracecall->blockStack[i]);
    }
    STD_PRINTF("\n");
}
C_VOID type_trace_print(type_trace *rtrace) {
    STD_SIZE_T i;
    if (rtrace->callCount) {
        for (i = 0; i < rtrace->callCount; ++i) {
            printTraceCall(&(rtrace->callStack[rtrace->callCount - 1 - i]));
        }
    }
}
C_VOID type_trace_initialize(type_trace *rtrace) {
    rtrace->callCount = 0;
    rtrace->flagError = 0;
}
C_VOID type_trace_finalize(type_trace *rtrace) {
    if (!rtrace->flagError && rtrace->callCount) {
        STD_PRINTF("trace_final: call stack is not balanced. (call: %d, block: %d)\n",
               rtrace->callCount, rtrace->callStack[rtrace->callCount].blockCount);
        rtrace->flagError = 1;
    }
    if (rtrace->flagError) {
        type_trace_print(rtrace);
    }
    rtrace->callCount = 0;
    rtrace->flagError = 0;
}
C_VOID type_trace_call_begin(type_trace *rtrace, C_CHAR *callName) {
    if (rtrace->flagError) return;
    if (rtrace->callCount < TYPE_TRACE_MAX_STACK) {
#if TYPE_TRACE_DEBUG == 1
        STD_PRINTF("enter call(%d): %s\n", rtrace->callCount, callName);
#endif
        rtrace->callStack[rtrace->callCount].callName = callName;
        rtrace->callStack[rtrace->callCount].blockCount = 0;
        rtrace->callCount++;
    } else {
        STD_PRINTF("trace_call_begin: call stack is full.\n");
        rtrace->flagError = 1;
    }
}
C_VOID type_trace_call_end(type_trace *rtrace) {
    if (rtrace->flagError) return;
    if (rtrace->callCount) {
        rtrace->callCount--;
#if TYPE_TRACE_DEBUG == 1
        STD_PRINTF("leave call(%d): %s\n", rtrace->callCount,
               rtrace->callStack[rtrace->callCount].callName);
#endif
        if (rtrace->callStack[rtrace->callCount].blockCount != 0) {
            STD_PRINTF("trace_call_end: call stack is not balanced. (call: %d, block: %d)\n",
                   rtrace->callCount, rtrace->callStack[rtrace->callCount].blockCount);
            rtrace->callCount++;
            rtrace->flagError = 1;
        }
    } else {
        STD_PRINTF("trace_call_end: call stack is empty.\n");
        rtrace->flagError = 1;
    }
}
C_VOID type_trace_block_begin(type_trace *rtrace, C_CHAR *blockName) {
    if (rtrace->flagError) return;
    if (rtrace->callStack[rtrace->callCount - 1].blockCount < TYPE_TRACE_MAX_STACK) {
#if TYPE_TRACE_DEBUG == 1
        STD_PRINTF("enter block(%d): %s\n", rtrace->callStack[rtrace->callCount - 1].blockCount, blockName);
#endif
        rtrace->callStack[rtrace->callCount - 1].
        blockStack[rtrace->callStack[rtrace->callCount - 1].blockCount++] = blockName;
    } else {
        STD_PRINTF("trace_block_begin: block stack is full.\n");
        rtrace->flagError = 1;
    }
}
C_VOID type_trace_block_end(type_trace *rtrace) {
    if (rtrace->flagError) return;
    if (rtrace->callStack[rtrace->callCount - 1].blockCount) {
        rtrace->callStack[rtrace->callCount - 1].blockCount--;
#if TYPE_TRACE_DEBUG == 1
        STD_PRINTF("leave block(%d): %s\n",
               rtrace->callStack[rtrace->callCount - 1].blockCount,
               rtrace->callStack[rtrace->callCount - 1].blockStack[rtrace->callStack[rtrace->callCount - 1].blockCount]);
#endif
    } else {
        STD_PRINTF("trace_block_end: block stack is empty.\n");
        rtrace->flagError = 1;
    }
}
