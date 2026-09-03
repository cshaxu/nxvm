#include "type.h"

#include "core/machine/machine.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "core/machine/rtc.h"
#include "../support/rom/session_assets.h"

static C_INT vm_default_pc_at_fdd_format_is_valid(
    vm_session_floppy_format format, type_unsigned_16 cylinders,
    type_unsigned_16 sectors, type_unsigned_8 cmos_type)
{
    vm_session_config config = {0};
    vm_session *session = STD_NULL;
    type_unsigned_32 port_b;
    type_unsigned_32 next_port_b;
    type_unsigned_32 tick;
    type_unsigned_16 checksum = 0u;
    type_unsigned_8 index;

    config.profile_kind = VM_SESSION_PROFILE_DEFAULT_PC_AT;
    config.floppy_format = format;
    if (vm_test_default_pc_at_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL ||
        session->fdd.data.ncyl != cylinders || session->fdd.data.nhead != 2u ||
        session->fdd.data.nsector != sectors || session->fdd.data.nbyte != 512u ||
        session->core_machine->shared_rtc.registers[CORE_MACHINE_RTC_TYPE_DISK_FLOPPY] !=
            cmos_type) {
        STD_PRINTF("FDD setup format=%u cmos=%02x expected=%02x\n",
            (unsigned int)format, (unsigned int)session->core_machine->
            shared_rtc.registers[CORE_MACHINE_RTC_TYPE_DISK_FLOPPY],
            (unsigned int)cmos_type);
        vm_session_destroy(session);
        return 0;
    }
    for (index = 0x10u; index < 0x2eu; ++index) {
        checksum = (type_unsigned_16)(checksum +
            session->core_machine->shared_rtc.registers[index]);
    }
    if (session->core_machine->shared_rtc.registers[0x2eu] !=
            (type_unsigned_8)(checksum >> 8u) ||
        session->core_machine->shared_rtc.registers[0x2fu] !=
            (type_unsigned_8)checksum) {
        vm_session_destroy(session);
        return 0;
    }
    if (core_machine_bus_read(session->core_machine, 0x0061u, &port_b) !=
        TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return 0;
    }
    for (tick = 0u; tick < 200u; ++tick) {
        if (core_machine_advance_time(session->core_machine, 1u) != TYPE_STATUS_OK ||
            core_machine_bus_read(session->core_machine, 0x0061u, &next_port_b) !=
                TYPE_STATUS_OK) {
            vm_session_destroy(session);
            return 0;
        }
        if ((port_b & 0x10u) != (next_port_b & 0x10u)) break;
    }
    if (tick == 200u) {
        vm_session_destroy(session);
        return 0;
    }
    vm_session_destroy(session);
    return 1;
}

static C_INT vm_default_pc_at_80186_refresh_polling_is_live(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0xb4u, 0x10u, 0xe4u, 0x61u, 0x24u, 0x10u,
        0x3au, 0xc4u, 0x74u, 0xf8u, 0xf4u
    };
    vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_DEFAULT_PC_AT,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80186
    };
    vm_session *session = STD_NULL;
    core_machine_run_result result = {0};
    type_unsigned_32 port_b;
    type_unsigned_32 tick;
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) {
        return 0;
    }
    for (tick = 0u; tick < 200u; ++tick) {
        if (core_machine_bus_read(session->core_machine, 0x0061u, &port_b) !=
                TYPE_STATUS_OK || (port_b & 0x10u) == 0u) break;
        if (core_machine_advance_time(session->core_machine, 1u) != TYPE_STATUS_OK) {
            failed = 1;
            break;
        }
    }
    if (!failed && (tick == 200u || core_machine_memory_write(session->core_machine,
            0x0500u, program, sizeof(program)) != TYPE_STATUS_OK)) {
        failed = 1;
    }
    if (!failed) {
        session->core_machine->executor_cpu.data.cs.selector = 0u;
        session->core_machine->executor_cpu.data.cs.base = 0u;
        session->core_machine->executor_cpu.data.ds.selector = 0u;
        session->core_machine->executor_cpu.data.ds.base = 0u;
        session->core_machine->executor_cpu.data.es.selector = 0u;
        session->core_machine->executor_cpu.data.es.base = 0u;
        session->core_machine->executor_cpu.data.ss.selector = 0u;
        session->core_machine->executor_cpu.data.ss.base = 0u;
        session->core_machine->executor_cpu.data.eip = 0x0500u;
        session->core_machine->executor_cpu.data.sp = 0xfffeu;
        session->core_machine->executor_cpu.data.flagHalt = TYPE_FALSE;
        failed = core_machine_run(session->core_machine,
            (core_machine_run_budget) {1000u, 0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    }
    vm_session_destroy(session);
    return !failed;
}

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    if (!session->active || session->profile == STD_NULL ||
        session->core_machine->fdc.connect.config.dor_port != 0x03f2u ||
        session->core_machine->fdc.connect.config.status_port != 0x03f4u ||
        session->core_machine->fdc.connect.config.data_port != 0x03f5u ||
        session->core_machine->fdc.connect.config.direction_port != 0x03f7u ||
        session->core_machine->fdc.connect.config.irq != 6u ||
        session->core_machine->fdc.connect.config.dma_channel != 2u ||
        session->core_machine->fdc.connect.config.unready_read_policy !=
            CORE_MACHINE_FDC_UNREADY_READ_GENERIC) {
        vm_session_destroy(session);
        return 1;
    }
    if (session->core_machine->shared_rtc.registers[CORE_MACHINE_RTC_EQUIPMENT] !=
            0x21u || session->core_machine->shared_rtc.registers[
            CORE_MACHINE_RTC_BASEMEM_LSB] != 0x7fu ||
        session->core_machine->shared_rtc.registers[
            CORE_MACHINE_RTC_BASEMEM_MSB] != 0x02u) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    if (!vm_default_pc_at_fdd_format_is_valid(VM_SESSION_FLOPPY_FORMAT_360K,
            40u, 9u, 0x10u) ||
        !vm_default_pc_at_fdd_format_is_valid(VM_SESSION_FLOPPY_FORMAT_720K,
            80u, 9u, 0x30u) ||
        !vm_default_pc_at_fdd_format_is_valid(VM_SESSION_FLOPPY_FORMAT_1200K,
            80u, 15u, 0x20u) ||
        !vm_default_pc_at_fdd_format_is_valid(VM_SESSION_FLOPPY_FORMAT_1440K,
            80u, 18u, 0x40u)) {
        return 1;
    }
    if (!vm_default_pc_at_80186_refresh_polling_is_live()) return 1;
    puts("M5:T208:S3:DEFAULT-PC-AT-APPLY:OK");
    return 0;
}
