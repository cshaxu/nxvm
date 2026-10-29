#include "app-mypc110/firmware_variants.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "app-mypc110/pc110_font_interface.h"
#include "app-mypc110/pc110_config_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/debug_interface.h"

static lib_bool check_variant_read_routes(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX};
    mypc110_firmware dstn = {0}, tft = {0};
    x86_core_machine *machine = LIB_NULL;
    x86_vl82c420 *chip = LIB_NULL;
    lib_bool ok = LIB_FALSE;
    lib_size differences = 0u;
    lib_size sample = 0u;
    const lib_u8 *dstn_bios = mypc110_dstn_firmware.bios[0].data;
    lib_u8 value;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot cpu = {0};
    x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)};

    if (mypc110_firmware_create(mypc110_dstn_firmware.bios[0].data,
            MYPC110_FIRMWARE_BYTES, &dstn) != LIB_STATUS_OK ||
        mypc110_firmware_create(mypc110_tft_firmware.bios[0].data,
            MYPC110_FIRMWARE_BYTES, &tft) != LIB_STATUS_OK ||
        x86_vl82c420_create(&chip) != LIB_STATUS_OK ||
        mypc110_config_initialize_cold_image(chip) != LIB_STATUS_OK ||
        x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_core_machine_bind_firmware_provider(machine,
            mypc110_firmware_provider(), &dstn) != LIB_STATUS_OK ||
        mypc110_firmware_install_routes(machine, &dstn) != LIB_STATUS_OK ||
        mypc110_config_install_routes(machine, &dstn, chip) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0xecu, 0x0cu) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0xedu, 0x09u) != LIB_STATUS_OK)
        goto done;

    /* Compare the actual replacement route against both immutable inputs. */
    for (lib_size index = 0u; index < MYPC110_FIRMWARE_BYTES; ++index) {
        if (dstn.bios[index] == tft.bios[index]) continue;
        if (differences == 0u) sample = index;
        if (index < 0x20000u || index >= 0x24000u ||
            x86_core_machine_memory_read(machine, 0xc0000u + (lib_u32)index,
                &value, 1u) != LIB_STATUS_OK || value != dstn.bios[index])
            goto done;
        ++differences;
    }
    if (differences == 0u) goto done;
    if (mypc110_firmware_select(LIB_NULL, tft.bios, MYPC110_FIRMWARE_BYTES) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        mypc110_firmware_select(&dstn, LIB_NULL, MYPC110_FIRMWARE_BYTES) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        mypc110_firmware_select(&dstn, tft.bios, MYPC110_FIRMWARE_BYTES - 1u) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        x86_core_machine_memory_read(machine, 0xc0000u + (lib_u32)sample,
            &value, 1u) != LIB_STATUS_OK || value != dstn_bios[sample]) goto done;
    if (mypc110_firmware_select(&dstn, tft.bios, MYPC110_FIRMWARE_BYTES) != LIB_STATUS_OK)
        goto done;
    for (lib_size index = 0x20000u; index < 0x24000u; ++index) {
        if (dstn_bios[index] == tft.bios[index]) continue;
        if (x86_core_machine_memory_read(machine, 0xc0000u + (lib_u32)index,
                &value, 1u) != LIB_STATUS_OK || value != tft.bios[index]) goto done;
    }
    /* Synthetic instructions test actual execution fetching through the same
     * route after invalidation; they do not modify either embedded asset. */
    tft.bios[0x20000u] = 0xb0u;
    tft.bios[0x20001u] = 0x12u;
    entry.values[X86_CORE_CPU_DEBUG_CS] = 0xe000u;
    entry.values[X86_CORE_CPU_DEBUG_EIP] = 0u;
    if (mypc110_firmware_select(&dstn, tft.bios, MYPC110_FIRMWARE_BYTES) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result) != LIB_STATUS_OK ||
        x86_core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK ||
        (cpu.eax & 0xffu) != 0x12u) goto done;
    tft.bios[0x20001u] = 0x34u;
    if (mypc110_firmware_select(&dstn, tft.bios, MYPC110_FIRMWARE_BYTES) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result) != LIB_STATUS_OK ||
        x86_core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK ||
        (cpu.eax & 0xffu) != 0x34u) goto done;
    /* Reject an incompatible reset bank without changing the current view. */
    tft.bios[0x30000u] ^= 1u;
    if (mypc110_firmware_select(&dstn, tft.bios, MYPC110_FIRMWARE_BYTES) != LIB_STATUS_UNSUPPORTED ||
        x86_core_machine_memory_read(machine, 0xe0001u, &value, 1u) != LIB_STATUS_OK ||
        value != 0x34u) goto done;
    tft.bios[0x30000u] ^= 1u;
    /* Shadow writes remain independent, and a cold reset clears the shadow
     * while keeping the selected external firmware view. */
    if (x86_core_machine_bus_write(machine, 0xecu, 0x11u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0xedu, 0x03u) != LIB_STATUS_OK)
        goto done;
    value = 0x5au;
    if (x86_core_machine_memory_write(machine, 0xc0000u + (lib_u32)sample,
            &value, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0xc0000u + (lib_u32)sample,
            &value, 1u) != LIB_STATUS_OK || value != 0x5au ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0xecu, 0x0cu) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0xedu, 0x09u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0xc0000u + (lib_u32)sample,
            &value, 1u) != LIB_STATUS_OK || value != tft.bios[sample]) goto done;
    ok = LIB_TRUE;
done:
    x86_core_machine_destroy(machine);
    x86_vl82c420_destroy(chip);
    mypc110_firmware_destroy(&dstn);
    mypc110_firmware_destroy(&tft);
    return ok;
}

int main(void)
{
    const x86_nxvm_machine_assets *tft = &mypc110_tft_firmware;
    const x86_nxvm_machine_assets *dstn = &mypc110_dstn_firmware;

    if (tft->bios[0].data == LIB_NULL || dstn->bios[0].data == LIB_NULL ||
        tft->bios[0].data == dstn->bios[0].data ||
        tft->bios[0].bytes != MYPC110_FIRMWARE_BYTES ||
        dstn->bios[0].bytes != MYPC110_FIRMWARE_BYTES ||
        tft->font.data != LIB_NULL || tft->font.bytes != 0u || dstn->font.data == LIB_NULL ||
        dstn->font.bytes != MYPC110_FONT_ROM_BYTES ||
        tft->cmos_seed.data == LIB_NULL || tft->cmos_seed.bytes != 128u ||
        dstn->cmos_seed.data != LIB_NULL || dstn->cmos_seed.bytes != 0u)
        return 1;
    return check_variant_read_routes() ? 0 : 1;
}
