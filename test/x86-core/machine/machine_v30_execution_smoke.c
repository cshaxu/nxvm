#include "lib/types/file.h"
#include "x86-core/machine/debug_interface.h"

typedef struct v30_execution_probe {
    lib_u32 retired, native_entries, emulation_entries, finalized;
    lib_u32 reads, writes, value;
    lib_u64 advanced, read_tick;
    lib_status device_status;
    lib_u32 acknowledgements;
    lib_bool interrupt, timer_armed;
} v30_execution_probe;

static void execution_retired(void *opaque,
    const x86_core_machine_retirement_observation *observation)
{
    v30_execution_probe *probe = opaque;

    ++probe->retired;
    if (observation->instruction_entry_cpu.eflags & 0x8000u)
        ++probe->native_entries;
    else
        ++probe->emulation_entries;
}

static void execution_advance(void *opaque, lib_u64 ticks, lib_u64 due)
{
    v30_execution_probe *probe = opaque;

    (void)due;
    probe->advanced += ticks;
    probe->timer_armed = LIB_FALSE;
}

static void execution_deadline(void *opaque, lib_u64 now,
    x86_core_machine_attachment_deadline_observation *observation)
{
    (void)now;
    *observation = (x86_core_machine_attachment_deadline_observation){
        .source_ticks = ((v30_execution_probe *)opaque)->timer_armed ? 7u : 0u};
}

static void execution_finalize(void *opaque)
{
    ++((v30_execution_probe *)opaque)->finalized;
}

static lib_bool execution_interrupt(void *opaque)
{
    return ((v30_execution_probe *)opaque)->interrupt;
}

static lib_u8 execution_acknowledge(void *opaque)
{
    v30_execution_probe *probe = opaque;

    ++probe->acknowledgements;
    probe->interrupt = LIB_FALSE;
    return 0x30u;
}

static lib_status execution_read(void *opaque, lib_u16 port, lib_u64 tick,
    lib_u32 *value)
{
    v30_execution_probe *probe = opaque;

    if (port != 0x55u) return LIB_STATUS_INVALID_ARGUMENT;
    ++probe->reads;
    probe->read_tick = tick;
    if (probe->device_status != LIB_STATUS_OK) return probe->device_status;
    *value = probe->value;
    return LIB_STATUS_OK;
}

static lib_status execution_write(void *opaque, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    v30_execution_probe *probe = opaque;

    (void)access_bytes;
    if (port != 0x55u) return LIB_STATUS_INVALID_ARGUMENT;
    ++probe->writes;
    if (probe->device_status != LIB_STATUS_OK) return probe->device_status;
    probe->value = value;
    return LIB_STATUS_OK;
}

/* Guest bytes enter each mode through its real instruction. Public paused
 * register writes select an entry, never change a profile or NEC private latch. */
