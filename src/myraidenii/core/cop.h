#ifndef MYRAIDENII_CORE_COP_H
#define MYRAIDENII_CORE_COP_H

#include "lib/types/types_interface.h"
#include "x86-core/machine/memory_interface.h"

typedef struct myraidenii_core_cop myraidenii_core_cop;

lib_status myraidenii_core_cop_create(myraidenii_core_cop **out_cop);
void myraidenii_core_cop_destroy(myraidenii_core_cop *cop);
void myraidenii_core_cop_reset(myraidenii_core_cop *cop);
lib_status myraidenii_core_cop_install(myraidenii_core_cop *cop,
    x86_core_machine *machine);

#endif
