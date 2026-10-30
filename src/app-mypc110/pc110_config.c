#include "app-mypc110/pc110_config_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "app-mypc110/pc110_inking_interface.h"
#include "x86-core/machine/machine_interface.h"

#define MYPC110_CPU_POWER_MODE_PORT 0x0022u
#define MYPC110_CPU_CONFIG_PORT 0x0023u
#define MYPC110_BLOCK2_UNLOCK_FC23_PORT 0xfc23u
#define MYPC110_BLOCK2_UNLOCK_F023_PORT 0xf023u
#define MYPC110_BLOCK2_UNLOCK_C023_PORT 0xc023u
#define MYPC110_BLOCK2_INDEX_PORT 0x0024u
#define MYPC110_BLOCK2_DATA_PORT 0x0025u
#define MYPC110_SCAMP_INDEX_PORT 0x0074u
#define MYPC110_SCAMP_DATA_PORT 0x0076u
#define MYPC110_ECED_INDEX_PORT 0x00ecu
#define MYPC110_ECED_DATA_PORT 0x00edu
#define MYPC110_ECED_CONFIG_CLOSE_PORT 0x00f9u
#define MYPC110_ECED_CONFIG_OPEN_PORT 0x00fbu
#define MYPC110_MEMORY_CONTROL_PORT 0x0302u
#define MYPC110_MEMORY_REFRESH_STATUS_PORT 0x0704u
#define MYPC110_ASMI_PORT 0x15eeu
#define MYPC110_PLANAR_CONTROL_PORT 0x0098u

/* L2 cold image: Open-Source-PC110 live VL82C420 captures, not imported
 * implementation code.  The source files' SHA-256 values are recorded in
 * T580 S19 evidence.  The source's no-module EC/ED[03] is replaced by the
 * documented 16 MiB module value because this App constructs 20 MiB.
 * Shadow access fields start external; guest firmware prepares internal RAM. */
static const lib_u8 mypc110_cold_scamp[] = {
    0x00u, 0xbbu, 0x80u, 0x00u, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0x6fu, 0x7eu, 0x50u,
    0x80u, 0x00u, 0x00u, 0x90u, 0xf0u, 0xe4u, 0xa1u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x8eu, 0x02u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x98u, 0x8au,
    0x10u, 0x14u, 0x10u, 0x20u, 0x08u, 0xbau, 0x9eu, 0xf1u, 0x5au, 0x50u, 0xf1u, 0x3cu, 0x0au, 0x1eu, 0x2cu, 0x01u,
    0x02u, 0x05u, 0x01u, 0x88u, 0x03u, 0x00u, 0x00u, 0x00u, 0x00u, 0x0au, 0x03u, 0x88u, 0x05u, 0x00u, 0x00u, 0x00u,
    0x08u, 0x1eu, 0x11u, 0x88u, 0x11u, 0x00u, 0x00u, 0x00u, 0x0cu, 0x00u, 0x00u, 0x88u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x10u, 0x4fu, 0x0fu, 0x10u, 0x53u, 0x4cu, 0x00u, 0x10u, 0x15u, 0xeeu
};

static const lib_u8 mypc110_cold_block2[] = {
    0xaau, 0xaau, 0xaau, 0xaau, 0xaau, 0xaau, 0xaau, 0xaau, 0x40u, 0x41u, 0x42u, 0x43u, 0xffu, 0xffu, 0x00u, 0x0fu,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu,
    0xffu, 0xffu, 0x11u, 0x08u, 0x04u, 0x01u, 0x00u, 0x20u, 0x0bu, 0x0cu, 0x00u, 0x08u, 0xffu, 0xffu, 0x80u, 0xffu,
    0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu,
    0x00u, 0x00u, 0x12u, 0x00u, 0x50u, 0x05u, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0x00u, 0xffu,
    0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu,
    0x00u, 0x01u, 0x00u, 0xffu, 0xffu, 0x00u, 0x00u, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu,
    0x02u, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu,
    0xffu, 0xffu, 0xffu, 0xffu, 0xeeu, 0x15u, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu,
    0xaau, 0xaau, 0xaau, 0xaau, 0xaau, 0xaau, 0xaau, 0xaau, 0xc0u, 0x41u, 0x42u, 0x43u, 0xaau, 0xaau, 0x00u, 0x0eu,
    0xffu, 0xffu, 0x11u, 0x70u, 0x02u, 0x01u, 0x00u, 0x20u, 0x0bu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu,
    0xbau, 0x9eu, 0xf0u, 0x5au, 0x50u, 0xf5u, 0xdau, 0x00u, 0x00u, 0x40u, 0x00u, 0x00u, 0x02u, 0xffu, 0x10u, 0x14u,
    0x00u, 0x00u, 0x00u, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xf2u, 0x03u, 0xa1u, 0x02u, 0xffu, 0xffu, 0xffu, 0xffu,
    0x60u, 0x00u, 0x24u, 0x00u, 0xffu, 0xffu, 0xffu, 0xffu, 0x00u, 0x00u, 0x20u, 0x00u, 0xffu, 0xffu, 0xffu, 0xffu,
    0x00u, 0x00u, 0x00u, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xf4u, 0x03u, 0xa1u, 0x02u, 0xffu, 0xffu, 0xffu, 0xffu,
    0x80u, 0x00u, 0x00u, 0x00u, 0x10u, 0x20u, 0x08u, 0x07u, 0x0eu, 0xffu, 0xffu, 0x18u, 0xe0u, 0xffu, 0x3fu, 0x00u
};

