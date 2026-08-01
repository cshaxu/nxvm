/* Copyright 2012-2014 Neko. */

/* UTILS implements code tracer and all utility functions. */

#include "core/product/debug/xasm32/aasm32.h"
#include "core/product/debug/xasm32/dasm32.h"
#include "vm/platform/platform.h"

#include "core/product/utils.h"

/* General Functions */
void utilsSleep(uint32_t milisec) {
    platformSleep(milisec);
}

/* NXVM Assembler Library */
uint8_t utilsAasm32(const char *stmt, uint8_t *rcode, int flag32) {
    return aasm32(stmt, rcode, flag32);
}
uint32_t utilsAasm32x(const char *stmt, uint8_t *rcode, int flag32) {
    return aasm32x(stmt, rcode, flag32);
}
uint8_t utilsDasm32(char *stmt, uint8_t *rcode, int flag32) {
    return dasm32(stmt, rcode, flag32);
}
