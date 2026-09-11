/* Copyright 2012-2014 Neko. */

#ifndef CORE_DEBUG_TEXT_INTERNAL_H
#define CORE_DEBUG_TEXT_INTERNAL_H

#include "type.h"

type_status core_debug_copy_text(C_CHAR *destination,
    STD_SIZE_T destination_capacity, const C_CHAR *source);
type_status core_debug_append_text(C_CHAR *destination,
    STD_SIZE_T destination_capacity, const C_CHAR *source);

#endif