static const lib_u8 mypc110_cold_eced[] = {
    0x42u, 0xd5u, 0x0bu, 0xccu, 0x06u, 0xa8u, 0x1au, 0xecu, 0x38u, 0x00u, 0x03u, 0x00u, MYPC110_RESET_ROMSET, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x1au, 0x04u, 0x08u, 0x74u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x10u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u
};

static lib_status mypc110_config_write_cold_bank(x86_vl82c420 *chip,
    x86_vl82c420_bank bank, const lib_u8 *bytes, lib_size count)
{
    lib_status status = LIB_STATUS_OK;

    if (chip == LIB_NULL || bytes == LIB_NULL || count > 256u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (lib_size index = 0u; status == LIB_STATUS_OK && index < count; ++index) {
        status = x86_vl82c420_select(chip, bank, (lib_u8)index);
        if (status == LIB_STATUS_OK) {
            status = x86_vl82c420_write_selected(chip, bank, bytes[index]);
        }
    }
    return status;
}

lib_status mypc110_config_initialize_cold_image(x86_vl82c420 *chip)
{
    lib_status status;

    if (chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = mypc110_config_write_cold_bank(chip, X86_VL82C420_BANK_SCAMP,
        mypc110_cold_scamp, sizeof(mypc110_cold_scamp));
    if (status == LIB_STATUS_OK) {
        status = mypc110_config_write_cold_bank(chip, X86_VL82C420_BANK_ECED,
            mypc110_cold_eced, sizeof(mypc110_cold_eced));
    }
    if (status == LIB_STATUS_OK) {
        x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_FC23);
        x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_F023);
        x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_C023);
        x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_0023);
        status = mypc110_config_write_cold_bank(chip, X86_VL82C420_BANK_BLOCK2,
            mypc110_cold_block2, sizeof(mypc110_cold_block2));
        x86_vl82c420_relock(chip);
    }
    return status;
}

