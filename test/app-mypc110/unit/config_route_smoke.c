#include "app-mypc110/pc110_config_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "app-mypc110/pc110_inking_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/attachment_interface.h"

static void config_route_finalize(void *owner)
{
    (void)owner;
}

static lib_bool config_route_execute(x86_core_machine *machine,
    const lib_u8 *code, lib_size bytes, lib_u64 instructions)
{
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_CS] = 0u,
            [X86_CORE_CPU_DEBUG_EIP] = 0x1000u}
    };
    x86_core_machine_run_result result = {0};

    if (x86_core_machine_memory_write(machine, 0x1000u, code, bytes) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK)
        return LIB_FALSE;
    for (lib_u64 index = 0u; index < instructions; ++index) {
        if (x86_core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
            result.executed != 1u || result.reason != X86_CORE_MACHINE_STOP_BUDGET)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    static lib_u8 bios[MYPC110_FIRMWARE_BYTES];
    const lib_u8 unlock[] = {0xb8u, 0x00u, 0x80u, 0xe6u, 0x23u,
        0x86u, 0xe0u, 0xe6u, 0x22u, 0xe7u, 0x22u};
    const lib_u8 byte_only[] = {0xb0u, 0x00u, 0xe6u, 0x23u,
        0xb0u, 0x80u, 0xe6u, 0x22u, 0xe6u, 0x22u};
    const lib_u8 cpu_lock[] = {0xb8u, 0x00u, 0x01u, 0xe7u, 0x22u};
    const lib_u8 flash_toggle[] = {0xbau, 0x98u, 0x00u, 0xecu, 0x34u, 0x04u, 0xeeu};
    const lib_u8 flash_copy[] = {0xbau, 0x98u, 0x00u, 0xecu, 0x34u, 0x04u, 0xeeu,
        0xb8u, 0x00u, 0xe0u, 0x8eu, 0xd8u, 0xbeu, 0x00u, 0x00u, 0xacu,
        0xbfu, 0x00u, 0x20u, 0x26u, 0x88u, 0x05u, 0x31u, 0xc0u, 0x8eu, 0xd8u};
    const x86_core_machine_executor_config config = {
        .memory_bytes = 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX
    };
    x86_core_machine *machine = LIB_NULL;
    x86_vl82c420 *chip = LIB_NULL;
    x86_nxvm_board_pic_bus *master = LIB_NULL, *slave = LIB_NULL;
    mypc110_firmware firmware = {0};
    mypc110_inking inking = {0};
    lib_u32 value = 0u;
    lib_u8 byte = 0u;
    const lib_u8 patch = 0x5au;
    const lib_u8 locked_patch = 0xa5u;
    const lib_u8 instruction_patch = 0x90u;
    x86_core_machine_run_result result = {0};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_CS] = 0xc000u,
            [X86_CORE_CPU_DEBUG_EIP] = 0u}
    };
    lib_bool failed = LIB_FALSE;

    bios[0u] = 0x55u;
    bios[48u * 1024u] = 0x33u;
    bios[2u * MYPC110_FIRMWARE_BANK_BYTES] = 0xf4u;
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_nxvm_board_pic_initialize(&master, &slave, machine,
            X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        x86_nxvm_board_pic_irq_source_bind(&inking.irq, master, slave, 10u) != LIB_STATUS_OK ||
        mypc110_firmware_create(bios, sizeof(bios), &firmware) != LIB_STATUS_OK ||
        x86_vl82c420_create(&chip) != LIB_STATUS_OK ||
        mypc110_config_initialize_cold_image(chip) != LIB_STATUS_OK ||
        mypc110_config_install_routes(machine, &firmware, chip) != LIB_STATUS_INVALID_STATE ||
        firmware.configuration_chip != LIB_NULL ||
        x86_core_machine_bind_firmware_provider(machine,
            mypc110_firmware_provider(), &firmware) != LIB_STATUS_OK ||
        mypc110_firmware_install_routes(machine, &firmware) != LIB_STATUS_OK ||
        mypc110_config_install_routes(machine, &firmware, chip) != LIB_STATUS_OK ||
        mypc110_config_install_routes(machine, &firmware, chip) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_bind_attachment(machine, &(x86_core_machine_attachment){
            .context = &firmware,
            .finalize_devices = config_route_finalize,
            .system_management_active = mypc110_firmware_system_management_active}) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) {
        firmware.inking = &inking;
        inking.control = 0x38u;
        inking.pending = LIB_TRUE;
        x86_nxvm_board_pic_irq_source_assert(inking.irq);
        failed |= x86_core_machine_bus_write(machine, 0x74u, 0x16u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x76u, 0xa1u) != LIB_STATUS_OK ||
            !inking.pending || !x86_nxvm_board_pic_irq_source_is_asserted(inking.irq) ||
            x86_core_machine_bus_write(machine, 0x76u, 0xe2u) != LIB_STATUS_OK ||
            inking.pending || inking.control != 0xc0u ||
            x86_nxvm_board_pic_irq_source_is_asserted(inking.irq) ||
            x86_core_machine_bus_write(machine, 0x76u, 0xa1u) != LIB_STATUS_OK;
    }
    if (!failed) {
        failed |= x86_core_machine_bus_write(machine, 0xecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xedu, 0x09u) != LIB_STATUS_OK ||
            !config_route_execute(machine, flash_copy, sizeof(flash_copy), 12u) ||
            x86_core_machine_bus_read(machine, 0x98u, &value) != LIB_STATUS_OK || value != 4u ||
            x86_core_machine_memory_read(machine, 0x2000u, &byte, 1u) != LIB_STATUS_OK ||
            byte != bios[0u] ||
            !config_route_execute(machine, flash_toggle, sizeof(flash_toggle), 4u) ||
            x86_core_machine_bus_read(machine, 0x98u, &value) != LIB_STATUS_OK || value != 0u ||
            x86_core_machine_bus_write(machine, 0xecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xedu, MYPC110_RESET_ROMSET) != LIB_STATUS_OK;
    }
    if (!failed) {
        failed |= x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0u ||
            !config_route_execute(machine, byte_only, sizeof(byte_only), 5u) ||
            x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0u ||
            !config_route_execute(machine, unlock, sizeof(unlock), 5u) ||
            x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0x81u ||
            x86_core_machine_bus_write(machine, 0x0022u, 0x0au) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0x0bu ||
            !config_route_execute(machine, cpu_lock, sizeof(cpu_lock), 2u) ||
            x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0x0100u ||
            x86_core_machine_bus_write(machine, 0x0074u, 0x7au) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0076u, &value) != LIB_STATUS_OK ||
            value != 0x53u ||
            x86_core_machine_bus_write(machine, 0x0074u, 0x7bu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0076u, &value) != LIB_STATUS_OK ||
            value != 0x4cu ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x03u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0xccu ||
            x86_core_machine_bus_write(machine, 0x00fbu, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x03u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0xccu ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0x29u ||
            x86_core_machine_bus_write(machine, 0x00f9u, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x03u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0xccu ||
            x86_core_machine_bus_write(machine, 0x00fbu, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xfc23u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xf023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xc023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0023u, &value) != LIB_STATUS_OK ||
            value != 0x7fu ||
            x86_core_machine_bus_write(machine, 0x0024u, 0xb0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0xbau ||
            x86_core_machine_bus_write(machine, 0x0024u, 0xb8u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0x00u ||
            !config_route_execute(machine, cpu_lock, sizeof(cpu_lock), 2u) ||
            x86_core_machine_bus_write(machine, 0x0074u, 0x80u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x0076u, 0x5au) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x0074u, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0076u, &value) != LIB_STATUS_OK ||
            value != 0x5au ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00edu, 0x29u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0x29u ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0x00u ||
            x86_core_machine_bus_read(machine, 0xf023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0x00u ||
            x86_core_machine_bus_read(machine, 0xfc23u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xf023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xc023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x0024u, 0xb8u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x0025u, 0x6cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0x6cu ||
            !config_route_execute(machine, cpu_lock, sizeof(cpu_lock), 2u) ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0x6cu ||
            x86_core_machine_bus_write(machine, 0x0023u, 1u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0x0100u ||
            x86_core_machine_bus_read(machine, 0x0023u, &value) != LIB_STATUS_OK ||
            value != 0x7fu ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0x6cu ||
            x86_core_machine_memory_read(machine, MYPC110_VIDEO_PHYSICAL_START,
                &byte, 1u) != LIB_STATUS_OK || byte != 0xffu ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00edu, 0x09u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0x000e0000u,
                &byte, 1u) != LIB_STATUS_OK || byte != 0xf4u ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0fu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00edu, 0xffu) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, MYPC110_VIDEO_PHYSICAL_START,
                &patch, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, MYPC110_VIDEO_PHYSICAL_START,
                &byte, 1u) != LIB_STATUS_OK || byte != patch ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0fu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00edu, 0x2au) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, MYPC110_VIDEO_PHYSICAL_START,
                &locked_patch, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, MYPC110_VIDEO_PHYSICAL_START,
                &byte, 1u) != LIB_STATUS_OK || byte != patch ||
            x86_core_machine_memory_read(machine,
                MYPC110_VIDEO_PHYSICAL_START + 48u * 1024u,
                &byte, 1u) != LIB_STATUS_OK || byte != 0xffu ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0fu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00edu, 0xffu) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, MYPC110_VIDEO_PHYSICAL_START,
                &instruction_patch, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, MYPC110_VIDEO_PHYSICAL_START,
                &byte, 1u) != LIB_STATUS_OK || byte != instruction_patch ||
            x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            x86_core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            x86_core_machine_bus_read(machine, 0xfc23u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xf023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xc023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0023u, &value) != LIB_STATUS_OK ||
            value != 0x7fu ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0u ||
            x86_core_machine_bus_read(machine, 0x0023u, &value) != LIB_STATUS_OK ||
            value != 0xffu ||
            x86_core_machine_bus_write(machine, 0x0074u, 0x7au) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0076u, &value) != LIB_STATUS_OK ||
            value != 0x53u ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x03u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0xccu ||
            x86_core_machine_bus_write(machine, 0x00fbu, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x03u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0xccu ||
            x86_core_machine_memory_read(machine, MYPC110_VIDEO_PHYSICAL_START,
                &byte, 1u) != LIB_STATUS_OK || byte != 0xffu;
    }
    if (!failed) {
        const lib_u8 handler[] = {
            0xbau, 0x24u, 0u, 0xb0u, 0xb7u, 0xeeu, 0x42u,
            0xb0u, 0xdfu, 0xeeu, 0x4au, 0xb0u, 0xfau, 0xeeu,
            0x42u, 0xb0u, 1u, 0xeeu, 0x0fu, 0xaau};
        const lib_u8 smi_configuration[][2u] = {
            {0xfau, 1u}, {0x84u, 0xeeu}, {0x85u, 0x15u},
            {0xb5u, 4u}, {0xb6u, 0x80u}};
        x86_core_cpu_debug_cpu_snapshot before = {0};
        x86_core_cpu_debug_cpu_snapshot after = {0};
        lib_u16 dma = 0u;

        failed |= x86_core_machine_memory_write(machine, 0x30000u,
                &(lib_u8){0xaau}, 1u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x302u, 8u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0x30000u, &byte, 1u) != LIB_STATUS_OK || byte != 0xaau ||
            !config_route_execute(machine, unlock, sizeof(unlock), 5u) ||
            x86_core_machine_bus_write(machine, 0x22u, 0x0au) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x302u, 8u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0x30000u, &byte, 1u) != LIB_STATUS_OK || byte != 0u ||
            x86_core_machine_memory_write(machine, 0x30000u,
                &(lib_u8){0xbbu}, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x38000u, handler, sizeof(handler)) != LIB_STATUS_OK ||
            x86_core_machine_dma_memory_cycle(machine, 0x30000u, 1u, 2u,
                X86_CORE_MACHINE_MEMORY_ACCESS_READ, &dma, LIB_NULL, LIB_NULL, LIB_NULL) !=
                LIB_STATUS_OK || dma != 0xaau;
        dma = 0xccu;
        failed |= !failed && (x86_core_machine_dma_memory_cycle(machine, 0x30000u, 1u, 2u,
                X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &dma, LIB_NULL, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0x30000u, &byte, 1u) != LIB_STATUS_OK || byte != 0xbbu ||
            x86_core_machine_bus_write(machine, 0x302u, 0u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0x30000u, &byte, 1u) != LIB_STATUS_OK || byte != 0xccu ||
            !config_route_execute(machine, cpu_lock, sizeof(cpu_lock), 2u) ||
            x86_core_machine_bus_read(machine, 0xfc23u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xf023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xc023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x23u, &value) != LIB_STATUS_OK);
        for (lib_size index = 0u; !failed && index < sizeof(smi_configuration) / sizeof(smi_configuration[0]); ++index)
            failed |= x86_core_machine_bus_write(machine, 0x24u, smi_configuration[index][0]) != LIB_STATUS_OK ||
                x86_core_machine_bus_write(machine, 0x25u, smi_configuration[index][1]) != LIB_STATUS_OK;
        failed |= !failed && (x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15eeu, 0x80u) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result) != LIB_STATUS_OK ||
            !firmware.smram_active || result.linear_pc != 0x38000u ||
            x86_core_machine_dma_memory_cycle(machine, 0x30000u, 1u, 2u,
                X86_CORE_MACHINE_MEMORY_ACCESS_READ, &dma, LIB_NULL, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
            dma != 0xccu);
        for (lib_u8 instruction = 0u; !failed && instruction < 13u; ++instruction)
            failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result) !=
                LIB_STATUS_OK || result.executed != 1u;
        failed |= !failed && (firmware.smram_active || x86_vl82c420_smi_asserted(chip) ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            after.eip != before.eip || after.cs.base != before.cs.base ||
            after.eax != before.eax || after.edx != before.edx ||
            x86_core_machine_memory_read(machine, 0x30000u, &byte, 1u) != LIB_STATUS_OK || byte != 0xccu ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK || firmware.memory_control != 0u ||
            firmware.smram_active || x86_vl82c420_smi_asserted(chip) ||
            !config_route_execute(machine, unlock, sizeof(unlock), 5u) ||
            x86_core_machine_bus_write(machine, 0x22u, 0x0au) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x302u, 8u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0x30000u, &byte, 1u) != LIB_STATUS_OK || byte != 0u ||
            x86_core_machine_memory_read(machine, 0x38000u, &byte, 1u) != LIB_STATUS_OK || byte != 0u);
    }
    if (!failed) {
        /* A physical button is not monitor resume. No event means no reset;
         * a new asserted edge resumes through RESET without clearing RAM. */
        failed |= x86_core_machine_bus_read(machine, 0xfc23u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xf023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xc023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x23u, &value) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x2200u, &patch, 1u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x24u, 0xb8u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x25u, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x24u, 0xfbu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x25u, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x704u, &value) != LIB_STATUS_OK || value != 1u ||
            mypc110_firmware_set_resume_button(&firmware, LIB_TRUE) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x24u, 0xffu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x25u, 0x82u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x704u, &value) != LIB_STATUS_OK || value != 0u ||
            mypc110_firmware_set_resume_button(&firmware, LIB_TRUE) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result) !=
                LIB_STATUS_OK || result.executed != 0u ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            mypc110_firmware_set_resume_button(&firmware, LIB_FALSE) != LIB_STATUS_OK ||
            mypc110_firmware_set_resume_button(&firmware, LIB_TRUE) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x704u, &value) != LIB_STATUS_OK || value != 1u ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result) !=
                LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_RESET_REQUESTED ||
            x86_core_machine_memory_read(machine, 0x2200u, &byte, 1u) != LIB_STATUS_OK || byte != patch ||
            x86_core_machine_bus_write(machine, 0x24u, 0xb8u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x25u, &value) != LIB_STATUS_OK || (value & 8u) == 0u ||
            x86_core_machine_bus_write(machine, 0x24u, 0xfbu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x25u, &value) != LIB_STATUS_OK || (value & 8u) == 0u;
        failed |= !failed && (x86_core_machine_bus_write(machine, 0x24u, 0xffu) !=
                LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x25u, 0x82u) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_vl82c420_suspend_refresh_enabled(chip));
    }
    if (machine != LIB_NULL && chip != LIB_NULL) {
        failed |= x86_core_machine_remove_port_routes(machine, &firmware) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0076u, &value) != LIB_STATUS_UNSUPPORTED;
    }
    x86_nxvm_board_pic_finalize(master, slave);
    x86_core_machine_destroy(machine);
    mypc110_firmware_destroy(&firmware);
    x86_vl82c420_destroy(chip);
    return failed ? 1 : 0;
}
