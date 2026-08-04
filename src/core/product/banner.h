#ifndef NTVDM64_CORE_PRODUCT_BANNER_H
#define NTVDM64_CORE_PRODUCT_BANNER_H

#include "type.h"

#ifndef NXVM_BUILD_SUFFIX
#define NXVM_BUILD_SUFFIX "0.5.0000"
#endif

#define PRODUCT_VERSION NXVM_BUILD_SUFFIX
#define PRODUCT_COPYRIGHT "Copyright (c) 2012-2026 Neko."
#define PRODUCT_BUILD_TIME __DATE__ " " __TIME__

#define CORE_PRODUCT_PRINT_BANNER()                          \
    STD_PRINTF("%s [%s]\n%s\n\nBuilt on %s\n", PRODUCT_NAME, \
               PRODUCT_VERSION, PRODUCT_COPYRIGHT, PRODUCT_BUILD_TIME)

#endif