static lib_status mypc110_config_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    mypc110_firmware *firmware = owner;
    x86_vl82c420 *chip;
    x86_vl82c420_bank bank;
    lib_u8 value;
    lib_status status;

    (void)tick;
    if (firmware == LIB_NULL || firmware->configuration_chip == LIB_NULL ||
        out_value == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    chip = firmware->configuration_chip;
    switch (port) {
    case MYPC110_MEMORY_REFRESH_STATUS_PORT:
        /* L2 functional REFNORM: resume firmware waits for normal refresh
         * after clearing suspend refresh. No host delay or read-count rule. */
        *out_value = x86_vl82c420_suspend_refresh_enabled(chip) ? 0u : 1u;
        return LIB_STATUS_OK;
    case MYPC110_PLANAR_CONTROL_PORT:
        *out_value = firmware->planar_control;
        return LIB_STATUS_OK;
    case MYPC110_CPU_POWER_MODE_PORT:
        *out_value = firmware->cpu_power_mode;
        return LIB_STATUS_OK;
    case MYPC110_MEMORY_CONTROL_PORT:
        *out_value = (firmware->cpu_power_mode & 0x0eu) == 0x0au ?
            firmware->memory_control : 0xffffu;
        return LIB_STATUS_OK;
    case MYPC110_BLOCK2_UNLOCK_FC23_PORT:
        x86_vl82c420_observe_unlock_read(chip,
            X86_VL82C420_UNLOCK_READ_FC23);
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    case MYPC110_BLOCK2_UNLOCK_F023_PORT:
        x86_vl82c420_observe_unlock_read(chip,
            X86_VL82C420_UNLOCK_READ_F023);
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    case MYPC110_BLOCK2_UNLOCK_C023_PORT:
        x86_vl82c420_observe_unlock_read(chip,
            X86_VL82C420_UNLOCK_READ_C023);
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    case MYPC110_CPU_CONFIG_PORT:
        x86_vl82c420_observe_unlock_read(chip,
            X86_VL82C420_UNLOCK_READ_0023);
        status = x86_vl82c420_read_selected(chip,
            X86_VL82C420_BANK_BLOCK2, &value);
        if (status != LIB_STATUS_OK && status != LIB_STATUS_INVALID_STATE)
            return status;
        *out_value = status == LIB_STATUS_OK ? 0x7fu : 0xffu;
        return LIB_STATUS_OK;
    case MYPC110_BLOCK2_DATA_PORT:
        bank = X86_VL82C420_BANK_BLOCK2;
        break;
    case MYPC110_SCAMP_DATA_PORT:
        bank = X86_VL82C420_BANK_SCAMP;
        break;
    case MYPC110_ECED_DATA_PORT:
        bank = X86_VL82C420_BANK_ECED;
        break;
    default: return LIB_STATUS_UNSUPPORTED;
    }
    status = x86_vl82c420_read_selected(chip, bank, &value);
    if (status == LIB_STATUS_INVALID_STATE && bank == X86_VL82C420_BANK_BLOCK2) {
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    }
    if (status != LIB_STATUS_OK) return status;
    *out_value = value;
    return LIB_STATUS_OK;
}

