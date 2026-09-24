#ifndef LIB_TYPES_FILE_H
#define LIB_TYPES_FILE_H

/* ISO C stream declarations; open modes and ownership belong to storage. */
#include <stdio.h>
#include <stdarg.h>

typedef FILE lib_c_file;
#define lib_c_fopen fopen
#define lib_c_fclose fclose
#define lib_c_fread fread
#define lib_c_fwrite fwrite
#define lib_c_ferror ferror
#define lib_c_fflush fflush
#define lib_c_fprintf fprintf
#define lib_c_printf printf
#define lib_c_remove remove
#define lib_c_snprintf snprintf
#define lib_c_vsnprintf vsnprintf
#define lib_c_stderr stderr
#define lib_c_stdout stdout
typedef va_list lib_c_va_list;
#define lib_c_va_start va_start
#define lib_c_va_end va_end
#define lib_c_va_copy va_copy
#define LIB_SEEK_SET SEEK_SET
#define LIB_SEEK_END SEEK_END
#define LIB_C_EOF EOF

#endif
