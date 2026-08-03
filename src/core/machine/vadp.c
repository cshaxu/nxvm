/* Copyright 2012-2014 Neko. */

/* VVADP is Video Adapter: not implemented yet. */

#include "core/machine/vadp.h"

void core_machine_vadp_initialize(t_vadp *adapter)
{
    if (adapter == NULL) return;
    MEMSET((void *)adapter, NTVDM64_TYPE_ZERO_8, sizeof(*adapter));
}

void core_machine_vadp_reset(t_vadp *adapter)
{
    if (adapter == NULL) return;
    MEMSET((void *)&adapter->data, NTVDM64_TYPE_ZERO_8, sizeof(adapter->data));
}

void core_machine_vadp_refresh(t_vadp *adapter)
{
    (void)adapter;
}

void core_machine_vadp_finalize(t_vadp *adapter)
{
    (void)adapter;
}
