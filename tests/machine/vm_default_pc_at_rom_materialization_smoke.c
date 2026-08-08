#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session.h"
#include "vm/profile/default_profile/firmware/bios.h"

#define TEST_ROM_CODE_PHYSICAL 0x000fe010u
#define TEST_ROM_RESET_LINEAR 0xfffffff0u
#define TEST_ROM_WORK_PHYSICAL (0x000f0000u + VBIOS_ADDR_HDD_PARAM)
#define TEST_BDA_VIDEO_MODE 0x00000449u
#define TEST_IVT_VIDEO 0x00000040u

int main(C_VOID)
{
    vm_session *session = STD_NULL;
    core_machine_memory_route route;
    uint8_t code_before = 0u;
    uint8_t code_after = 0u;
    uint8_t reset_opcode = 0u;
    uint8_t overwrite = 0u;
    uint8_t bda_mode = 0u;
    uint8_t ivt_before[4] = {0};
    uint8_t ivt_after[4] = {0};
    C_INT failed = vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL;

    if (!failed) {
        failed |= core_machine_memory_query(session->core_machine,
            TEST_ROM_CODE_PHYSICAL, 1u, CORE_MACHINE_MEMORY_ACCESS_READ,
            &route) != TYPE_STATUS_OK || route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
        failed |= core_machine_memory_query(session->core_machine,
            TEST_ROM_RESET_LINEAR, 1u, CORE_MACHINE_MEMORY_ACCESS_READ,
            &route) != TYPE_STATUS_OK || route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
        failed |= core_machine_memory_query(session->core_machine,
            TEST_ROM_CODE_PHYSICAL, 1u, CORE_MACHINE_MEMORY_ACCESS_WRITE,
            &route) != TYPE_STATUS_FAULT;
        failed |= core_machine_memory_query(session->core_machine,
            TEST_ROM_WORK_PHYSICAL, 1u, CORE_MACHINE_MEMORY_ACCESS_WRITE,
            &route) != TYPE_STATUS_OK || route != CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM;
        failed |= core_machine_memory_read(session->core_machine,
            TEST_ROM_CODE_PHYSICAL, &code_before, sizeof(code_before)) != TYPE_STATUS_OK;
        failed |= code_before != 'q';
        failed |= core_machine_memory_read(session->core_machine,
            TEST_ROM_RESET_LINEAR, &reset_opcode, sizeof(reset_opcode)) != TYPE_STATUS_OK ||
            reset_opcode != 0xeau;
        failed |= core_machine_memory_write(session->core_machine,
            TEST_ROM_CODE_PHYSICAL, &overwrite, sizeof(overwrite)) != TYPE_STATUS_FAULT;
        failed |= core_machine_memory_read(session->core_machine, TEST_IVT_VIDEO,
            ivt_before, sizeof(ivt_before)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(session->core_machine, TEST_IVT_VIDEO,
            &overwrite, sizeof(overwrite)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(session->core_machine, TEST_BDA_VIDEO_MODE,
            &overwrite, sizeof(overwrite)) != TYPE_STATUS_OK;
        vm_session_reset(session);
        failed |= core_machine_memory_read(session->core_machine,
            TEST_ROM_CODE_PHYSICAL, &code_after, sizeof(code_after)) != TYPE_STATUS_OK ||
            code_after != code_before;
        failed |= core_machine_memory_read(session->core_machine, TEST_IVT_VIDEO,
            ivt_after, sizeof(ivt_after)) != TYPE_STATUS_OK ||
            STD_MEMCMP(ivt_before, ivt_after, sizeof(ivt_before)) != 0;
        failed |= core_machine_memory_read(session->core_machine,
            TEST_BDA_VIDEO_MODE, &bda_mode, sizeof(bda_mode)) != TYPE_STATUS_OK ||
            bda_mode != 3u;
    }
    vm_session_destroy(session);
    if (failed) return 1;
    STD_PRINTF("M5:T289:S2:DEFAULT-ROM-MATERIALIZATION:OK\n");
    return 0;
}
