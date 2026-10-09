#include "lib/types/types_interface.h"

#include "core/machine/machine_private.h"

#include "core/board-base/machine_board_interface.h"
#include "core/machine/control.h"
#include "core/machine/lifecycle.h"
#include "core/machine/display.h"
#include "core/machine/media/media_interface.h"
#include "core/machine/machine_devices.h"
#include "core/machine/media/fdd_interface.h"
#include "core/machine/media/hdd_interface.h"
#include "core/machine/keyboard_mapper_interface.h"
#include "core/machine/mouse_mapper_interface.h"

static lib_status vm_machine_insert_floppy_at(vm_machine *session, lib_size slot,
    const char *path, lib_storage_medium_mode mode);
static lib_status vm_machine_remove_fdd_direct(vm_machine *session);

static lib_status vm_machine_deliver_key(vm_machine *session,
    const kvm_input_event *event)
{
    lib_status status = LIB_STATUS_OK;

    if (session == LIB_NULL || !session->active) return LIB_STATUS_INVALID_ARGUMENT;
    {
        vm_profile_default_keyboard_sequence sequence;
        lib_u8 native_scan_set;

        if (core_machine_keyboard_get_native_scan_set(session->board,
                &native_scan_set) == LIB_STATUS_OK &&
            vm_profile_default_keyboard_map_kvm_event_for_scan_set(
                event,
                native_scan_set, &sequence) ==
            LIB_STATUS_OK) {
            status = core_machine_keyboard_receive_native_bytes(session->board,
                sequence.bytes, sequence.count);
        }
    }
    return status;
}

static lib_status vm_machine_deliver_mouse(vm_machine *session,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons)
{
    if (session == LIB_NULL || !session->active) return LIB_STATUS_INVALID_ARGUMENT;
    {
        vm_profile_default_mouse_report report;

        if (vm_profile_default_mouse_map_host_relative(
                delta_x, delta_y, buttons, &report) ==
            LIB_STATUS_OK) {
            return core_machine_mouse_receive_relative(session->board,
                report.delta_x, report.delta_y, report.buttons);
        }
    }
    return LIB_STATUS_OK;
}

