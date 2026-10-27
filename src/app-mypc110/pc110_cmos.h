#ifndef MYPC110_CMOS_H
#define MYPC110_CMOS_H

#include "x86-nxvm/machine/machine_interface.h"

lib_status mypc110_cmos_load(const char *path, lib_u8 *image, lib_bool *present);
lib_status mypc110_cmos_save(const char *path, x86_nxvm_machine *machine);

#endif
