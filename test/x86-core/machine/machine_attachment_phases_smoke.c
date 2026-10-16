#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86-core/machine/machine.h"

typedef struct board_phase_probe {
    x86_core_machine *machine;
    lib_u32 calls[4];
    lib_u32 reset_phase;
    lib_bool failed;
} board_phase_probe;

static void board_phase_reset_devices(void *owner)
{
    board_phase_probe *probe = owner;
    x86_core_machine *machine = probe->machine;
    ++probe->calls[0];
    probe->failed |= probe->reset_phase != 0u ||
        machine->elapsed_ticks != 17u;
    probe->reset_phase = 1u;
}

static void board_phase_reset_clocks(void *owner)
{
    board_phase_probe *probe = owner;
    x86_core_machine *machine = probe->machine;
    ++probe->calls[1];
    probe->failed |= probe->reset_phase != 1u ||
        machine->elapsed_ticks != 0u;
    probe->reset_phase = 2u;
}

static void board_phase_refresh_nmi(void *owner)
{
    board_phase_probe *probe = owner;
    x86_core_machine *machine = probe->machine;
    ++probe->calls[2];
    probe->failed |= x86_core_cpu_nmi_is_masked(machine->executor_cpu_execution);
}

static void board_phase_finalize_devices(void *owner)
{
    board_phase_probe *probe = owner;
    x86_core_machine *machine = probe->machine;
    ++probe->calls[3];
    probe->failed |= machine->executor_cpu_execution == LIB_NULL ||
        machine->firmware_provider != LIB_NULL;
}

static lib_i32 verify_board_phases(const x86_core_machine_executor_config *config)
{
    x86_core_machine *machine = LIB_NULL;
    board_phase_probe probe = {0};
    board_phase_probe rejected = {0};
    x86_core_machine_attachment binding = {
        .reset_devices = board_phase_reset_devices,
        .reset_clocks = board_phase_reset_clocks,
        .refresh_nmi = board_phase_refresh_nmi,
        .finalize_devices = board_phase_finalize_devices,
        .context = &probe
    };

    if (x86_core_machine_neutral_create(config, &machine) !=
            LIB_STATUS_OK) return 1;
    if (x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_STATE ||
        machine->attachment.context != LIB_NULL) probe.failed = LIB_TRUE;
    x86_core_machine_destroy(machine);
    if (x86_core_machine_neutral_create(config, &machine) !=
            LIB_STATUS_OK) return 1;
    probe.machine = machine;
    if (x86_core_machine_bind_attachment(machine, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_core_machine_bind_attachment(LIB_NULL, &binding) != LIB_STATUS_INVALID_ARGUMENT)
        probe.failed = LIB_TRUE;
    binding.context = LIB_NULL;
    if (x86_core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_ARGUMENT ||
        machine->attachment.context != LIB_NULL) probe.failed = LIB_TRUE;
    binding.context = &probe;
    binding.finalize_devices = LIB_NULL;
    if (x86_core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_ARGUMENT ||
        machine->attachment.context != LIB_NULL) probe.failed = LIB_TRUE;
    binding.finalize_devices = board_phase_finalize_devices;
    if (x86_core_machine_bind_attachment(machine, &binding) != LIB_STATUS_OK)
        probe.failed = LIB_TRUE;
    /* Core owns the copy, not the caller's binding storage. */
    binding.context = &rejected;
    binding.reset_devices = LIB_NULL;
    if (x86_core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_STATE ||
        machine->attachment.context != &probe ||
        machine->attachment.reset_devices != board_phase_reset_devices ||
        machine->attachment.reset_clocks != board_phase_reset_clocks ||
        machine->attachment.refresh_nmi != board_phase_refresh_nmi ||
        machine->attachment.finalize_devices != board_phase_finalize_devices)
        probe.failed = LIB_TRUE;
    machine->elapsed_ticks = 17u;
    if (x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_bind_attachment(machine, &binding) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_set_nmi_mask(machine, 1) != LIB_STATUS_OK ||
        probe.calls[2] != 0u ||
        x86_core_machine_set_nmi_mask(machine, 0) != LIB_STATUS_OK) probe.failed = LIB_TRUE;
    x86_core_machine_destroy(machine);
    return probe.failed || probe.reset_phase != 2u || probe.calls[0] != 1u ||
        probe.calls[1] != 1u || probe.calls[2] != 1u || probe.calls[3] != 1u ||
        rejected.calls[0] != 0u || rejected.calls[3] != 0u;
}

typedef struct deadline_probe {
    lib_u64 due, l1_until;
    lib_bool blocked;
    lib_u32 finalized;
} deadline_probe;

static void probe_deadline(void *owner, lib_u64 now,
    x86_core_machine_attachment_deadline_observation *out)
{
    deadline_probe *probe = owner;
    *out = (x86_core_machine_attachment_deadline_observation){
        .source_ticks = probe->due > now ? probe->due - now : 0u,
        .immediate_due = probe->due <= now,
        .l1_compatibility = now < probe->l1_until,
        .fast_advance_blocked = probe->blocked
    };
}
static void deadline_finalize(void *owner) { ++((deadline_probe *)owner)->finalized; }

static lib_i32 verify_deadline_admission(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_8086,
        .l1_compatibility_policy = X86_CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS
    };
    deadline_probe probe = {.due = 5u, .l1_until = 3u};
    const x86_core_machine_attachment binding = {.deadline = probe_deadline,
        .finalize_devices = deadline_finalize, .context = &probe};
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_time_observation observed;
    lib_u8 advanced = LIB_FALSE;
    lib_u64 ticks = 0u;
    lib_i32 failed = 1;
    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_core_machine_bind_attachment(machine, &binding) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_capture_time_observation(machine, &observed) != LIB_STATUS_OK ||
        observed.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY || observed.next_deadline_valid ||
        x86_core_machine_advance_l1_compatibility(machine, &advanced) != LIB_STATUS_OK || !advanced ||
        x86_core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 3u) goto done;
    probe.blocked = LIB_TRUE;
    if (x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK || advanced ||
        x86_core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 3u) goto done;
    probe.blocked = LIB_FALSE;
    if (x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK || !advanced ||
        x86_core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 5u) goto done;
    probe.l1_until = probe.due = LIB_UINT64_MAX;
    /* The existing 16-step control bound is not an emulated device duration. */
    if (x86_core_machine_advance_l1_compatibility(machine, &advanced) != LIB_STATUS_OK || !advanced ||
        x86_core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 21u) goto done;
    failed = 0;
