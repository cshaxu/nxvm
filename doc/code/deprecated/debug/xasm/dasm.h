/* This file is a part of NXVM project. */

#ifndef NXVM_DASM_H
#define NXVM_DASM_H

#include "../vcpu.h"

t_nubitcc dasmx(t_string stmt, t_nubit32 linear, t_nubit8 flagout);
t_nubitcc dasm(t_string stmt, t_nubit16 seg, t_nubit16 off, t_nubit8 flagout);

#endif
