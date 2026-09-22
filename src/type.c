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

C_INT STD_VSNPRINTF(C_CHAR *_Dest, STD_SIZE_T _Size, const C_CHAR *_Format,
    STD_VA_LIST _Arguments) {
    C_INT nWrittenBytes;
    if (_Format == STD_NULL || (_Size != 0u && _Dest == STD_NULL)) {
        if (_Dest != STD_NULL && _Size != 0u) _Dest[0] = '\0';
        return -1;
    }
    nWrittenBytes = vsnprintf(_Dest, _Size, _Format, _Arguments);
    if (_Size != 0u) {
        _Dest[_Size - 1u] = '\0';
        if (nWrittenBytes < 0) _Dest[0] = '\0';
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
