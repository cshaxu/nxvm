#include "lib/types/types_interface.h"

#include "app-nxvm/machine/machine_private.h"

#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/control.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/display.h"
#include "app-nxvm/machine/media/media.h"
#include "app-nxvm/machine/machine_devices.h"
#include "app-nxvm/machine/media/fdd.h"
#include "app-nxvm/machine/media/hdd.h"
#include "app-nxvm/profiles/default_profile/keyboard_mapper.h"
#include "app-nxvm/profiles/default_profile/mouse_mapper.h"

static lib_i32 vm_machine_insert_floppy_at(vm_machine *session, lib_size slot,
    const char *path, lib_storage_medium_mode mode);
static lib_i32 vm_machine_remove_fdd_direct(vm_machine *session);

static void vm_machine_capture_fdc_terminal(void *opaque,
    const core_machine_fdc_terminal_observation *observation)
{
    vm_machine *machine = (vm_machine *)opaque;

    if (machine == LIB_NULL || observation == LIB_NULL) return;
    machine->model40_fdc_terminal_observation = *observation;
    machine->model40_fdc_terminal_observation_valid = LIB_TRUE;
}

static lib_status vm_machine_deliver_key(vm_machine *session,
    lib_u16 scan_code, lib_u16 virtual_key, lib_i32 pressed)
{
    lib_status status = LIB_STATUS_OK;

    if (session == LIB_NULL || !session->active) return LIB_STATUS_INVALID_ARGUMENT;
    {
        vm_profile_default_keyboard_sequence sequence;
        lib_u8 native_scan_set;

        if (core_machine_keyboard_get_native_scan_set(session->core_machine,
                &native_scan_set) == LIB_STATUS_OK &&
            vm_profile_default_keyboard_map_host_key_for_scan_set(
                scan_code, virtual_key, pressed,
                native_scan_set, &sequence) ==
            LIB_STATUS_OK) {
            status = core_machine_keyboard_receive_native_bytes(session->core_machine,
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
            return core_machine_mouse_receive_relative(session->core_machine,
                report.delta_x, report.delta_y, report.buttons);
        }
    }
    return LIB_STATUS_OK;
}

lib_status vm_machine_deliver_common_input(vm_machine *session,
    const kvm_input_event *event)
{
    if (event == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (event->type == KVM_EVENT_KEY) return vm_machine_deliver_key(session,
        event->data.key.scan_code, (lib_u16)event->data.key.key,
        event->data.key.pressed != 0u);
    if (event->type == KVM_EVENT_MOUSE) return vm_machine_deliver_mouse(session,
        event->data.mouse.delta_x, event->data.mouse.delta_y,
        event->data.mouse.buttons);
    return LIB_STATUS_UNSUPPORTED;
}

lib_u8 vm_machine_copy_common_frame(vm_machine *machine, common_machine_frame *frame)
{
    if (machine == LIB_NULL || frame == LIB_NULL) return LIB_FALSE;
    (void)vm_machine_publish_display(machine, LIB_FALSE);
    if (!machine->latest_frame_valid) return LIB_FALSE;
    return common_machine_frame_copy(frame, &machine->latest_frame);
}

static lib_i32 vm_machine_copy_path(char *destination, lib_size capacity,
    const char *source)
{
    lib_size length;

    if (destination == LIB_NULL || capacity == 0u) return 0;
    destination[0] = '\0';
    if (source == LIB_NULL) return 1;
    length = lib_text_length(source);
    if (length >= capacity) return 0;
    lib_memory_copy(destination, source, length + 1u);
    return 1;
}

lib_status vm_machine_submit_host_input(vm_machine *session,
    const core_machine_guest_input_event *event)
{
    kvm_input_event input = {0};

    if (session == LIB_NULL || !session->active) return LIB_STATUS_INVALID_STATE;
    if (event == LIB_NULL || (event->kind != CORE_MACHINE_GUEST_INPUT_KEY &&
        event->kind != CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (event->kind == CORE_MACHINE_GUEST_INPUT_KEY) {
        input.type = KVM_EVENT_KEY;
        input.data.key.scan_code = event->data.key.scan_code;
        input.data.key.key = event->data.key.virtual_key;
        input.data.key.pressed = event->data.key.pressed;
    } else {
        input.type = KVM_EVENT_MOUSE;
        input.data.mouse.delta_x = event->data.relative_mouse.delta_x;
        input.data.mouse.delta_y = event->data.relative_mouse.delta_y;
        input.data.mouse.buttons = event->data.relative_mouse.buttons;
    }
    /* A composed product has exactly one executor-side Core mutation path.
     * The Common FIFO serializes host input with its bounded safe points.
     * An explicitly uncomposed deterministic Core loop retains direct
     * owner-local delivery. */
    if (session->executor != LIB_NULL) {
        return common_machine_enqueue_input(session->executor, &input) ?
            LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
    }
    /* The uncomposed test ingress retains the prior source contract: a valid
     * copied host record was accepted even when this board has no receiver
     * for that optional class (for example AUX input on Model 40). */
    (void)vm_machine_deliver_common_input(session, &input);
    return LIB_STATUS_OK;
}

lib_status vm_machine_submit_input(vm_machine *session,
    const vm_machine_input *input)
{
    core_machine_guest_input_event event = {0};

    if (input == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (input->kind == VM_MACHINE_INPUT_KEY_EVENT) {
        event.kind = CORE_MACHINE_GUEST_INPUT_KEY;
        event.data.key.scan_code = input->data.key_event.scan_code;
        event.data.key.virtual_key = input->data.key_event.virtual_key;
        event.data.key.pressed = input->data.key_event.pressed;
    } else if (input->kind == VM_MACHINE_INPUT_MOUSE_EVENT) {
        event.kind = CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE;
        event.data.relative_mouse.delta_x = input->data.mouse_event.delta_x;
        event.data.relative_mouse.delta_y = input->data.mouse_event.delta_y;
        event.data.relative_mouse.buttons = input->data.mouse_event.buttons;
    } else return LIB_STATUS_INVALID_ARGUMENT;
    return vm_machine_submit_host_input(session, &event);
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
    if (!session->cmos_seed_present) return LIB_STATUS_OK;
    if (!topology->rtc_cmos_present) return LIB_STATUS_INVALID_STATE;
    /* A seed is a board configuration image, not a replacement RTC state:
     * copy only the manual-defined CMOS NVRAM 0Eh--3Fh.  It is the sole
     * owner of board configuration, including the firmware checksum. */
    for (index = 0u; index < CORE_MACHINE_RTC_DEFAULT_CAPACITY; ++index) {
        topology->rtc_cmos.defaults[index] = (core_machine_rtc_default_byte) {
            (lib_u8)(0x0eu + index), session->cmos_seed[0x0eu + index] };
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

static lib_i32 vm_machine_insert_floppy_at(vm_machine *session, lib_size slot,
    const char *path, lib_storage_medium_mode mode)
{
    char candidate[sizeof(session->floppy_image_path[slot])];

    if (session == LIB_NULL || session->profile_plan == LIB_NULL ||
        slot >= vm_profile_machine_plan_floppy_slot_count(session->profile_plan) ||
        vm_machine_control_is_running(&session->control) ||
        mode > LIB_STORAGE_MEDIUM_OVERLAY ||
        !vm_machine_copy_path(candidate, sizeof(candidate), path) ||
        vm_machine_fdd_insert_for(&session->floppy[slot], candidate, mode) != 0 ||
        !vm_machine_copy_path(session->floppy_image_path[slot],
            sizeof(session->floppy_image_path[slot]),
            candidate)) return -1;
    session->retained_config.floppy_image[slot] = session->floppy_image_path[slot];
    session->retained_config.floppy_mode[slot] = mode;
    return 0;
}

static lib_i32 vm_machine_remove_fdd_direct(vm_machine *session)
{
    if (session == LIB_NULL || vm_machine_control_is_running(&session->control) ||
        vm_machine_fdd_remove_for(&session->fdd) != 0) return -1;
    session->fdd_image_path[0] = '\0';
    session->retained_config.floppy_image[0u] = LIB_NULL;
    return 0;
}

lib_status vm_machine_set_common_media(vm_machine *session, const char *path,
    lib_storage_medium_mode mode)
{
    if (session == LIB_NULL || !session->active) return LIB_STATUS_INVALID_STATE;
    if (path != LIB_NULL && path[0] != '\0')
        return vm_machine_insert_floppy_at(session, 0u, path, mode) == 0 ?
            LIB_STATUS_OK : LIB_STATUS_INTERNAL_ERROR;
    return vm_machine_remove_fdd_direct(session) == 0 ?
        LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
}

lib_i32 vm_machine_insert_fdd(vm_machine *session, const char *path)
{ return session != LIB_NULL && session->executor != LIB_NULL &&
    common_machine_set_removable_media(session->executor, path,
        LIB_STORAGE_MEDIUM_OVERLAY) ? 0 : -1; }

lib_i32 vm_machine_eject_fdd(vm_machine *session)
{ return session != LIB_NULL && session->executor != LIB_NULL &&
    common_machine_set_removable_media(session->executor, LIB_NULL,
        LIB_STORAGE_MEDIUM_OVERLAY) ? 0 : -1; }
static lib_i32 vm_machine_insert_hdd_at_startup(vm_machine *session,
    const char *path, lib_storage_medium_mode mode)
{
    char candidate[sizeof(session->hdd_image_path)];

    lib_u16 cylinders = 0u;
    lib_u8 heads = 0u, sectors = 0u;

    if (session == LIB_NULL || session->profile_plan == LIB_NULL ||
        !vm_profile_machine_plan_hdc_present(session->profile_plan) || !vm_machine_copy_path(candidate,
            sizeof(candidate), path) || mode > LIB_STORAGE_MEDIUM_OVERLAY ||
        vm_machine_hdd_insert(&session->hdd, candidate, mode) != 0 ||
        !vm_machine_copy_path(session->hdd_image_path, sizeof(session->hdd_image_path),
            candidate)) return -1;
    session->retained_config.fixed_disk_image[0u] = session->hdd_image_path;
    session->retained_config.fixed_disk_mode[0u] = mode;
    if (vm_profile_machine_plan_hdd_geometry_get(session->profile_plan,
            &cylinders, &heads, &sectors)) {
        const lib_size expected_bytes = (lib_size)cylinders * heads * sectors * 512u;
        if (vm_machine_hdd_raw_byte_count(&session->hdd) != expected_bytes ||
            vm_machine_hdd_set_geometry(&session->hdd, cylinders, heads, sectors) != LIB_FALSE) {
            return -1;
        }
    }
    return 0;
}

lib_status vm_machine_storage_initialize(vm_machine *machine)
{
    core_machine_plan_topology topology = {0};
    lib_status status;

    if (machine == LIB_NULL || machine->core_machine != LIB_NULL) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (machine->profile_plan == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_plan_create(&machine->core_machine_config,
        &machine->core_machine_plan);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_plan_set_controller_timing_rules(
        machine->core_machine_plan, &machine->controller_timing_rules);
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
    topology = *vm_profile_machine_plan_topology_get(machine->profile_plan);
    status = vm_machine_apply_cmos_seed(machine, &topology);
    if (status != LIB_STATUS_OK) { vm_machine_storage_finalize(machine); return status; }
    topology.display.text_glyphs = machine->text_glyphs;
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
    status = vm_profile_machine_plan_materialize(machine->profile_plan,
        machine->core_machine_plan,
        (core_machine_fdc_terminal_observation_provider) {
            vm_machine_capture_fdc_terminal, machine });
    if (status != LIB_STATUS_OK) {
        vm_machine_storage_finalize(machine);
        return status;
    }
    status = core_machine_create_from_plan(machine->core_machine_plan,
        &machine->core_machine);
    if (status == LIB_STATUS_OK) {
        status = core_machine_get_fdc_dma_request_binding(machine->core_machine,
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
    core_machine_display_provider_slot_destroy(machine->display_provider);
    machine->display_provider = LIB_NULL;
    core_machine_media_registry_destroy(machine->media_registry);
    machine->media_registry = LIB_NULL;
    core_machine_plan_destroy(machine->core_machine_plan);
    machine->core_machine_plan = LIB_NULL;
}

static lib_status vm_machine_create_from_plan(const vm_machine_config *config,
    vm_profile_machine_plan *plan, vm_machine **out_session)
{
    vm_machine *session;
    lib_status status;

    if (out_session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_session = LIB_NULL;
    if (config == LIB_NULL || plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    session = (vm_machine *)lib_allocate_zero(1u, sizeof(*session));
    if (session == LIB_NULL) {
        vm_profile_machine_plan_destroy(plan);
        return LIB_STATUS_NO_MEMORY;
    }
    session->profile_plan = plan;
    session->core_machine_config = *vm_profile_machine_plan_core_config_get(session->profile_plan);
    session->controller_timing_rules = *vm_profile_machine_plan_timing_rules_get(session->profile_plan);
    session->floppy_kind = vm_profile_machine_plan_drive_floppy_get(session->profile_plan);
    session->fdd_media_kind = vm_profile_machine_plan_media_floppy_get(session->profile_plan);
    if (!vm_profile_machine_plan_hdc_present(session->profile_plan) &&
        (vm_machine_config_fixed_disk(config, 0u) != LIB_NULL || config->create_hdd_cylinders != 0u)) {
        vm_machine_destroy(session); return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (vm_profile_machine_plan_copy_cmos_seed(session->profile_plan, session->cmos_seed,
            &session->cmos_seed_present) != LIB_STATUS_OK ||
        vm_profile_machine_plan_copy_text_glyphs(session->profile_plan,
            &session->text_glyphs) != LIB_STATUS_OK) { vm_machine_destroy(session); return LIB_STATUS_INVALID_ARGUMENT; }
    session->retained_config = *config;
    session->retained_config.cmos_seed = LIB_NULL;
    session->retained_config.bios_path[0u] = LIB_NULL;
    session->retained_config.bios_path[1u] = LIB_NULL;
    session->retained_config.video_path = LIB_NULL;
    session->retained_config.font_path = LIB_NULL;
    status = vm_machine_initialize(session);
    if (status != LIB_STATUS_OK) { vm_machine_destroy(session); return status; }
    if ((vm_machine_config_floppy(config, 0u) != LIB_NULL &&
            vm_machine_insert_floppy_at(session, 0u, vm_machine_config_floppy(config, 0u),
                vm_machine_config_floppy_mode(config, 0u))) ||
        (vm_machine_config_floppy(config, 1u) != LIB_NULL &&
            vm_machine_insert_floppy_at(session, 1u, vm_machine_config_floppy(config, 1u),
                vm_machine_config_floppy_mode(config, 1u))) ||
        (vm_machine_config_fixed_disk(config, 0u) != LIB_NULL &&
            vm_machine_insert_hdd_at_startup(session,
                vm_machine_config_fixed_disk(config, 0u),
                vm_machine_config_fixed_disk_mode(config, 0u)))) {
        vm_machine_destroy(session);
        return LIB_STATUS_INTERNAL_ERROR;
    }
    if (config->create_fdd) vm_machine_fdd_create_for(&session->fdd);
    if (vm_profile_machine_plan_hdc_present(session->profile_plan) &&
        config->create_hdd_cylinders != 0u &&
        vm_machine_hdd_create(&session->hdd, config->create_hdd_cylinders) != LIB_FALSE) {
        vm_machine_destroy(session);
        return LIB_STATUS_NO_MEMORY;
    }
    status = vm_machine_reset(session);
    if (status != LIB_STATUS_OK) { vm_machine_destroy(session); return status; }
    *out_session = session;
    return LIB_STATUS_OK;
}

lib_status vm_machine_create_from_assets(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine **out_session)
{
    vm_profile_machine_plan *plan = LIB_NULL;
    lib_status status;

    if (out_session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_session = LIB_NULL;
    status = vm_profile_machine_plan_create(config, assets, &plan);
    if (status != LIB_STATUS_OK) return status;
    return vm_machine_create_from_plan(config, plan, out_session);
}

lib_status vm_machine_create(const vm_machine_config *config,
    vm_machine **out_session)
{
    vm_profile_machine_plan *plan = LIB_NULL;
    lib_status status;

    if (out_session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_session = LIB_NULL;
    if (config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_profile_machine_plan_create_file_backed(config, &plan);
    if (status != LIB_STATUS_OK) return status;
    return vm_machine_create_from_plan(config, plan, out_session);
}

lib_status vm_machine_reconfigure_memory(vm_machine *session,
    lib_size memory_bytes)
{
    lib_status status;

    if (session == LIB_NULL || !vm_profile_machine_plan_memory_reconfigurable(
            session->profile_plan) ||
        (session->executor != LIB_NULL && common_machine_state_get(
            session->executor) != COMMON_MACHINE_STOPPED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    status = core_machine_reconfigure_memory(session->core_machine, memory_bytes);
    if (status != LIB_STATUS_OK) return status;
    session->retained_config.memory_bytes = memory_bytes;
    session->core_machine_config.memory_bytes = memory_bytes;
    vm_machine_debug_reset(&session->debug);
    vm_machine_publish_display(session, 1);
    return LIB_STATUS_OK;
}

void vm_machine_destroy(vm_machine *session)
{
    if (session == LIB_NULL) return;
    vm_machine_finalize(session);
    vm_profile_machine_plan_destroy(session->profile_plan);
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
