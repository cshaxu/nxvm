#define PRODUCT "Neko's Virtual DOS Machine [0.0.0003]\n\
Copyright (c) 2013-2014 Neko.\n"

#define USAGE "INFO: command-line usage\n\
nkvdm [<dos-executable> <arg1> <arg2> ...]"

/*
 * main.c: version statement
 * exec.ch: main entry to execute program
 * msdos.ch: dos kernel
 */

#include "../../src/utils.h"
#include "exec.h"

int main(int argc, char **argv) {
    PRINTF("%s\n", PRODUCT);
    PRINTF("Built on %s at %s.\n", __DATE__, __TIME__);
#if 0
    if (argc < 2) {
        PRINTF("%s\n", USAGE);
        return 1;
    }
#endif
    return exec(argc - 1, argv + 1);
}
