#include "lib/types/file.h"
#include "port_assembly_fixture.h"
lib_i32 main(void)
{
    if (port_assembly_dma_byte_lanes() ||
        port_assembly_port_b_time() ||
        port_assembly_create_failure() || port_assembly_pit_transaction() ||
        port_assembly_pic_transaction()) return 1;
    lib_i32 failed = 0;
    failed |= port_assembly_fdc_transaction(LIB_TRUE) ||
        port_assembly_fdc_transaction(LIB_FALSE) ||
        port_assembly_rtc_transaction() || port_assembly_rtc_collision();
    failed |= port_assembly_port_b_transaction(LIB_NULL);
    for (lib_u32 protocol = X86_NXVM_BOARD_HDC_PROTOCOL_ATA_PIO;
        protocol <= X86_NXVM_BOARD_HDC_PROTOCOL_XEBEC_XT; ++protocol) {
        failed |= port_assembly_hdc_transaction((x86_nxvm_board_hdc_protocol)protocol,
            LIB_TRUE, LIB_FALSE, LIB_FALSE) ||
            port_assembly_hdc_transaction((x86_nxvm_board_hdc_protocol)protocol,
                LIB_FALSE, LIB_FALSE, LIB_FALSE);
    }
    failed |= port_assembly_hdc_transaction(X86_NXVM_BOARD_HDC_PROTOCOL_XEBEC_XT,
        LIB_FALSE, LIB_TRUE, LIB_FALSE);
    failed |= port_assembly_hdc_transaction(X86_NXVM_BOARD_HDC_PROTOCOL_ATA_PIO,
        LIB_FALSE, LIB_FALSE, LIB_TRUE);
    if (failed) return 1;
    lib_c_printf("%s\n", "CORE-PORT-ROLLBACK:OK");
    return 0;
}