static lib_i32 execution_modes(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 0x100000u, .cpu_profile = X86_CORE_CPU_PROFILE_V30};
    const lib_u8 native[] = {0x0fu, 0xffu, 0x80u, 0xf4u};
    const lib_u8 emulation[] = {
        0x31u, 0x00u, 0x70u,       /* LXI SP,7000 */
        0x3eu, 0x3cu,              /* MVI A,3c */
        0xd3u, 0x55u, 0xdbu, 0x55u, /* OUT 55; IN 55 */
        0xedu, 0xedu, 0x81u,       /* CALLN 81 */
        0x3cu, 0xedu, 0xfdu};      /* INR A; RETEM */
    const lib_u8 handler[] = {0x46u, 0xcfu}; /* native INC SI; IRET */
    const lib_u8 vectors[] = {0u, 5u, 0u, 0u, 0u, 3u, 0u, 0u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x400u,
            [X86_CORE_CPU_DEBUG_ESP] = 0x6000u}};
    v30_execution_probe probe = {0};
    const x86_core_machine_attachment attachment = {
        .context = &probe, .after_internal_advance = execution_advance,
        .finalize_devices = execution_finalize};
    const x86_core_machine_port_route route = {
        .address = 0x55u, .read = execution_read, .write = execution_write,
        .owner = &probe};
    const x86_core_machine_retirement_observation_provider observer = {
        execution_retired, &probe};
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot cpu;
    const char *phase = "construct";
    lib_i32 failed = 1;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        return 1;
    if (x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
        x86_core_machine_install_port_routes(machine, &route, 1u) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_set_retirement_observation_provider(machine, &observer) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x400u, native, sizeof(native)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x500u, emulation, sizeof(emulation)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x300u, handler, sizeof(handler)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x200u, vectors, sizeof(vectors)) != LIB_STATUS_OK)
        goto done;
    phase = "guest modes";
    if (x86_core_machine_debug_continue(machine,
            (x86_core_machine_run_budget){32u, 0u}, &result) != LIB_STATUS_OK ||
        result.executed != 11u || result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        probe.retired != 11u || probe.native_entries != 4u || probe.emulation_entries != 7u ||
        probe.reads != 1u || probe.writes != 1u || probe.value != 0x3cu ||
        probe.advanced != result.elapsed_ticks || probe.read_tick > probe.advanced ||
        x86_core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK ||
        cpu.eip != 0x404u || cpu.esi != 1u || cpu.esp != 0x6000u ||
        cpu.ebp != 0x7000u || (cpu.eax & 0xffu) != 0x3du || !(cpu.eflags & 0x8000u))
        goto done;
    phase = "observational paused read";
    const lib_u64 elapsed = result.elapsed_ticks;
    lib_u8 byte = 0u;
    if (x86_core_machine_debug_read_memory(machine, 0x500u, &byte, 1u) != LIB_STATUS_OK ||
        byte != 0x31u || probe.reads != 1u || probe.writes != 1u ||
        probe.advanced != elapsed || probe.retired != 11u) goto done;
    phase = "reset";
    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK ||
        cpu.cs.selector != 0xffffu || cpu.cs.base != 0xffff0u || cpu.eip != 0u ||
        !(cpu.eflags & 0x8000u))
        goto done;
    failed = 0;
done:
    if (failed) lib_c_printf("V30:MACHINE:FAIL %s retired=%u native=%u emulation=%u pc=%x\n",
        phase, probe.retired, probe.native_entries, probe.emulation_entries, result.linear_pc);
    x86_core_machine_destroy(machine);
    return failed || probe.finalized != 1u;
}

/* HALT wake must use the attached interrupt or NMI input, with a real native
 * handler and IRET restoring the interrupted mode. No private mode mutation. */
