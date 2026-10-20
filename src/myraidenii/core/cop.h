#ifndef MYRAIDENII_CORE_COP_H
#define MYRAIDENII_CORE_COP_H

#include "lib/types/types_interface.h"
#include "x86-core/machine/memory_interface.h"

typedef struct myraidenii_core_cop myraidenii_core_cop;
typedef struct myraidenii_core_video myraidenii_core_video;

typedef enum myraidenii_core_cop_operation {
    MYRAIDENII_CORE_COP_OPERATION_NONE,
    MYRAIDENII_CORE_COP_OPERATION_COMMAND,
    MYRAIDENII_CORE_COP_OPERATION_DMA,
    MYRAIDENII_CORE_COP_OPERATION_SORT
} myraidenii_core_cop_operation;

/* This is a bounded failure summary for the Board diagnostic only. It never
 * retains guest payload or execution position. Rejected DMA triggers retain
 * only their selected source, size, destination and relative descriptor values. */
typedef struct myraidenii_core_cop_failure_observation {
    lib_bool valid;
    myraidenii_core_cop_operation operation;
    lib_u16 operation_value;
    lib_u16 dma_source;
    lib_u16 dma_size;
    lib_u16 dma_destination;
    lib_u16 dma_relative;
    lib_u32 physical;
    lib_status status;
} myraidenii_core_cop_failure_observation;

lib_status myraidenii_core_cop_create(myraidenii_core_video *video,
    myraidenii_core_cop **out_cop);
void myraidenii_core_cop_destroy(myraidenii_core_cop *cop);
void myraidenii_core_cop_reset(myraidenii_core_cop *cop);
lib_status myraidenii_core_cop_advance(myraidenii_core_cop *cop);
lib_status myraidenii_core_cop_failure_get(const myraidenii_core_cop *cop,
    myraidenii_core_cop_failure_observation *out_observation);
lib_status myraidenii_core_cop_install(myraidenii_core_cop *cop,
    x86_core_machine *machine);

#endif