static lib_status mypc110_config_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    mypc110_firmware *firmware = owner;
    x86_vl82c420 *chip;
    x86_vl82c420_bank bank;

    if (firmware == LIB_NULL || firmware->configuration_chip == LIB_NULL ||
        value > 0xffffu) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    chip = firmware->configuration_chip;
    if (port == MYPC110_PLANAR_CONTROL_PORT) {
        if (access_bytes != 1u || value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
        const lib_u8 previous = firmware->planar_control;
        firmware->planar_control = (lib_u8)value;
        if (((previous ^ value) & 4u) != 0u)
            x86_core_machine_signal_memory_decode_change(firmware->machine,
                0x000e0000u, 2u * MYPC110_FIRMWARE_BANK_BYTES);
        return LIB_STATUS_OK;
    }
    if (port == MYPC110_ASMI_PORT) {
        x86_vl82c420_write_asmi(chip, port);
        return x86_core_machine_set_smi(firmware->machine, x86_vl82c420_smi_asserted(chip));
    }
    if (port == MYPC110_MEMORY_CONTROL_PORT) {
        if ((firmware->cpu_power_mode & 0x0eu) != 0x0au) return LIB_STATUS_OK;
        if (access_bytes != 1u && access_bytes != 2u) return LIB_STATUS_UNSUPPORTED;
        lib_u16 control = access_bytes == 1u ?
            (lib_u16)((firmware->memory_control & 0xff00u) | value) : (lib_u16)value;
        if (control != firmware->memory_control) {
            firmware->memory_control = control;
            x86_core_machine_signal_memory_decode_change(firmware->machine,
                MYPC110_SMRAM_PHYSICAL_START, MYPC110_SMRAM_BYTES);
        }
        return LIB_STATUS_OK;
    }
    /* L2 SL-compatible configuration protocol, distinct from Block 2's
     * indexed-register unlock. The final write must be a word access. */
    if (port == MYPC110_CPU_CONFIG_PORT) {
        if (access_bytes == 1u && (value & 1u) != 0u) {
            firmware->cpu_power_mode &= (lib_u16)~1u;
            firmware->cpu_unlock_step = 0u;
        } else {
            firmware->cpu_unlock_step = access_bytes == 1u &&
                (value & 0x80u) == 0u ? 1u : 0u;
        }
        return LIB_STATUS_OK;
    }
    if (port == MYPC110_CPU_POWER_MODE_PORT) {
        if ((firmware->cpu_power_mode & 1u) == 0u) {
            if (firmware->cpu_unlock_step == 1u && access_bytes == 1u &&
                (value & 0x80u) != 0u) {
                firmware->cpu_unlock_step = 2u;
            } else {
                if (firmware->cpu_unlock_step == 2u && access_bytes == 2u &&
                    (value & 0x8000u) == 0u)
                    firmware->cpu_power_mode = (lib_u16)(value | 1u);
                firmware->cpu_unlock_step = 0u;
            }
        } else if (access_bytes == 1u) {
            firmware->cpu_power_mode = (lib_u16)(
                (firmware->cpu_power_mode & 0xff00u) | (value & 0xfeu) | 1u);
        } else if (access_bytes == 2u) {
            firmware->cpu_power_mode = (lib_u16)(value | 1u);
        }
        if (access_bytes == 2u && (value & 0x0100u) != 0u) {
            /* CPU configuration lock is independent of Block 2's protocol. */
            firmware->cpu_power_mode &= (lib_u16)~1u;
            firmware->cpu_unlock_step = 0u;
        }
        return LIB_STATUS_OK;
    }
    /* L2 compatibility strobes. The unchanged BIOS accesses EC/ED after F9;
     * reference models keep the bank accessible. Do not invent a hard gate. */
    if (port == MYPC110_ECED_CONFIG_OPEN_PORT ||
        port == MYPC110_ECED_CONFIG_CLOSE_PORT) return LIB_STATUS_OK;
    if (value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == MYPC110_BLOCK2_INDEX_PORT) {
        lib_status status = x86_vl82c420_select(chip,
            X86_VL82C420_BANK_BLOCK2, (lib_u8)value);
        return status == LIB_STATUS_INVALID_STATE ? LIB_STATUS_OK : status;
    }
    if (port == MYPC110_SCAMP_INDEX_PORT) {
        return x86_vl82c420_select(chip,
            X86_VL82C420_BANK_SCAMP, (lib_u8)value);
    }
    if (port == MYPC110_ECED_INDEX_PORT) {
        return x86_vl82c420_select(chip,
            X86_VL82C420_BANK_ECED, (lib_u8)value);
    }
    if (port == MYPC110_BLOCK2_DATA_PORT) {
        bank = X86_VL82C420_BANK_BLOCK2;
    } else if (port == MYPC110_SCAMP_DATA_PORT) bank = X86_VL82C420_BANK_SCAMP;
    else if (port == MYPC110_ECED_DATA_PORT) bank = X86_VL82C420_BANK_ECED;
    else return LIB_STATUS_UNSUPPORTED;
    {
        lib_u8 inking_before = 0u;
        lib_u8 cache_index = 0u, cache_before = 0u;
        if (bank == X86_VL82C420_BANK_ECED) {
            lib_status status = x86_vl82c420_selected_index(chip, bank, &cache_index);
            if (status != LIB_STATUS_OK) return status;
            status = x86_vl82c420_read_selected(chip, bank, &cache_before);
            if (status != LIB_STATUS_OK) return status;
        }
        if (bank == X86_VL82C420_BANK_SCAMP && firmware->inking != LIB_NULL) {
            lib_status status = x86_vl82c420_read_register(chip, bank, 0x16u,
                &inking_before);
            if (status != LIB_STATUS_OK) return status;
        }
        lib_status status = x86_vl82c420_write_selected(chip, bank,
            (lib_u8)value);

        if (status == LIB_STATUS_OK && bank == X86_VL82C420_BANK_ECED &&
            cache_before != (lib_u8)value) {
            lib_u32 start = 0u;
            lib_size bytes = 0u;
            if (cache_index == 0x07u && ((cache_before ^ value) & 0x08u) != 0u) {
                start = MYPC110_VIDEO_PHYSICAL_START;
                bytes = MYPC110_FIRMWARE_BYTES;
            } else if (cache_index == 0x0cu &&
                ((cache_before ^ value) & 0x30u) != 0u) {
                start = 0xe0000u;
                bytes = 0x20000u;
            } else if ((cache_index >= 0x0fu && cache_index <= 0x12u) ||
                (cache_index >= 0x15u && cache_index <= 0x18u &&
                    ((cache_before ^ value) & 0xaau) != 0u)) {
                start = MYPC110_VIDEO_PHYSICAL_START +
                    ((cache_index - (cache_index <= 0x12u ? 0x0fu : 0x15u)) << 16u);
                bytes = 0x10000u;
            }
            if (bytes != 0u)
                x86_core_machine_signal_memory_decode_change(firmware->machine, start, bytes);
        }

        if (status == LIB_STATUS_OK && bank == X86_VL82C420_BANK_SCAMP &&
            firmware->inking != LIB_NULL) {
            lib_u8 inking_after;
            status = x86_vl82c420_read_register(chip, bank, 0x16u, &inking_after);
            if (status == LIB_STATUS_OK && inking_before != inking_after)
                mypc110_inking_reset(firmware->inking);
        }

        if (status == LIB_STATUS_OK && bank == X86_VL82C420_BANK_BLOCK2) {
            status = x86_core_machine_set_smi(firmware->machine, x86_vl82c420_smi_asserted(chip));
            if (status == LIB_STATUS_OK)
                status = x86_core_machine_set_stop_clock(firmware->machine,
                    x86_vl82c420_suspend_refresh_enabled(chip));
        }
        return status == LIB_STATUS_INVALID_STATE &&
            bank == X86_VL82C420_BANK_BLOCK2 ? LIB_STATUS_OK : status;
    }
}

