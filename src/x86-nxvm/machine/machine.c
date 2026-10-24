#include "lib/types/types_interface.h"

#include "x86-nxvm/machine/machine_private.h"

#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/machine/control.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/display.h"
#include "x86-nxvm/machine/frame_interface.h"
#include "x86-nxvm/machine/media/media_interface.h"
#include "x86-nxvm/machine/machine_devices.h"
#include "x86-nxvm/machine/media/fdd_interface.h"
#include "x86-nxvm/machine/media/hdd_interface.h"
#include "x86-nxvm/machine/keyboard_mapper_interface.h"
#include "x86-nxvm/machine/mouse_mapper_interface.h"

static lib_status x86_nxvm_machine_insert_floppy_at(x86_nxvm_machine *session, lib_size slot,
    const char *path, lib_storage_medium_mode mode);
static lib_status x86_nxvm_machine_remove_fdd_direct(x86_nxvm_machine *session);

static lib_status x86_nxvm_machine_deliver_key(x86_nxvm_machine *session,
    const kvm_input_event *event)
{
    lib_status status = LIB_STATUS_OK;

    if (session == LIB_NULL || !session->active) return LIB_STATUS_INVALID_ARGUMENT;
    {
        x86_nxvm_machine_keyboard_sequence sequence;
        lib_u8 native_scan_set;

        if (x86_nxvm_board_keyboard_get_native_scan_set(session->board,
                &native_scan_set) == LIB_STATUS_OK &&
            x86_nxvm_machine_keyboard_map_kvm_event_for_scan_set(
                event,
                native_scan_set, &sequence) ==
            LIB_STATUS_OK) {
            status = x86_nxvm_board_keyboard_receive_native_bytes(session->board,
                sequence.bytes, sequence.count);
        }
    }
    return status;
}

