/* Copyright 2012-2014 Neko. */

#include "type.h"

struct tm* LOCALTIME(const time_t *_Time) { return localtime(_Time); }
char* STRCAT(char *_Dest, const char *_Source) { return strcat(_Dest, _Source); }
char* STRCPY(char *_Dest, const char *_Source) { return strcpy(_Dest, _Source); }
char* STRTOK(char *_Str, const char *_Delim) { return strtok(_Str, _Delim); }
int STRCMP(const char *_Str1, const char *_Str2) { return strcmp(_Str1, _Str2); }
size_t STRLEN(const char *_Str) { return strlen(_Str); }

int PRINTF(const char *_Format, ...) {
    int nWrittenBytes;
    va_list arg_ptr;
    va_start(arg_ptr, _Format);
    nWrittenBytes = vfprintf(stdout, _Format, arg_ptr);
    va_end(arg_ptr);
    fflush(stdout);
    return nWrittenBytes;
}
int FPRINTF(FILE *_File, const char *_Format, ...) {
    int nWrittenBytes;
    va_list arg_ptr;
    va_start(arg_ptr, _Format);
    nWrittenBytes = vfprintf(_File, _Format, arg_ptr);
    va_end(arg_ptr);
    return nWrittenBytes;
}
int SPRINTF(char *_Dest, const char *_Format, ...) {
    int nWrittenBytes;
    va_list arg_ptr;
    va_start(arg_ptr, _Format);
    nWrittenBytes = vsprintf(_Dest, _Format, arg_ptr);
    va_end(arg_ptr);
    return nWrittenBytes;
}

FILE* FOPEN(const char *_Filename, const char *_Mode) { return fopen(_Filename, _Mode); }
int FCLOSE(FILE *_File) { return fclose(_File); }
size_t FREAD(void *_DstBuf, size_t _ElementSize, size_t _Count, FILE *_File) { return fread(_DstBuf, _ElementSize, _Count, _File); }
size_t FWRITE(void *_Str, size_t _Size, size_t _Count, FILE *_File) { return fwrite(_Str, _Size, _Count, _File); }
char* FGETS(char *_Buf, int _MaxCount, FILE *_File) { return fgets(_Buf, _MaxCount, _File); }
void* MALLOC(size_t _Size) { return malloc(_Size); }
void FREE(void *_Memory) { free(_Memory); }
void* MEMSET(void *_Dst, int _Val, size_t _Size) { return memset(_Dst, _Val, _Size); }
void* MEMCPY(void *_Dst, const void *_Src, size_t _Size) { return memcpy(_Dst, _Src, _Size); }
int MEMCMP(const void *_Buf1, const void *_Buf2, size_t _Size) { return memcmp(_Buf1, _Buf2, _Size); }

void utilsLowerStr(char *str) {
    size_t i = 0;
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
