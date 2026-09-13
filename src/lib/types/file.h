#ifndef LIB_TYPES_FILE_H
#define LIB_TYPES_FILE_H

/* ISO C stream declarations; open modes and ownership belong to storage. */
#include <stdio.h>

typedef FILE lib_c_file;
#define lib_c_fopen fopen
#define lib_c_fclose fclose
#define lib_c_fread fread
#define lib_c_fwrite fwrite
#define lib_c_ferror ferror
#define lib_c_fflush fflush
#define LIB_SEEK_SET SEEK_SET
#define LIB_SEEK_END SEEK_END

#endif
