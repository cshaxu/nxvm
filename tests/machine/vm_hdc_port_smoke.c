#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/machine.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session.h"
#include "core/machine/hdc.h"
#include "vm/profile/default_profile/pc_at_profile.h"

#define HDC_DATA_PORT 0x01f0u
#define HDC_ERROR_PORT 0x01f1u
#define HDC_SECTOR_COUNT_PORT 0x01f2u
#define HDC_SECTOR_NUMBER_PORT 0x01f3u
#define HDC_CYLINDER_LOW_PORT 0x01f4u
#define HDC_CYLINDER_HIGH_PORT 0x01f5u
#define HDC_DRIVE_HEAD_PORT 0x01f6u
#define HDC_STATUS_COMMAND_PORT 0x01f7u
#define HDC_ALT_STATUS_CONTROL_PORT 0x03f6u

static C_INT vm_hdc_write(core_machine *machine, uint16_t port, uint32_t value)
{
    return core_machine_bus_write(machine, port, value) == TYPE_STATUS_OK;
}

static C_INT vm_hdc_read(core_machine *machine, uint16_t port, uint32_t *value)
{
    return core_machine_bus_read(machine, port, value) == TYPE_STATUS_OK;
}

static C_INT vm_hdc_program_chs(vm_session *session)
{
    return vm_hdc_write(session->core_machine, HDC_SECTOR_COUNT_PORT, 1u) &&
        vm_hdc_write(session->core_machine, HDC_SECTOR_NUMBER_PORT, 1u) &&
        vm_hdc_write(session->core_machine, HDC_CYLINDER_LOW_PORT, 0u) &&
        vm_hdc_write(session->core_machine, HDC_CYLINDER_HIGH_PORT, 0u) &&
        vm_hdc_write(session->core_machine, HDC_DRIVE_HEAD_PORT, 0u);
}

static C_INT vm_hdc_program_lba(vm_session *session, uint32_t lba,
    uint8_t sector_count)
{
    return vm_hdc_write(session->core_machine, HDC_SECTOR_COUNT_PORT,
            sector_count) &&
        vm_hdc_write(session->core_machine, HDC_SECTOR_NUMBER_PORT,
            (uint8_t)lba) &&
        vm_hdc_write(session->core_machine, HDC_CYLINDER_LOW_PORT,
            (uint8_t)(lba >> 8u)) &&
        vm_hdc_write(session->core_machine, HDC_CYLINDER_HIGH_PORT,
            (uint8_t)(lba >> 16u)) &&
        vm_hdc_write(session->core_machine, HDC_DRIVE_HEAD_PORT,
            0x40u | (uint8_t)(lba >> 24u));
}

static C_INT vm_hdc_drain_data(core_machine *machine, uint16_t *first_word)
{
    STD_SIZE_T index;
    uint32_t value;

    for (index = 0u; index < 256u; ++index) {
        if (!vm_hdc_read(machine, HDC_DATA_PORT, &value)) return 0;
        if (index == 0u && first_word != STD_NULL) *first_word = (uint16_t)value;
    }
    return 1;
}

static C_INT vm_hdc_fill_data(core_machine *machine, uint16_t first_word)
{
    STD_SIZE_T index;

    for (index = 0u; index < 256u; ++index) {
        if (!vm_hdc_write(machine, HDC_DATA_PORT,
                index == 0u ? first_word : 0u)) return 0;
    }
    return 1;
}

static C_INT vm_hdc_profile_contract_is_valid(C_VOID)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();
    const vm_profile_default_pc_at_hdc_pio *hdc;

    if (profile == STD_NULL) return 0;
    hdc = &profile->hdc_pio;
    return hdc->data_port == HDC_DATA_PORT &&
        hdc->error_features_port == HDC_ERROR_PORT &&
        hdc->sector_count_port == HDC_SECTOR_COUNT_PORT &&
        hdc->sector_number_port == HDC_SECTOR_NUMBER_PORT &&
        hdc->cylinder_low_port == HDC_CYLINDER_LOW_PORT &&
        hdc->cylinder_high_port == HDC_CYLINDER_HIGH_PORT &&
        hdc->drive_head_port == HDC_DRIVE_HEAD_PORT &&
        hdc->status_command_port == HDC_STATUS_COMMAND_PORT &&
        hdc->alternate_status_device_control_port == HDC_ALT_STATUS_CONTROL_PORT &&
        hdc->irq == 14u &&
        hdc->dma_channel == VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL &&
        hdc->data_width_bits == 16u && hdc->register_width_bits == 8u &&
        hdc->lba28_supported && !hdc->slave_present &&
        !hdc->secondary_channel_present;
}