static lib_status x86_nxvm_machine_deliver_mouse(x86_nxvm_machine *session,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons)
{
    if (session == LIB_NULL || !session->active) return LIB_STATUS_INVALID_ARGUMENT;
    {
        x86_nxvm_machine_mouse_report report;

        if (x86_nxvm_machine_mouse_map_host_relative(
                delta_x, delta_y, buttons, &report) ==
            LIB_STATUS_OK) {
            return x86_nxvm_board_mouse_receive_relative(session->board,
                report.delta_x, report.delta_y, report.buttons);
        }
    }
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_deliver_emulator_input(x86_nxvm_machine *session,
    const kvm_input_event *event)
{
    if (event == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (event->type == KVM_EVENT_KEY) return x86_nxvm_machine_deliver_key(session, event);
    if (event->type == KVM_EVENT_MOUSE) return x86_nxvm_machine_deliver_mouse(session,
        event->data.mouse.delta_x, event->data.mouse.delta_y,
        event->data.mouse.buttons);
    return LIB_STATUS_UNSUPPORTED;
}

lib_status x86_nxvm_machine_copy_emulator_frame(x86_nxvm_machine *machine,
    kvm_window_frame_storage *frame, kvm_console_text_frame *console, lib_bool *produced)
{
    lib_status status;

    if (machine == LIB_NULL || frame == LIB_NULL || console == LIB_NULL || produced == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *produced = LIB_FALSE;
    status = x86_nxvm_machine_publish_display(machine, LIB_FALSE);
    if (status != LIB_STATUS_OK) return status;
    if (!machine->display_frame_pending) {
        return LIB_STATUS_OK;
    }
    status = x86_nxvm_machine_frame_from_display(&machine->latest_snapshot,
        machine->display_generation, frame, console);
    if (status != LIB_STATUS_OK) return status;
    machine->display_frame_pending = LIB_FALSE;
    *produced = LIB_TRUE;
    return LIB_STATUS_OK;
}

static lib_status x86_nxvm_machine_copy_path(char *destination, lib_size capacity,
    const char *source)
{
    lib_size length;

    if (destination == LIB_NULL || capacity == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    destination[0] = '\0';
    if (source == LIB_NULL) return LIB_STATUS_OK;
    length = lib_text_length(source);
    if (length >= capacity) return LIB_STATUS_LIMIT_EXCEEDED;
    lib_memory_copy(destination, source, length + 1u);
    return LIB_STATUS_OK;
}

static const char *x86_nxvm_machine_config_floppy(const x86_nxvm_machine_config *config,
    lib_size slot)
{
    if (config == LIB_NULL || slot >= X86_NXVM_MACHINE_FLOPPY_SLOT_COUNT) return LIB_NULL;
    return config->floppy_image[slot];
}

static const char *x86_nxvm_machine_config_fixed_disk(const x86_nxvm_machine_config *config,
    lib_size slot)
{
    if (config == LIB_NULL || slot >= X86_NXVM_MACHINE_FIXED_DISK_SLOT_COUNT) return LIB_NULL;
    return config->fixed_disk_image[slot];
}

static lib_storage_medium_mode x86_nxvm_machine_config_floppy_mode(
    const x86_nxvm_machine_config *config, lib_size slot)
{
    return config != LIB_NULL && slot < X86_NXVM_MACHINE_FLOPPY_SLOT_COUNT ?
        config->floppy_mode[slot] : LIB_STORAGE_MEDIUM_OVERLAY;
}

static lib_storage_medium_mode x86_nxvm_machine_config_fixed_disk_mode(
    const x86_nxvm_machine_config *config, lib_size slot)
{
    return config != LIB_NULL && slot < X86_NXVM_MACHINE_FIXED_DISK_SLOT_COUNT ?
        config->fixed_disk_mode[slot] : LIB_STORAGE_MEDIUM_OVERLAY;
}

lib_status x86_nxvm_machine_apply_cmos_seed(const x86_nxvm_machine *session,
    x86_nxvm_board_plan_topology *topology)
{
    lib_size index;

    if (session == LIB_NULL || topology == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!session->construction.cmos_seed_present) return LIB_STATUS_OK;
    if (!topology->rtc_cmos_present) return LIB_STATUS_INVALID_STATE;
    /* A seed is a board configuration image, not a replacement RTC state:
     * copy only the manual-defined CMOS NVRAM 0Eh--3Fh.  It is the sole
     * owner of board configuration, including the firmware checksum. */
    for (index = 0u; index < X86_NXVM_BOARD_RTC_DEFAULT_CAPACITY; ++index) {
        topology->rtc_cmos.defaults[index] = (x86_nxvm_board_rtc_default_byte) {
            (lib_u8)(0x0eu + index), session->construction.cmos_seed[0x0eu + index] };
    }
    topology->rtc_cmos.default_count = X86_NXVM_BOARD_RTC_DEFAULT_CAPACITY;
    topology->rtc_cmos.derive_configuration_checksum = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_get_speed(const x86_nxvm_machine *session,
    x86_nxvm_machine_speed *out_speed)
{
    if (session == LIB_NULL || !session->active || out_speed == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_speed = session->speed;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_set_speed(x86_nxvm_machine *session, x86_nxvm_machine_speed speed)
{
    if (session == LIB_NULL || !session->active ||
        (speed != X86_NXVM_MACHINE_SPEED_STANDARD && speed != X86_NXVM_MACHINE_SPEED_TURBO)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (x86_nxvm_machine_control_is_running(&session->control)) return LIB_STATUS_INVALID_STATE;
    session->speed = speed;
    return LIB_STATUS_OK;
}

static lib_status x86_nxvm_machine_insert_floppy_at(x86_nxvm_machine *session, lib_size slot,
    const char *path, lib_storage_medium_mode mode)
{
    char candidate[sizeof(session->floppy_image_path[0])];
    lib_status status;

    if (session == LIB_NULL || slot >= session->construction.floppy_slot_count)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (x86_nxvm_machine_control_is_running(&session->control)) return LIB_STATUS_INVALID_STATE;
    status = x86_nxvm_machine_copy_path(candidate, sizeof(candidate), path);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_machine_fdd_insert_for(session->floppy[slot], candidate, mode);
    if (status != LIB_STATUS_OK) return status;
    lib_memory_copy(session->floppy_image_path[slot], candidate,
        lib_text_length(candidate) + 1u);
    return LIB_STATUS_OK;
}

static lib_status x86_nxvm_machine_remove_fdd_direct(x86_nxvm_machine *session)
{
    lib_status status;

    if (session == LIB_NULL || session->construction.floppy_slot_count == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (x86_nxvm_machine_control_is_running(&session->control)) return LIB_STATUS_INVALID_STATE;
    status = x86_nxvm_machine_fdd_remove_for(session->floppy[0u]);
    if (status == LIB_STATUS_OK) session->floppy_image_path[0u][0] = '\0';
    return status;
}

lib_status x86_nxvm_machine_set_emulator_media(x86_nxvm_machine *session, const char *path,
    lib_storage_medium_mode mode)
{
    if (session == LIB_NULL || !session->active) return LIB_STATUS_INVALID_STATE;
    if (session->construction.floppy_slot_count == 0u) return LIB_STATUS_UNSUPPORTED;
    if (path != LIB_NULL && path[0] != '\0')
        return x86_nxvm_machine_insert_floppy_at(session, 0u, path, mode);
    return x86_nxvm_machine_remove_fdd_direct(session);
}

lib_i32 x86_nxvm_machine_insert_fdd(x86_nxvm_machine *session, const char *path)
{ return session != LIB_NULL && session->executor != LIB_NULL &&
    emulator_machine_set_removable_media(session->executor, path,
        LIB_STORAGE_MEDIUM_OVERLAY) ? 0 : -1; }

lib_i32 x86_nxvm_machine_eject_fdd(x86_nxvm_machine *session)
{ return session != LIB_NULL && session->executor != LIB_NULL &&
    emulator_machine_set_removable_media(session->executor, LIB_NULL,
        LIB_STORAGE_MEDIUM_OVERLAY) ? 0 : -1; }
static lib_status x86_nxvm_machine_insert_hdd_at_startup(x86_nxvm_machine *session,
    const char *path, lib_storage_medium_mode mode)
{
    char candidate[sizeof(session->fixed_disk_image_path[0u])];
    lib_status status;

    if (session == LIB_NULL || !session->construction.hdc_present)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_machine_copy_path(candidate, sizeof(candidate), path);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_machine_hdd_insert(session->fixed_disk[0u], candidate, mode);
    if (status != LIB_STATUS_OK) return status;
    if (session->construction.fixed_geometry) {
        status = x86_nxvm_machine_hdd_set_geometry(session->fixed_disk[0u],
            session->construction.cylinders, session->construction.heads,
            session->construction.sectors);
        if (status != LIB_STATUS_OK) return status;
    }
    lib_memory_copy(session->fixed_disk_image_path[0u], candidate, lib_text_length(candidate) + 1u);
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_storage_initialize(x86_nxvm_machine *machine)
{
    x86_nxvm_board_plan_topology topology = {0};
    lib_status status;

    if (machine == LIB_NULL || machine->x86_core_machine != LIB_NULL) {
        return LIB_STATUS_INVALID_STATE;
    }
    status = x86_nxvm_board_plan_create(&machine->construction.core_config,
        &machine->x86_nxvm_board_plan);
    if (status != LIB_STATUS_OK) return status;
    status = x86_nxvm_board_plan_set_controller_timing_rules(
        machine->x86_nxvm_board_plan,
        &machine->construction.timing_rules);
    if (status != LIB_STATUS_OK) return status;
    status = x86_nxvm_board_media_registry_create(&machine->media_registry);
    if (status != LIB_STATUS_OK) {
        x86_nxvm_machine_storage_finalize(machine);
        return status;
    }
    status = x86_nxvm_board_display_provider_slot_create(&machine->display_provider);
    if (status != LIB_STATUS_OK) {
        x86_nxvm_machine_storage_finalize(machine);
        return status;
    }
    if (machine->construction.display_snapshot_provider != LIB_NULL) {
        x86_nxvm_board_display_provider_slot_bind(machine->display_provider,
            machine->construction.display_snapshot_context,
            machine->construction.display_snapshot_provider);
        x86_nxvm_board_display_provider_slot_freeze(machine->display_provider);
    } else {
        x86_nxvm_machine_bind_display(machine);
    }
    topology = machine->construction.topology;
    status = x86_nxvm_machine_apply_cmos_seed(machine, &topology);
    if (status != LIB_STATUS_OK) { x86_nxvm_machine_storage_finalize(machine); return status; }
    topology.display.text_glyphs = machine->construction.text_glyphs;
    status = x86_nxvm_board_plan_set_topology(machine->x86_nxvm_board_plan, &topology);
    if (status == LIB_STATUS_OK && machine->construction.floppy_slot_count != 0u) {
        status = x86_nxvm_board_plan_bind_media_registry(machine->x86_nxvm_board_plan,
            machine->media_registry);
    }
    if (status == LIB_STATUS_OK) {
        status = x86_nxvm_board_plan_bind_display_provider(machine->x86_nxvm_board_plan,
            machine->display_provider);
    }
    if (status != LIB_STATUS_OK) {
        x86_nxvm_machine_storage_finalize(machine);
        return status;
    }
    status = machine->construction.profile.configure == LIB_NULL ? LIB_STATUS_OK :
        machine->construction.profile.configure(machine->construction.profile.context,
            machine->x86_nxvm_board_plan);
    if (status != LIB_STATUS_OK) {
        x86_nxvm_machine_storage_finalize(machine);
        return status;
    }
    status = x86_nxvm_board_create_from_plan(machine->x86_nxvm_board_plan,
        &machine->x86_core_machine, &machine->board);
    if (status == LIB_STATUS_OK && machine->construction.floppy_slot_count != 0u) {
        status = x86_nxvm_board_get_fdc_dma_request_binding(machine->board,
            &machine->fdc_dma_request);
    }
    if (status != LIB_STATUS_OK) {
        x86_nxvm_machine_storage_finalize(machine);
        return status;
    }
    machine->display_generation = 0u;
    return LIB_STATUS_OK;
}

void x86_nxvm_machine_storage_finalize(x86_nxvm_machine *machine)
{
    if (machine == LIB_NULL) return;
    x86_core_machine_destroy(machine->x86_core_machine);
    machine->x86_core_machine = LIB_NULL;
    if (machine->x86_nxvm_board_plan != LIB_NULL &&
        machine->construction.profile.notify != LIB_NULL)
        machine->construction.profile.notify(machine->construction.profile.context,
            X86_NXVM_MACHINE_PROFILE_BOARD_DETACHED);
    machine->board = LIB_NULL;
    x86_nxvm_board_display_provider_slot_destroy(machine->display_provider);
    machine->display_provider = LIB_NULL;
    x86_nxvm_board_media_registry_destroy(machine->media_registry);
    machine->media_registry = LIB_NULL;
    x86_nxvm_board_plan_destroy(machine->x86_nxvm_board_plan);
    machine->x86_nxvm_board_plan = LIB_NULL;
}

lib_status x86_nxvm_machine_create(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_construction *construction, x86_nxvm_machine **out_session)
{
    x86_nxvm_machine *session;
    lib_status status;

    if (out_session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_session = LIB_NULL;
    if (config == LIB_NULL || construction == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    session = (x86_nxvm_machine *)lib_allocate_zero(1u, sizeof(*session));
    if (session == LIB_NULL) {
        if (construction->profile.release != LIB_NULL)
            construction->profile.release(construction->profile.context);
        return LIB_STATUS_NO_MEMORY;
    }
    session->construction = *construction;
    if (construction->floppy_slot_count > X86_NXVM_MACHINE_FLOPPY_SLOT_COUNT ||
        (construction->floppy_slot_count != 0u &&
         x86_nxvm_board_floppy_geometry_get(construction->media_kind) == LIB_NULL) ||
        (config->floppy_image[0u] != LIB_NULL && construction->floppy_slot_count == 0u) ||
        (construction->fixed_geometry && (!construction->hdc_present ||
            construction->cylinders == 0u || construction->heads == 0u ||
            construction->sectors == 0u)) ||
        (config->floppy_image[1u] != LIB_NULL && construction->floppy_slot_count < 2u)) {
        x86_nxvm_machine_destroy(session); return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!construction->hdc_present &&
        (x86_nxvm_machine_config_fixed_disk(config, 0u) != LIB_NULL || config->create_hdd_cylinders != 0u)) {
        x86_nxvm_machine_destroy(session); return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = x86_nxvm_machine_initialize(session);
    if (status != LIB_STATUS_OK) { x86_nxvm_machine_destroy(session); return LIB_STATUS_NO_MEMORY; }
    for (lib_size slot = 0u; slot < construction->floppy_slot_count; ++slot) {
        if (x86_nxvm_machine_config_floppy(config, slot) == LIB_NULL) continue;
        status = x86_nxvm_machine_insert_floppy_at(session, slot,
            x86_nxvm_machine_config_floppy(config, slot),
            x86_nxvm_machine_config_floppy_mode(config, slot));
        if (status != LIB_STATUS_OK) { x86_nxvm_machine_destroy(session); return status; }
    }
    if (x86_nxvm_machine_config_fixed_disk(config, 0u) != LIB_NULL) {
        status = x86_nxvm_machine_insert_hdd_at_startup(session,
            x86_nxvm_machine_config_fixed_disk(config, 0u),
            x86_nxvm_machine_config_fixed_disk_mode(config, 0u));
        if (status != LIB_STATUS_OK) { x86_nxvm_machine_destroy(session); return status; }
    }
    if (config->create_fdd && construction->floppy_slot_count != 0u)
        x86_nxvm_machine_fdd_create_for(session->floppy[0u]);
    if (construction->hdc_present && config->create_hdd_cylinders != 0u) {
        status = x86_nxvm_machine_hdd_create(session->fixed_disk[0u], config->create_hdd_cylinders);
        if (status != LIB_STATUS_OK) { x86_nxvm_machine_destroy(session); return status; }
    }
    status = x86_nxvm_machine_reset(session);
    if (status != LIB_STATUS_OK) { x86_nxvm_machine_destroy(session); return status; }
    *out_session = session;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_reconfigure_memory(x86_nxvm_machine *session,
    lib_size memory_bytes)
{
    lib_status status;

    if (session == LIB_NULL || !session->construction.memory_reconfigurable ||
        (session->executor != LIB_NULL && emulator_machine_state_get(
            session->executor) != EMULATOR_MACHINE_STOPPED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    status = x86_core_machine_reconfigure_memory(session->x86_core_machine, memory_bytes);
    if (status != LIB_STATUS_OK) return status;
    x86_nxvm_machine_debug_reset(&session->debug);
    return x86_nxvm_machine_publish_display(session, LIB_TRUE);
}

void x86_nxvm_machine_destroy(x86_nxvm_machine *session)
{
    if (session == LIB_NULL) return;
    x86_nxvm_machine_finalize(session);
    if (session->construction.profile.release != LIB_NULL)
        session->construction.profile.release(session->construction.profile.context);
    lib_release(session);
}

lib_status x86_nxvm_machine_get_reset_vector(const x86_nxvm_machine *session,
    x86_nxvm_machine_reset_vector *out_vector)
{
    x86_core_machine_observation observation;
    lib_status status;

    if (session == LIB_NULL || out_vector == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (session->x86_core_machine == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    status = x86_core_machine_capture_observation(session->x86_core_machine, &observation);
    if (status != LIB_STATUS_OK) return status;
    out_vector->cs = observation.cpu.cs;
    out_vector->ip = (lib_u16)observation.cpu.eip;
    return LIB_STATUS_OK;
}
