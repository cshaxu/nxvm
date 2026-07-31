#define PRODUCT "Neko's x86 Virtual Machine [0.4.015d]\n\
Copyright (c) 2012-2014 Neko.\n"

/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */

#include "utils.h"
#include "console.h"

int main(int argc, char **argv) {
    PRINTF("%s\n", PRODUCT);
    PRINTF("Built on %s at %s.\n", __DATE__, __TIME__);
    consoleMain();
    return 0;
}
