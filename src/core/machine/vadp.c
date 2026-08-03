/* Copyright 2012-2014 Neko. */

/* VVADP is Video Adapter: not implemented yet. */

#include "type.h"

#include "core/machine/vadp.h"

C_VOID core_machine_vadp_initialize(t_vadp *adapter)
{
    if (adapter == STD_NULL) return;
    STD_MEMSET((C_VOID *)adapter, NTVDM64_TYPE_ZERO_8, sizeof(*adapter));
}

C_VOID core_machine_vadp_reset(t_vadp *adapter)
{
    if (adapter == STD_NULL) return;
    STD_MEMSET((C_VOID *)&adapter->data, NTVDM64_TYPE_ZERO_8, sizeof(adapter->data));
}

C_VOID core_machine_vadp_refresh(t_vadp *adapter)
{
    (C_VOID)adapter;
}

C_VOID core_machine_vadp_finalize(t_vadp *adapter)
{
    (C_VOID)adapter;
}
