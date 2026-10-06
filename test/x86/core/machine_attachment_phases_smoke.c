#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86/core/machine.h"

typedef struct board_phase_probe {
    core_machine *machine;
    lib_u32 calls[4];
    lib_u32 reset_phase;
    lib_bool failed;
} board_phase_probe;

static void board_phase_reset_devices(void *owner)
{
    board_phase_probe *probe = owner;
    core_machine *machine = probe->machine;
    ++probe->calls[0];
    probe->failed |= probe->reset_phase != 0u ||
        machine->elapsed_ticks != 17u;
    probe->reset_phase = 1u;
}

static void board_phase_reset_clocks(void *owner)
{
    board_phase_probe *probe = owner;
    core_machine *machine = probe->machine;
    ++probe->calls[1];
    probe->failed |= probe->reset_phase != 1u ||
        machine->elapsed_ticks != 0u;
    probe->reset_phase = 2u;
}

static void board_phase_refresh_nmi(void *owner)
{
    board_phase_probe *probe = owner;
    core_machine *machine = probe->machine;
    ++probe->calls[2];
    probe->failed |= core_machine_cpu_nmi_is_masked(machine->executor_cpu_execution);
}

static void board_phase_finalize_devices(void *owner)
{
    board_phase_probe *probe = owner;
    core_machine *machine = probe->machine;
    ++probe->calls[3];
    probe->failed |= machine->executor_cpu_execution == LIB_NULL ||
        machine->firmware_provider != LIB_NULL;
}

static lib_i32 verify_board_phases(const core_machine_executor_config *config)
{
    core_machine *machine = LIB_NULL;
    board_phase_probe probe = {0};
    board_phase_probe rejected = {0};
    core_machine_attachment binding = {
        .reset_devices = board_phase_reset_devices,
        .reset_clocks = board_phase_reset_clocks,
        .refresh_nmi = board_phase_refresh_nmi,
        .finalize_devices = board_phase_finalize_devices,
        .context = &probe
    };

    if (core_machine_neutral_create(config, &machine) !=
            LIB_STATUS_OK) return 1;
    if (core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_STATE ||
        machine->attachment.context != LIB_NULL) probe.failed = LIB_TRUE;
    core_machine_destroy(machine);
    if (core_machine_neutral_create(config, &machine) !=
            LIB_STATUS_OK) return 1;
    probe.machine = machine;
    if (core_machine_bind_attachment(machine, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_bind_attachment(LIB_NULL, &binding) != LIB_STATUS_INVALID_ARGUMENT)
        probe.failed = LIB_TRUE;
    binding.context = LIB_NULL;
    if (core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_ARGUMENT ||
        machine->attachment.context != LIB_NULL) probe.failed = LIB_TRUE;
    binding.context = &probe;
    binding.finalize_devices = LIB_NULL;
    if (core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_ARGUMENT ||
        machine->attachment.context != LIB_NULL) probe.failed = LIB_TRUE;
    binding.finalize_devices = board_phase_finalize_devices;
    if (core_machine_bind_attachment(machine, &binding) != LIB_STATUS_OK)
        probe.failed = LIB_TRUE;
    /* Core owns the copy, not the caller's binding storage. */
    binding.context = &rejected;
    binding.reset_devices = LIB_NULL;
    if (core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_STATE ||
        machine->attachment.context != &probe ||
        machine->attachment.reset_devices != board_phase_reset_devices)
        probe.failed = LIB_TRUE;
    machine->elapsed_ticks = 17u;
    if (core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_STATE ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_set_nmi_mask(machine, 1) != LIB_STATUS_OK ||
        probe.calls[2] != 0u ||
        core_machine_set_nmi_mask(machine, 0) != LIB_STATUS_OK) probe.failed = LIB_TRUE;
    core_machine_destroy(machine);
    return probe.failed || probe.reset_phase != 2u || probe.calls[0] != 1u ||
        probe.calls[1] != 1u || probe.calls[2] != 1u || probe.calls[3] != 1u ||
        rejected.calls[0] != 0u || rejected.calls[3] != 0u;
}

typedef struct deadline_probe {
    lib_u64 due, l1_until;
    lib_bool blocked;
    lib_u32 finalized;
} deadline_probe;

static void probe_deadline(void *owner, lib_u64 now, lib_bool qualified,
    core_machine_attachment_deadline_observation *out)
{
    deadline_probe *probe = owner;
    (void)qualified;
    *out = (core_machine_attachment_deadline_observation){
        .source_ticks = probe->due > now ? probe->due - now : 0u,
        .immediate_due = probe->due <= now,
        .l1_compatibility = now < probe->l1_until,
        .fast_advance_blocked = probe->blocked
    };
}
static void deadline_finalize(void *owner) { ++((deadline_probe *)owner)->finalized; }

static lib_i32 verify_deadline_admission(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .l1_compatibility_policy = CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS
    };
    deadline_probe probe = {.due = 5u, .l1_until = 3u};
    const core_machine_attachment binding = {.deadline = probe_deadline,
        .finalize_devices = deadline_finalize, .context = &probe};
    core_machine *machine = LIB_NULL;
    core_machine_time_observation observed;
    lib_u8 advanced = LIB_FALSE;
    lib_u64 ticks = 0u;
    lib_i32 failed = 1;
    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_bind_attachment(machine, &binding) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_capture_time_observation(machine, &observed) != LIB_STATUS_OK ||
        observed.progress_disposition != CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY || observed.next_deadline_valid ||
        core_machine_advance_l1_compatibility(machine, &advanced) != LIB_STATUS_OK || !advanced ||
        core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 3u) goto done;
    probe.blocked = LIB_TRUE;
    if (core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK || advanced ||
        core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 3u) goto done;
    probe.blocked = LIB_FALSE;
    if (core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK || !advanced ||
        core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 5u) goto done;
    probe.l1_until = probe.due = LIB_UINT64_MAX;
    /* The existing 16-step control bound is not an emulated device duration. */
    if (core_machine_advance_l1_compatibility(machine, &advanced) != LIB_STATUS_OK || !advanced ||
        core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 21u) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed || probe.finalized != 1u;
}

lib_i32 main(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .ticks_per_instruction = 1u
    };
    if (verify_board_phases(&config) || verify_deadline_admission()) return 1;
    lib_c_printf("%s\n", "M5:T540:S93:CORE-ATTACHMENT-PHASES:OK");
    return 0;
}
