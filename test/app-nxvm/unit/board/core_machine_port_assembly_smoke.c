#include "app-mydeskpro386/profiles/d4_platform_interface.h"
#include "../../../core/board-base/composition/port_assembly_fixture.h"
#include <stdio.h>

/* The product owns this D4 attachment; Shared knows only the test operation. */
static lib_status port_assembly_attach_d4(core_machine_board_state *board)
{
    const core_machine_d4_platform_config config = {CORE_MACHINE_PC_AT_PORT_B, 0u};
    core_machine_d4_platform *platform = LIB_NULL;
    return core_machine_d4_platform_attach(board, &config, &platform);
}

lib_i32 main(void)
{
    lib_i32 failed = port_assembly_range_transaction() ||
        port_assembly_batch_transaction() ||
        port_assembly_dma_byte_lanes() || port_assembly_read_time() ||
        port_assembly_port_b_time() ||
        port_assembly_create_failure() ||
        port_assembly_pit_transaction() || port_assembly_pic_transaction();

    /* Chip creation, then all seven routes, including the optional diagnostic. */
    for (lib_size fail_at = 0u; fail_at <= 7u; ++fail_at)
        failed |= port_assembly_fdc_transaction(fail_at);

    if (failed) return 1;
    puts("M5:T313:S3:PORT-ASSEMBLY:OK");

    failed = port_assembly_rtc_transaction(1u) || port_assembly_rtc_transaction(2u) ||
        port_assembly_rtc_collision();
    for (lib_u32 d4 = 0u; d4 <= 1u; ++d4)
        for (lib_size fail_at = 1u; fail_at <= 2u; ++fail_at)
            failed |= port_assembly_port_b_transaction(
                d4 ? port_assembly_attach_d4 : LIB_NULL, fail_at);
    for (lib_u32 protocol = CORE_MACHINE_HDC_PROTOCOL_ATA_PIO;
        protocol <= CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT; ++protocol) {
        const lib_size routes = protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT ? 7u :
            protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB ? 19u : 18u;
        for (lib_size fail_at = 0u; fail_at <= routes; ++fail_at)
            failed |= port_assembly_hdc_transaction((core_machine_hdc_protocol)protocol,
                fail_at, LIB_FALSE, LIB_FALSE);
    }
    failed |= port_assembly_hdc_transaction(CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT,
        0u, LIB_TRUE, LIB_FALSE);
    failed |= port_assembly_hdc_transaction(CORE_MACHINE_HDC_PROTOCOL_ATA_PIO,
        0u, LIB_FALSE, LIB_TRUE);

    if (failed) return 1;
    puts("M5:T313:S4:CONTROLLER-ROLLBACK:OK");
    return 0;
}