static lib_i32 execution_interrupts(lib_bool emulation, lib_bool nmi)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 0x100000u, .cpu_profile = X86_CORE_CPU_PROFILE_V30};
    const lib_u8 native[] = {0xfbu, 0x90u, 0xf4u, 0x40u, 0xf4u};
    const lib_u8 bridge[] = {0x0fu, 0xffu, 0x80u, 0xf4u};
    const lib_u8 emulated[] = {0xfbu, 0x76u, 0x3cu, 0x76u};
    const lib_u8 handler[] = {0x46u, 0xcfu};
    const lib_u8 handler_vector[] = {0u, 3u, 0u, 0u};
    const lib_u8 emulated_vector[] = {0u, 5u, 0u, 0u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x400u,
            [X86_CORE_CPU_DEBUG_ESP] = 0x6000u}};
    v30_execution_probe probe = {0};
    const x86_core_machine_attachment attachment = {
        .context = &probe, .interrupt_pending = execution_interrupt,
        .interrupt_acknowledge = execution_acknowledge,
        .deadline = execution_deadline,
        .after_internal_advance = execution_advance,
        .finalize_devices = execution_finalize};
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot cpu;
    const char *phase = "construct";
    lib_i32 failed = 1;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        return 1;
    if (x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x400u,
            emulation ? bridge : native, emulation ? sizeof(bridge) : sizeof(native)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x500u, emulated, sizeof(emulated)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x300u, handler, sizeof(handler)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x200u, emulated_vector, sizeof(emulated_vector)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, nmi ? 8u : 0xc0u,
            handler_vector, sizeof(handler_vector)) != LIB_STATUS_OK) goto done;
    phase = "halt";
    if (x86_core_machine_run(machine, (x86_core_machine_run_budget){3u, 0u},
            &result) != LIB_STATUS_OK || result.executed != 3u ||
        result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) goto done;
    phase = "halt deadline";
    const lib_u64 halted_at = result.elapsed_ticks;
    lib_u8 advanced = LIB_FALSE;
    probe.timer_armed = LIB_TRUE;
    if (x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK ||
        !advanced || probe.timer_armed || probe.advanced != halted_at + 7u ||
        x86_core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK ||
        cpu.eip != (emulation ? 0x502u : 0x403u) ||
        ((cpu.eflags & 0x8000u) != 0u) == emulation) goto done;
    if (nmi) {
        if (!x86_core_machine_signal_nmi(machine)) goto done;
    } else {
        probe.interrupt = LIB_TRUE;
    }
    phase = "entry";
    if (x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
            &result) != LIB_STATUS_OK || result.executed != 0u ||
        probe.acknowledgements != (nmi ? 0u : 1u) ||
        x86_core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK ||
        cpu.eip != 0x300u || !(cpu.eflags & 0x8000u)) goto done;
    phase = "handler return";
    if (x86_core_machine_run(machine, (x86_core_machine_run_budget){2u, 0u},
            &result) != LIB_STATUS_OK || result.executed != 2u ||
        x86_core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK ||
        cpu.eip != (emulation ? 0x502u : 0x403u) || cpu.esi != 1u ||
        ((cpu.eflags & 0x8000u) != 0u) == emulation) goto done;
    phase = "continue";
    if (x86_core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
        result.executed != 1u ||
        x86_core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK ||
        (cpu.eax & 0xffu) != 1u || probe.advanced != result.elapsed_ticks) goto done;
    failed = 0;
done:
    if (failed) lib_c_printf("V30:MACHINE:INTERRUPT:FAIL %s emulation=%u nmi=%u pc=%x\n",
        phase, emulation, nmi, result.linear_pc);
    x86_core_machine_destroy(machine);
    return failed || probe.finalized != 1u;
}

