/* Copyright 2012-2014 Neko. */

#ifndef NXVM_CONSOLE_H
#define NXVM_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vm/product/console_target.h"

/* Entry point of NXVM console */
void consoleMain(const nxvm_product_console_target *target);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
