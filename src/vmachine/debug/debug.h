/* This file is a part of NXVM project. */

#ifndef NXVM_DEBUG_H
#define NXVM_DEBUG_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "../vmachine.h"

t_nubit32 _dbgm_addr_physical_linear(t_nubit32 linear);
t_nubit32 _dbgm_addr_linear_logical(t_cpu_sreg *rsreg, t_nubit32 offset);
t_nubit32 _dbgm_addr_physical_logical(t_cpu_sreg *rsreg, t_nubit32 offset);
t_nubit64 _dbgm_read_physical(t_nubit32 physical, t_nubit8 byte);
t_nubit64 _dbgm_read_linear(t_nubit32 linear, t_nubit8 byte);
t_nubit64 _dbgm_read_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte);

void debug();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