lib_status vm_machine_deliver_emulator_input(vm_machine *session,
    const kvm_input_event *event)
{
    if (event == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (event->type == KVM_EVENT_KEY) return vm_machine_deliver_key(session, event);
    if (event->type == KVM_EVENT_MOUSE) return vm_machine_deliver_mouse(session,
        event->data.mouse.delta_x, event->data.mouse.delta_y,
        event->data.mouse.buttons);
    return LIB_STATUS_UNSUPPORTED;
}

lib_status vm_machine_copy_emulator_frame(vm_machine *machine, emulator_machine_frame *frame)
{
    lib_status status;

    if (machine == LIB_NULL || frame == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_machine_publish_display(machine, LIB_FALSE);
    if (status != LIB_STATUS_OK) return status;
    if (!machine->latest_frame_valid) return LIB_STATUS_OK;
    return emulator_machine_frame_copy(frame, &machine->latest_frame) ?
        LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

static lib_status vm_machine_copy_path(char *destination, lib_size capacity,
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

static const char *vm_machine_config_floppy(const vm_machine_config *config,
    lib_size slot)
{
    if (config == LIB_NULL || slot >= VM_MACHINE_FLOPPY_SLOT_COUNT) return LIB_NULL;
    return config->floppy_image[slot];
}

static const char *vm_machine_config_fixed_disk(const vm_machine_config *config,
    lib_size slot)
{
    if (config == LIB_NULL || slot >= VM_MACHINE_FIXED_DISK_SLOT_COUNT) return LIB_NULL;
    return config->fixed_disk_image[slot];
}

static lib_storage_medium_mode vm_machine_config_floppy_mode(
    const vm_machine_config *config, lib_size slot)
{
    return config != LIB_NULL && slot < VM_MACHINE_FLOPPY_SLOT_COUNT ?
        config->floppy_mode[slot] : LIB_STORAGE_MEDIUM_OVERLAY;
}

static lib_storage_medium_mode vm_machine_config_fixed_disk_mode(
    const vm_machine_config *config, lib_size slot)
{
    return config != LIB_NULL && slot < VM_MACHINE_FIXED_DISK_SLOT_COUNT ?
        config->fixed_disk_mode[slot] : LIB_STORAGE_MEDIUM_OVERLAY;
}

lib_status vm_machine_apply_cmos_seed(const vm_machine *session,
    core_machine_plan_topology *topology)
{
    lib_size index;

    if (session == LIB_NULL || topology == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!session->construction.cmos_seed_present) return LIB_STATUS_OK;
    if (!topology->rtc_cmos_present) return LIB_STATUS_INVALID_STATE;
    /* A seed is a board configuration image, not a replacement RTC state:
     * copy only the manual-defined CMOS NVRAM 0Eh--3Fh.  It is the sole
     * owner of board configuration, including the firmware checksum. */
    for (index = 0u; index < CORE_MACHINE_RTC_DEFAULT_CAPACITY; ++index) {
        topology->rtc_cmos.defaults[index] = (core_machine_rtc_default_byte) {
            (lib_u8)(0x0eu + index), session->construction.cmos_seed[0x0eu + index] };
    }
    topology->rtc_cmos.default_count = CORE_MACHINE_RTC_DEFAULT_CAPACITY;
    topology->rtc_cmos.derive_configuration_checksum = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status vm_machine_get_speed(const vm_machine *session,
    vm_machine_speed *out_speed)
{
    if (session == LIB_NULL || !session->active || out_speed == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_speed = session->speed;
    return LIB_STATUS_OK;
}

lib_status vm_machine_set_speed(vm_machine *session, vm_machine_speed speed)
{
    if (session == LIB_NULL || !session->active ||
        (speed != VM_MACHINE_SPEED_STANDARD && speed != VM_MACHINE_SPEED_TURBO)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (vm_machine_control_is_running(&session->control)) return LIB_STATUS_INVALID_STATE;
    session->speed = speed;
    return LIB_STATUS_OK;
}

static lib_status vm_machine_insert_floppy_at(vm_machine *session, lib_size slot,
    const char *path, lib_storage_medium_mode mode)
{
    char candidate[sizeof(session->floppy_image_path[0])];
    lib_status status;

    if (session == LIB_NULL || slot >= session->construction.floppy_slot_count)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (vm_machine_control_is_running(&session->control)) return LIB_STATUS_INVALID_STATE;
    status = vm_machine_copy_path(candidate, sizeof(candidate), path);
    if (status == LIB_STATUS_OK)
        status = vm_machine_fdd_insert_for(session->floppy[slot], candidate, mode);
    if (status != LIB_STATUS_OK) return status;
    lib_memory_copy(session->floppy_image_path[slot], candidate,
        lib_text_length(candidate) + 1u);
    return LIB_STATUS_OK;
}

static lib_status vm_machine_remove_fdd_direct(vm_machine *session)
{
    lib_status status;

    if (session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (vm_machine_control_is_running(&session->control)) return LIB_STATUS_INVALID_STATE;
    status = vm_machine_fdd_remove_for(session->floppy[0u]);
    if (status == LIB_STATUS_OK) session->floppy_image_path[0u][0] = '\0';
    return status;
}

lib_status vm_machine_set_emulator_media(vm_machine *session, const char *path,
    lib_storage_medium_mode mode)
{
    if (session == LIB_NULL || !session->active) return LIB_STATUS_INVALID_STATE;
    if (path != LIB_NULL && path[0] != '\0')
        return vm_machine_insert_floppy_at(session, 0u, path, mode);
    return vm_machine_remove_fdd_direct(session);
}

lib_i32 vm_machine_insert_fdd(vm_machine *session, const char *path)
{ return session != LIB_NULL && session->executor != LIB_NULL &&
    emulator_machine_set_removable_media(session->executor, path,
        LIB_STORAGE_MEDIUM_OVERLAY) ? 0 : -1; }

lib_i32 vm_machine_eject_fdd(vm_machine *session)
{ return session != LIB_NULL && session->executor != LIB_NULL &&
    emulator_machine_set_removable_media(session->executor, LIB_NULL,
        LIB_STORAGE_MEDIUM_OVERLAY) ? 0 : -1; }
static lib_status vm_machine_insert_hdd_at_startup(vm_machine *session,
    const char *path, lib_storage_medium_mode mode)
{
    char candidate[sizeof(session->fixed_disk_image_path[0u])];
    lib_status status;

    if (session == LIB_NULL || !session->construction.hdc_present)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_machine_copy_path(candidate, sizeof(candidate), path);
    if (status == LIB_STATUS_OK)
        status = vm_machine_hdd_insert(session->fixed_disk[0u], candidate, mode);
    if (status != LIB_STATUS_OK) return status;
    if (session->construction.fixed_geometry) {
        status = vm_machine_hdd_set_geometry(session->fixed_disk[0u],
            session->construction.cylinders, session->construction.heads,
            session->construction.sectors);
        if (status != LIB_STATUS_OK) return status;
    }
    lib_memory_copy(session->fixed_disk_image_path[0u], candidate, lib_text_length(candidate) + 1u);
    return LIB_STATUS_OK;
}

lib_status vm_machine_storage_initialize(vm_machine *machine)
{
    core_machine_plan_topology topology = {0};
    lib_status status;

    if (machine == LIB_NULL || machine->core_machine != LIB_NULL) {
        return LIB_STATUS_INVALID_STATE;
    }
    status = core_machine_plan_create(&machine->construction.core_config,
        &machine->core_machine_plan);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_plan_set_controller_timing_rules(
        machine->core_machine_plan,
        &machine->construction.timing_rules);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_media_registry_create(&machine->media_registry);
    if (status != LIB_STATUS_OK) {
        vm_machine_storage_finalize(machine);
        return status;
    }
    status = core_machine_display_provider_slot_create(&machine->display_provider);
    if (status != LIB_STATUS_OK) {
        vm_machine_storage_finalize(machine);
        return status;
    }
    vm_machine_bind_display(machine);
    topology = machine->construction.topology;
    status = vm_machine_apply_cmos_seed(machine, &topology);
    if (status != LIB_STATUS_OK) { vm_machine_storage_finalize(machine); return status; }
    topology.display.text_glyphs = machine->construction.text_glyphs;
    status = core_machine_plan_set_topology(machine->core_machine_plan, &topology);
    if (status == LIB_STATUS_OK) {
        status = core_machine_plan_bind_media_registry(machine->core_machine_plan,
            machine->media_registry);
    }
    if (status == LIB_STATUS_OK) {
        status = core_machine_plan_bind_display_provider(machine->core_machine_plan,
            machine->display_provider);
    }
    if (status != LIB_STATUS_OK) {
        vm_machine_storage_finalize(machine);
        return status;
    }
    status = machine->construction.profile.configure == LIB_NULL ? LIB_STATUS_OK :
        machine->construction.profile.configure(machine->construction.profile.context,
            machine->core_machine_plan);
    if (status != LIB_STATUS_OK) {
        vm_machine_storage_finalize(machine);
        return status;
    }
    status = core_machine_create_from_plan(machine->core_machine_plan,
        &machine->core_machine, &machine->board);
    if (status == LIB_STATUS_OK) {
        status = core_machine_get_fdc_dma_request_binding(machine->board,
            &machine->fdc_dma_request);
    }
    if (status != LIB_STATUS_OK) {
        vm_machine_storage_finalize(machine);
        return status;
    }
    machine->display_generation = 0u;
    return LIB_STATUS_OK;
}

void vm_machine_storage_finalize(vm_machine *machine)
{
    if (machine == LIB_NULL) return;
    core_machine_destroy(machine->core_machine);
    machine->core_machine = LIB_NULL;
    if (machine->core_machine_plan != LIB_NULL &&
        machine->construction.profile.notify != LIB_NULL)
        machine->construction.profile.notify(machine->construction.profile.context,
            VM_MACHINE_PROFILE_BOARD_DETACHED);
    machine->board = LIB_NULL;
    core_machine_display_provider_slot_destroy(machine->display_provider);
    machine->display_provider = LIB_NULL;
    core_machine_media_registry_destroy(machine->media_registry);
    machine->media_registry = LIB_NULL;
    core_machine_plan_destroy(machine->core_machine_plan);
    machine->core_machine_plan = LIB_NULL;
}

lib_status vm_machine_create(const vm_machine_config *config,
    const vm_machine_construction *construction, vm_machine **out_session)
{
    vm_machine *session;
    lib_status status;

    if (out_session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_session = LIB_NULL;
    if (config == LIB_NULL || construction == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    session = (vm_machine *)lib_allocate_zero(1u, sizeof(*session));
    if (session == LIB_NULL) {
        if (construction->profile.release != LIB_NULL)
            construction->profile.release(construction->profile.context);
        return LIB_STATUS_NO_MEMORY;
    }
    session->construction = *construction;
    if (construction->floppy_slot_count == 0u ||
        construction->floppy_slot_count > VM_MACHINE_FLOPPY_SLOT_COUNT ||
        (construction->fixed_geometry && (!construction->hdc_present ||
            construction->cylinders == 0u || construction->heads == 0u ||
            construction->sectors == 0u)) ||
        (config->floppy_image[1u] != LIB_NULL && construction->floppy_slot_count < 2u)) {
        vm_machine_destroy(session); return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!construction->hdc_present &&
        (vm_machine_config_fixed_disk(config, 0u) != LIB_NULL || config->create_hdd_cylinders != 0u)) {
        vm_machine_destroy(session); return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = vm_machine_initialize(session);
    if (status != LIB_STATUS_OK) { vm_machine_destroy(session); return status; }
    for (lib_size slot = 0u; slot < construction->floppy_slot_count; ++slot) {
        if (vm_machine_config_floppy(config, slot) == LIB_NULL) continue;
        status = vm_machine_insert_floppy_at(session, slot,
            vm_machine_config_floppy(config, slot),
            vm_machine_config_floppy_mode(config, slot));
        if (status != LIB_STATUS_OK) { vm_machine_destroy(session); return status; }
    }
    if (vm_machine_config_fixed_disk(config, 0u) != LIB_NULL) {
        status = vm_machine_insert_hdd_at_startup(session,
            vm_machine_config_fixed_disk(config, 0u),
            vm_machine_config_fixed_disk_mode(config, 0u));
        if (status != LIB_STATUS_OK) { vm_machine_destroy(session); return status; }
    }
    if (config->create_fdd) vm_machine_fdd_create_for(session->floppy[0u]);
    if (construction->hdc_present && config->create_hdd_cylinders != 0u) {
        status = vm_machine_hdd_create(session->fixed_disk[0u], config->create_hdd_cylinders);
        if (status != LIB_STATUS_OK) { vm_machine_destroy(session); return status; }
    }
    status = vm_machine_reset(session);
    if (status != LIB_STATUS_OK) { vm_machine_destroy(session); return status; }
    *out_session = session;
    return LIB_STATUS_OK;
}

lib_status vm_machine_reconfigure_memory(vm_machine *session,
    lib_size memory_bytes)
{
    lib_status status;

    if (session == LIB_NULL || !session->construction.memory_reconfigurable ||
        (session->executor != LIB_NULL && emulator_machine_state_get(
            session->executor) != EMULATOR_MACHINE_STOPPED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    status = core_machine_reconfigure_memory(session->core_machine, memory_bytes);
    if (status != LIB_STATUS_OK) return status;
    vm_machine_debug_reset(&session->debug);
    return vm_machine_publish_display(session, LIB_TRUE);
}

void vm_machine_destroy(vm_machine *session)
{
    if (session == LIB_NULL) return;
    vm_machine_finalize(session);
    if (session->construction.profile.release != LIB_NULL)
        session->construction.profile.release(session->construction.profile.context);
    lib_release(session);
}

lib_status vm_machine_get_reset_vector(const vm_machine *session,
    vm_machine_reset_vector *out_vector)
{
    core_machine_observation observation;
    lib_status status;

    if (session == LIB_NULL || out_vector == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (session->core_machine == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    status = core_machine_capture_observation(session->core_machine, &observation);
    if (status != LIB_STATUS_OK) return status;
    out_vector->cs = observation.cpu.cs;
    out_vector->ip = (lib_u16)observation.cpu.eip;
    return LIB_STATUS_OK;
}