lib_status mypc110_firmware_set_resume_button(mypc110_firmware *firmware,
    lib_bool asserted)
{
    if (firmware == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (firmware->machine == LIB_NULL || firmware->configuration_chip == LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    if (x86_vl82c420_set_resume_button(firmware->configuration_chip, asserted)) {
        lib_status status = x86_core_machine_set_stop_clock(firmware->machine, LIB_FALSE);
        if (status != LIB_STATUS_OK) return status;
        x86_core_machine_signal_processor_reset(firmware->machine);
    }
    return LIB_STATUS_OK;
}

lib_status mypc110_config_install_routes(x86_core_machine *machine,
    mypc110_firmware *firmware, x86_vl82c420 *chip)
{
    lib_status status;
    const x86_core_machine_port_route port_routes[] = {
        {MYPC110_PLANAR_CONTROL_PORT, mypc110_config_read, mypc110_config_write, firmware, LIB_FALSE, 0u},
        {MYPC110_MEMORY_CONTROL_PORT, mypc110_config_read, mypc110_config_write, firmware, LIB_FALSE, 0u},
        {MYPC110_MEMORY_REFRESH_STATUS_PORT, mypc110_config_read, LIB_NULL, firmware, LIB_FALSE, 0u},
        {MYPC110_ASMI_PORT, LIB_NULL, mypc110_config_write, firmware, LIB_FALSE, 0u},
        {MYPC110_CPU_POWER_MODE_PORT, mypc110_config_read, mypc110_config_write, firmware, LIB_FALSE, 0u},
        {MYPC110_BLOCK2_INDEX_PORT, LIB_NULL, mypc110_config_write, firmware, LIB_FALSE, 0u},
        {MYPC110_BLOCK2_DATA_PORT, mypc110_config_read, mypc110_config_write, firmware, LIB_FALSE, 0u},
        {MYPC110_SCAMP_INDEX_PORT, LIB_NULL, mypc110_config_write, firmware, LIB_FALSE, 0u},
        {MYPC110_SCAMP_DATA_PORT, mypc110_config_read, mypc110_config_write, firmware, LIB_FALSE, 0u},
        {MYPC110_ECED_INDEX_PORT, LIB_NULL, mypc110_config_write, firmware, LIB_FALSE, 0u},
        {MYPC110_ECED_DATA_PORT, mypc110_config_read, mypc110_config_write, firmware, LIB_FALSE, 0u},
        {MYPC110_ECED_CONFIG_CLOSE_PORT, LIB_NULL, mypc110_config_write, firmware, LIB_FALSE, 0u},
        {MYPC110_ECED_CONFIG_OPEN_PORT, LIB_NULL, mypc110_config_write, firmware, LIB_FALSE, 0u},
        {MYPC110_BLOCK2_UNLOCK_FC23_PORT, mypc110_config_read, LIB_NULL, firmware, LIB_FALSE, 0u},
        {MYPC110_BLOCK2_UNLOCK_F023_PORT, mypc110_config_read, LIB_NULL, firmware, LIB_FALSE, 0u},
        {MYPC110_BLOCK2_UNLOCK_C023_PORT, mypc110_config_read, LIB_NULL, firmware, LIB_FALSE, 0u},
        {MYPC110_CPU_CONFIG_PORT, mypc110_config_read, mypc110_config_write, firmware, LIB_FALSE, 0u}
    };

    if (machine == LIB_NULL || firmware == LIB_NULL || chip == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (firmware->machine != machine) return LIB_STATUS_INVALID_STATE;
    status = x86_core_machine_install_port_routes(machine, port_routes,
        sizeof(port_routes) / sizeof(port_routes[0]));
    if (status == LIB_STATUS_OK) firmware->configuration_chip = chip;
    return status;
}
