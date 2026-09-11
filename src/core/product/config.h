/* Copyright 2012-2014 Neko. */

#ifndef CORE_PRODUCT_CONFIG_H
#define CORE_PRODUCT_CONFIG_H

#include "type.h"

type_status core_product_parse_memory_kib(const C_CHAR *text,
    STD_SIZE_T *out_memory_bytes);

#endif