static C_INT vm_hdc_progress_probe(vm_session *session)
{
    uint32_t value;
    uint16_t word;

    if (!vm_hdc_program_lba(session, 0u, 2u) ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0x20u) ||
        !vm_hdc_drain_data(session->core_machine, &word) ||
        !vm_hdc_read(session->core_machine, HDC_SECTOR_COUNT_PORT, &value) ||
        value != 1u || !vm_hdc_read(session->core_machine,
            HDC_SECTOR_NUMBER_PORT, &value) || value != 1u ||
        !core_machine_hdc_irq_pending(&session->core_machine->shared_hdc) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        core_machine_hdc_irq_pending(&session->core_machine->shared_hdc) ||
        !vm_hdc_drain_data(session->core_machine, &word) ||
        !vm_hdc_read(session->core_machine, HDC_SECTOR_COUNT_PORT, &value) ||
        value != 0u || !vm_hdc_read(session->core_machine,
            HDC_SECTOR_NUMBER_PORT, &value) || value != 1u) return 0;
    if (!vm_hdc_program_lba(session, 3u, 2u) ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0x30u) ||
        !vm_hdc_fill_data(session->core_machine, 0x1357u) ||
        !vm_hdc_read(session->core_machine, HDC_SECTOR_COUNT_PORT, &value) ||
        value != 1u || !vm_hdc_fill_data(session->core_machine, 0x2468u) ||
        !vm_hdc_read(session->core_machine, HDC_SECTOR_COUNT_PORT, &value) ||
        value != 0u || !vm_hdc_program_lba(session, 3u, 2u) ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0x20u) ||
        !vm_hdc_drain_data(session->core_machine, &word) || word != 0x1357u ||
        !vm_hdc_drain_data(session->core_machine, &word) || word != 0x2468u) return 0;
    return 1;
}