done:
    x86_core_machine_destroy(machine);
    return failed || probe.finalized != 1u;
}

typedef struct internal_advance_probe {
    x86_core_machine *machine;
    lib_u64 pending_ticks;
    lib_u32 before_calls, after_calls, finalized;
    lib_bool failed;
} internal_advance_probe;

static void internal_advance_before(void *owner, lib_u64 ticks, lib_u64 due)
{
    internal_advance_probe *probe = owner;
    lib_u64 pending = 0u;

    ++probe->before_calls;
    probe->failed |= probe->after_calls != 0u ||
        ticks != probe->pending_ticks || due != probe->pending_ticks ||
        x86_fpu_ticks_until_completion(probe->machine->fpu, &pending) !=
            LIB_STATUS_OK || pending != probe->pending_ticks;
}

static void internal_advance_after(void *owner, lib_u64 ticks, lib_u64 due)
{
    internal_advance_probe *probe = owner;

    ++probe->after_calls;
    probe->failed |= probe->before_calls != 1u ||
        ticks != probe->pending_ticks || due != probe->pending_ticks ||
        x86_fpu_ticks_until_completion(probe->machine->fpu, &(lib_u64){0u}) !=
            LIB_STATUS_INVALID_STATE;
}

static void internal_advance_finalize(void *owner)
{
    ++((internal_advance_probe *)owner)->finalized;
}

static lib_i32 verify_internal_advance_boundary(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_80387
    };
    internal_advance_probe probe = {0};
    const x86_core_machine_attachment attachment = {
        .before_internal_advance = internal_advance_before,
        .after_internal_advance = internal_advance_after,
        .finalize_devices = internal_advance_finalize,
        .context = &probe
    };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = 1;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    probe.machine = machine;
    if (x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    x86_fpu_begin_command(machine->fpu, 0xd8u, 0xc0u);
    if (x86_fpu_ticks_until_completion(machine->fpu, &probe.pending_ticks) !=
            LIB_STATUS_OK || probe.pending_ticks == 0u ||
        x86_core_machine_publish_elapsed_ticks(machine, probe.pending_ticks,
            X86_CORE_MACHINE_TIME_PUBLICATION_DEADLINE) != LIB_STATUS_OK) goto done;
    failed = probe.failed || probe.before_calls != 1u || probe.after_calls != 1u;
done:
    x86_core_machine_destroy(machine);
    return failed || probe.finalized != 1u;
}

static lib_i32 verify_explicit_time_contract(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_timeline_observation timeline;
    lib_u64 elapsed = 1u;
    lib_i32 failed = 1;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        return 1;
    if (x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_INVALID_STATE ||
        machine->elapsed_ticks != 0u ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_advance_time(machine, 0u) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_core_machine_advance_time(machine, 5u) != LIB_STATUS_OK ||
        x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != 5u ||
        x86_core_machine_get_timeline_observation(machine, &timeline) != LIB_STATUS_OK ||
        timeline.now != 5u ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != 0u ||
        x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
        x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != 1u) goto done;
    machine->elapsed_ticks = LIB_UINT64_MAX;
    failed = x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        machine->elapsed_ticks != LIB_UINT64_MAX;
done:
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_8086,
        .ticks_per_instruction = 1u
    };
    if (verify_board_phases(&config) || verify_deadline_admission() ||
        verify_internal_advance_boundary() || verify_explicit_time_contract()) return 1;
    lib_c_printf("%s\n", "CORE-ATTACHMENT-PHASES:OK");
    return 0;
}