static lib_status execution_query(void *opaque, lib_u32 address, lib_uptr bytes,
    x86_core_machine_memory_access access)
{
    (void)opaque;
    (void)access;
    return address == 0x7000u && bytes == 1u ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status execution_memory_read(void *opaque, lib_u32 address,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    v30_execution_probe *probe = opaque;

    if (address != 0x7000u || bytes != 1u) return LIB_STATUS_INVALID_ARGUMENT;
    if (!observe_only) {
        ++probe->reads;
        if (probe->device_status != LIB_STATUS_OK) return probe->device_status;
    }
    *(lib_u8 *)destination = (lib_u8)probe->value;
    return LIB_STATUS_OK;
}

static lib_status execution_memory_write(void *opaque, lib_u32 address,
    lib_uptr source, lib_uptr bytes)
{
    v30_execution_probe *probe = opaque;

    if (address != 0x7000u || bytes != 1u) return LIB_STATUS_INVALID_ARGUMENT;
    ++probe->writes;
    if (probe->device_status != LIB_STATUS_OK) return probe->device_status;
    probe->value = *(const lib_u8 *)source;
    return LIB_STATUS_OK;
}

/* Accept or refuse the same real guest device operation. Reset must recover
 * without replaying a rejected operation or retiring it as success. */
static lib_i32 execution_devices(lib_bool emulation, lib_bool memory,
    lib_bool write, lib_bool refuse)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 0x100000u, .cpu_profile = X86_CORE_CPU_PROFILE_V30};
    const lib_u8 bridge[] = {0x0fu, 0xffu, 0x80u};
    const lib_u8 vector[] = {0u, 5u, 0u, 0u};
    const lib_u8 halt = emulation ? 0x76u : 0xf4u;
    lib_u8 code[] = {0u, memory ? 0u : 0x55u, 0x70u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = emulation ? 0x400u : 0x500u,
            [X86_CORE_CPU_DEBUG_ESP] = 0x6000u}};
    v30_execution_probe probe = {
        .device_status = refuse ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK, .value = 0x5au};
    const x86_core_machine_attachment attachment = {
        .context = &probe, .after_internal_advance = execution_advance,
        .finalize_devices = execution_finalize};
    const x86_core_machine_port_route port_route = {
        .address = 0x55u, .read = execution_read, .write = execution_write, .owner = &probe};
    const x86_core_machine_memory_device_route memory_route = {
        .physical_start = 0x7000u, .bytes = 1u,
        .callbacks = {execution_memory_read, execution_memory_write, execution_query},
        .failure = LIB_NULL};
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    const char *phase = "construct";
    lib_i32 failed = 1;

    code[0] = memory ? (emulation ? (write ? 0x32u : 0x3au) : (write ? 0xa2u : 0xa0u)) :
        (emulation ? (write ? 0xd3u : 0xdbu) : (write ? 0xe6u : 0xe4u));
    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    if (x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
        x86_core_machine_install_port_routes(machine, &port_route, 1u) != LIB_STATUS_OK ||
        x86_core_machine_install_memory_device_routes(machine, &memory_route, 1u,
            LIB_NULL, LIB_NULL, &probe) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x400u, bridge, sizeof(bridge)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x500u, code, memory ? 3u : 2u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x200u, vector, sizeof(vector)) != LIB_STATUS_OK)
        goto done;
    phase = "enter";
    if (emulation && (x86_core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
            result.executed != 1u)) goto done;
    phase = "device result";
    const lib_u64 before = probe.advanced;
    if (x86_core_machine_debug_step(machine, &result) !=
            (refuse ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
        result.reason != (refuse ? X86_CORE_MACHINE_STOP_FAULT : X86_CORE_MACHINE_STOP_BUDGET) ||
        result.executed != (refuse ? 0u : 1u) ||
        result.elapsed_ticks <= before || probe.advanced != result.elapsed_ticks ||
        probe.reads != (write ? 0u : 1u) || probe.writes != (write ? 1u : 0u) ||
        probe.value != (!refuse && write ? 0u : 0x5au)) goto done;
    if (!refuse && !write) {
        lib_u32 accumulator = 0u;

        if (x86_core_machine_debug_read_register(machine, X86_CORE_CPU_DEBUG_EAX,
                &accumulator) != LIB_STATUS_OK || (accumulator & 0xffu) != 0x5au)
            goto done;
    }
    phase = "observational device read";
    lib_u8 value = 0u;
    if (x86_core_machine_memory_inspect(machine, 0x7000u, &value, 1u) != LIB_STATUS_OK ||
        value != (!refuse && write ? 0u : 0x5au) ||
        probe.reads != (write ? 0u : 1u)) goto done;
    phase = "reset recovery";
    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x400u, bridge, sizeof(bridge)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x500u, &halt, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x200u, vector, sizeof(vector)) != LIB_STATUS_OK ||
        x86_core_machine_debug_continue(machine,
            (x86_core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        result.executed != (emulation ? 2u : 1u) ||
        probe.reads != (write ? 0u : 1u) || probe.writes != (write ? 1u : 0u)) goto done;
    failed = 0;
done:
    if (failed) lib_c_printf("V30:MACHINE:DEVICE:FAIL %s emulation=%u memory=%u write=%u refuse=%u ticks=%llu reason=%u\n",
        phase, emulation, memory, write, refuse, result.elapsed_ticks, (lib_u32)result.reason);
    x86_core_machine_destroy(machine);
    return failed || probe.finalized != 1u;
}