C_INT main(C_VOID)
{
    vm_session_config config = {0};
    vm_session *session = STD_NULL;
    vm_session *no_media = STD_NULL;
    uint32_t value;
    uint32_t invalid_lba;
    uint16_t word = 0u;
    C_INT failed = 0;

    config.create_hdd_cylinders = 2u;
    if (!vm_hdc_profile_contract_is_valid() ||
        vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || session->core_machine == STD_NULL) goto fail;
    invalid_lba = (uint32_t)session->hdd.data.ncyl * session->hdd.data.nhead *
        session->hdd.data.nsector;
    if (!session->media_registry.frozen || session->media_registry.binding_count != 2u ||
        session->media_registry.bindings[VM_SESSION_MEDIA_HDD_ID - 1u].context !=
            &session->hdd ||
        session->media_registry.bindings[VM_SESSION_MEDIA_HDD_ID - 1u].provider !=
            vm_machine_hdd_media_provider() ||
        session->core_machine->shared_hdc.connect.media_registry != &session->media_registry ||
        session->core_machine->shared_hdc.connect.media_id != VM_SESSION_MEDIA_HDD_ID ||
        session->core_machine->shared_hdc.connect.irq_source.master == STD_NULL ||
        session->core_machine->shared_hdc.connect.irq_source.slave == STD_NULL ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0xecu) ||
        !vm_hdc_read(session->core_machine, HDC_ALT_STATUS_CONTROL_PORT, &value) ||
        value != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC |
            CORE_MACHINE_HDC_STATUS_DRQ) || !core_machine_hdc_irq_pending(
                &session->core_machine->shared_hdc) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        core_machine_hdc_irq_pending(&session->core_machine->shared_hdc) ||
        !vm_hdc_drain_data(session->core_machine, &word) || word != 0x0040u ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        value != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC) ||
        !vm_hdc_program_chs(session) ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0x30u) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        value != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC |
            CORE_MACHINE_HDC_STATUS_DRQ) ||
        !vm_hdc_fill_data(session->core_machine, 0xa55au) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        value != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC) ||
        !vm_hdc_program_chs(session) ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0x20u) ||
        !vm_hdc_drain_data(session->core_machine, &word) || word != 0xa55au ||
        !vm_hdc_program_lba(session, 1u, 1u) ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0x30u) ||
        !vm_hdc_fill_data(session->core_machine, 0x5aa5u) ||
        !vm_hdc_program_lba(session, 1u, 1u) ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0x20u) ||
        !vm_hdc_drain_data(session->core_machine, &word) || word != 0x5aa5u ||
        !vm_hdc_program_lba(session, 0u, 0u) ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0x20u) ||
        session->core_machine->shared_hdc.data.sectors_remaining != 256u ||
        !vm_hdc_read(session->core_machine, HDC_ALT_STATUS_CONTROL_PORT, &value) ||
        !core_machine_hdc_irq_pending(&session->core_machine->shared_hdc) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        core_machine_hdc_irq_pending(&session->core_machine->shared_hdc) ||
        !vm_hdc_read(session->core_machine, HDC_DATA_PORT, &value) ||
        session->core_machine->shared_hdc.data.sectors_remaining != 256u ||
        !vm_hdc_write(session->core_machine, HDC_ALT_STATUS_CONTROL_PORT, 0x04u) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        value != CORE_MACHINE_HDC_STATUS_BSY || core_machine_hdc_irq_pending(
            &session->core_machine->shared_hdc) ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0x20u) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        value != CORE_MACHINE_HDC_STATUS_BSY ||
        !vm_hdc_write(session->core_machine, HDC_ALT_STATUS_CONTROL_PORT, 0x00u) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        value != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC) ||
        !vm_hdc_program_lba(session, invalid_lba, 1u) ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0x20u) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        value != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR) ||
        !vm_hdc_read(session->core_machine, HDC_ERROR_PORT, &value) ||
        value != CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND ||
        !vm_hdc_write(session->core_machine, HDC_DRIVE_HEAD_PORT, 0x10u) ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0xecu) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        value != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR) ||
        !vm_hdc_read(session->core_machine, HDC_ERROR_PORT, &value) ||
        value != CORE_MACHINE_HDC_ERROR_ABORT ||
        !vm_hdc_write(session->core_machine, HDC_STATUS_COMMAND_PORT, 0x99u) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        value != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR) ||
        !vm_hdc_read(session->core_machine, HDC_ERROR_PORT, &value) ||
        value != CORE_MACHINE_HDC_ERROR_ABORT ||
        !vm_hdc_write(session->core_machine, HDC_ALT_STATUS_CONTROL_PORT, 0x04u) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        value != CORE_MACHINE_HDC_STATUS_BSY ||
        !vm_hdc_write(session->core_machine, HDC_ALT_STATUS_CONTROL_PORT, 0x00u) ||
        !vm_hdc_read(session->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        value != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC)) {
        failed = 1;
    }
    if (!failed && !vm_hdc_progress_probe(session)) failed = 1;
    vm_session_destroy(session);
    session = STD_NULL;

    if (failed || vm_session_create(STD_NULL, &no_media) != TYPE_STATUS_OK ||
        no_media == STD_NULL || !vm_hdc_program_chs(no_media) ||
        !vm_hdc_write(no_media->core_machine, HDC_STATUS_COMMAND_PORT, 0x20u) ||
        !vm_hdc_read(no_media->core_machine, HDC_STATUS_COMMAND_PORT, &value) ||
        value != (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR) ||
        !vm_hdc_read(no_media->core_machine, HDC_ERROR_PORT, &value) ||
        value != CORE_MACHINE_HDC_ERROR_ABORT) {
        failed = 1;
    }
    vm_session_destroy(no_media);
    if (failed) return 1;
    STD_PRINTF("M5:T253:S2:ATA-PIO-PROGRESS:PORT:OK lba=%04X irq=14\n",
        0x5aa5u);
    return 0;

fail:
    vm_session_destroy(session);
    vm_session_destroy(no_media);
    return 1;
}
