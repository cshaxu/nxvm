#ifndef NXVM_BUILD_SUFFIX
#define NXVM_BUILD_SUFFIX "m1t2s1"
#endif

#define PRODUCT "Neko's x86 Virtual Machine [0.4.015d." NXVM_BUILD_SUFFIX "]\n\
Copyright (c) 2012-2014 Neko.\n"

/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */

#include "core/product/utils.h"
#include "vm/product/console.h"

int main(int argc, char **argv) {
    PRINTF("%s\n", PRODUCT);
    PRINTF("Built on %s at %s.\n", __DATE__, __TIME__);
    consoleMain();
    return 0;
}