/* A one-tick budget can leave an accepted instruction awaiting completion.
 * Reset/stop owns cancellation; paused debugger writes cannot bypass it. */
static lib_i32 execution_boundaries(lib_bool stop)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 0x100000u, .cpu_profile = X86_CORE_CPU_PROFILE_V30};
    const lib_u8 code[] = {0x40u, 0x90u, 0xf4u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x400u}};
    v30_execution_probe probe = {0};
    const x86_core_machine_attachment attachment = {
        .context = &probe, .finalize_devices = execution_finalize};
    const x86_core_machine_retirement_observation_provider observer = {
        execution_retired, &probe};
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_machine_lifecycle lifecycle;
    x86_core_cpu_debug_cpu_snapshot cpu;
    lib_i32 failed = 1;
    const char *phase = "construction";

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    if (x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_set_retirement_observation_provider(machine, &observer) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x400u, code, sizeof(code)) != LIB_STATUS_OK)
        goto done;
    phase = "pending completion";
    if (x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 1u},
            &result) != LIB_STATUS_OK || result.ticks != 1u || result.executed != 0u ||
        probe.retired != 0u ||
        x86_core_machine_debug_write_register(machine, X86_CORE_CPU_DEBUG_EAX,
            0x1234u) != LIB_STATUS_INVALID_STATE) goto done;
    phase = "cancel";
    if (stop) {
        if (x86_core_machine_request_stop(machine) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_REQUESTED ||
            result.executed != 0u) goto done;
    } else if (x86_core_machine_reset(machine) != LIB_STATUS_OK) {
        goto done;
    }
    if (x86_core_machine_get_lifecycle(machine, &lifecycle) != LIB_STATUS_OK ||
        lifecycle != X86_CORE_MACHINE_STOPPED || probe.retired != 0u ||
        x86_core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK ||
        cpu.cs.base != 0xffff0u || cpu.eip != 0u || cpu.eax != 0u) goto done;
    phase = "clean continuation";
    if (x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x400u, code, sizeof(code)) != LIB_STATUS_OK ||
        x86_core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
        result.executed != 1u || probe.retired != 1u ||
        x86_core_machine_debug_capture_cpu_snapshot(machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK || cpu.eax != 1u ||
        x86_core_machine_get_lifecycle(machine, &lifecycle) != LIB_STATUS_OK ||
        lifecycle != X86_CORE_MACHINE_PAUSED) goto done;
    failed = 0;
done:
    if (failed) lib_c_printf("V30:MACHINE:BOUNDARY:FAIL %s stop=%u retired=%u\n",
        phase, stop, probe.retired);
    x86_core_machine_destroy(machine);
    return failed || probe.finalized != 1u;
}

lib_i32 main(void)
{
    if (execution_modes()) return 1;
    if (execution_boundaries(LIB_FALSE) || execution_boundaries(LIB_TRUE)) return 1;
    for (lib_u8 mode = 0u; mode < 2u; ++mode)
        for (lib_u8 nmi = 0u; nmi < 2u; ++nmi)
            if (execution_interrupts(mode != 0u, nmi != 0u)) return 1;
    for (lib_u8 mode = 0u; mode < 2u; ++mode)
        for (lib_u8 memory = 0u; memory < 2u; ++memory)
            for (lib_u8 write = 0u; write < 2u; ++write)
                for (lib_u8 refuse = 0u; refuse < 2u; ++refuse)
                    if (execution_devices(mode != 0u, memory != 0u,
                            write != 0u, refuse != 0u)) return 1;
    lib_c_printf("V30:MACHINE:OK\n");
    return 0;
}
