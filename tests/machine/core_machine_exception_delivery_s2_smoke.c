#define main exception_delivery_s2_inc_dec_main
#include "core_machine_inc_dec_smoke.c"
#undef main

#define main exception_delivery_s2_paging_main
#include "core_machine_80386_paging_smoke.c"
#undef main

#define main exception_delivery_s2_fpu_interface_main
#include "core_machine_fpu_interface_s65_smoke.c"
#undef main

#define main exception_delivery_s2_fpu_8087_main
#include "core_machine_fpu_8087_smoke.c"
#undef main

C_INT main(C_VOID)
{
    if (exception_delivery_s2_inc_dec_main() ||
        exception_delivery_s2_paging_main() ||
        exception_delivery_s2_fpu_interface_main() ||
        exception_delivery_s2_fpu_8087_main()) {
        return 1;
    }
    STD_PRINTF("M5:T321:S2:EXCEPTION-DELIVERY:OK\n");
    return 0;
}
