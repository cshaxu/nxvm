# M2 Machine And DOS Architecture Requirements

## Decision

This is version 1 of the Machine/DOS design. M3 creates the Machine boundary;
M4 specifies the DOS ABI; M5 implements it. No M2 decision adds runtime
behavior.

M1 proves that the imported whole machine reaches DOS code from both local boot
fixtures, but not that booted guest DOS is the product backend. The FDD trace
reached a final `INT 21h` after 469,926 records; the HDD trace reached `INT 21h`
by record 224,602. Both runs stopped at a ten-second watchdog while still making
progress. See `docs/verification/m1-t2-s1-nxvm-baseline.md`.

## Profiles And Devices

M3 supports two explicit profiles. `legacy_boot` exists solely to preserve M1
regressions. `dos_minimal` is the M5 foundation: it starts from a controlled
reset, and the DOS loader, not POST or a disk image, selects program execution.
No implicit shell, disk boot, or BIOS program loader is permitted in that
profile.

| Baseline unit | M3 disposition | Reason and M3 proof |
| --- | --- | --- |
| `vcpu`, `vram`, `vport` | retain behind `Machine` | Required for real-mode program execution, memory and I/O; CPU/memory microtests plus both M1 boot traces. |
| `vpic`, `vpit` | retain as optional core devices | Required for an explicit interrupt/timing model; disabled only by a declared profile, never by hidden global state. |
| `vkbc`, `qdx`/CGA display | retain as optional DOS-profile devices | M5 deterministic keyboard/text I/O needs abstract input and text presentation; platform code supplies events and consumes snapshots. |
| `vbios`, `vcmos`, `vdma`, `vfdc`, `vfdd`, `vhdc`, `vhdd`, `vvadp` | legacy-boot adapter only | Needed to reproduce M1 POST/disk boot, but not a dependency of `dos_minimal`. Each is initialized only by `legacy_boot`. |
| `vdebug`, `debug`, `console`, `xasm32` | developer tooling, outside `machine` | May inspect a paused machine through a read-only debug interface; it cannot own execution or mutate device globals directly. |
| `machine.c`, `platform/*` startup loops | replace | The baseline has `machine -> platform`, global `device.flagRun`, and platform-created kernel/display threads. M3 replaces these with runtime-owned composition and a synchronized command boundary. |

No unit is deleted in M3 merely because it is absent from `dos_minimal`; the
legacy profile remains until the M1 regression has a replacement baseline. A
future removal requires an approved task, a usage search, and the same M1 trace
checkpoints. Graphics, mouse, DMA consumers, disk emulation for DOS programs,
and a BIOS compatibility surface are deferred to corpus-driven M8 work.

## Module Ownership

```text
app -> runtime -> adapters -> machine
                  |             ^
                  +-> dos ------+
                  +-> platform
```

`runtime` owns one `Machine`, one selected profile, the run loop, and final
result. `machine` owns CPU, RAM, I/O dispatch, enabled devices, and mutable
execution state. `dos` owns loader state, PSP/environment and `INT 20h`/`INT
21h` dispatch; it calls only Machine and abstract HostService capabilities.
`platform` owns host threads, input collection, display presentation, clocks,
and later Windows filesystem implementation. `adapters` install DOS interrupt
handlers and translate platform events at declared synchronization points.

There is no `machine -> dos`, `machine -> platform/Windows`, `dos -> Win32`, or
`platform -> DOS internals` dependency. The M1 direct calls from platform code
to `deviceConnect*` are legacy behavior to be replaced in M3, not an allowed
new boundary.

## Lifecycle And Threading

```text
new -> initialized -> reset -> paused -> running -> paused
                                      |          |
                                      +-> stopped+
                                      +-> faulted
initialized/paused/stopped/faulted -> finalized
```

`runtime` owns lifecycle transitions and the only execution thread. `Machine`
API calls that mutate guest state are legal only on that thread, at a command
boundary before or after `machine_run`; callers on other threads enqueue an
immutable command and wait for its result. Platform input and display threads
never read or write CPU, RAM, device, or stop state directly. They exchange
timestamped input events and immutable display snapshots through `adapters`.

`machine_reset` is synchronous at a command boundary. `machine_request_stop`
is asynchronous but yields one terminal stop reason. `machine_run` accepts a
finite instruction budget and returns at a boundary; no M3 test is allowed an
unbounded loop. Faults preserve a stable fault code and last program counter;
they never call a platform UI. Timer and IRQ delivery occur only inside the
execution thread and before the next instruction boundary according to the
selected profile's deterministic tick policy.

## Versioned C Contracts

The exact header locations are an M3 implementation choice. All declarations
below carry `NTVDM64_MACHINE_ABI_V1`; future changes are append-only or require
a new ABI version. Opaque handles prevent callers from reaching NXVM globals.

```c
typedef struct ntvdm64_machine ntvdm64_machine;
typedef enum {
    NTVDM64_STOP_BUDGET, NTVDM64_STOP_PAUSED, NTVDM64_STOP_GUEST_EXIT,
    NTVDM64_STOP_REQUESTED, NTVDM64_STOP_FAULT
} ntvdm64_stop_reason;
typedef struct { uint64_t instructions; uint32_t ticks; } ntvdm64_run_budget;
typedef struct { ntvdm64_stop_reason reason; uint64_t executed;
                 uint32_t linear_pc; uint32_t detail; } ntvdm64_run_result;

int machine_create_v1(const ntvdm64_machine_config *, ntvdm64_machine **out);
int machine_reset_v1(ntvdm64_machine *);
int machine_run_v1(ntvdm64_machine *, ntvdm64_run_budget, ntvdm64_run_result *);
int machine_request_stop_v1(ntvdm64_machine *);
int machine_destroy_v1(ntvdm64_machine *);

int machine_mem_read_v1(ntvdm64_machine *, uint32_t linear, void *, size_t);
int machine_mem_write_v1(ntvdm64_machine *, uint32_t linear, const void *, size_t);
int machine_set_vector_v1(ntvdm64_machine *, uint8_t vector,
                          uint16_t segment, uint16_t offset);
int machine_install_port_v1(ntvdm64_machine *, uint16_t first, uint16_t last,
                            const ntvdm64_port_ops *, void *owner);
int machine_install_interrupt_v1(ntvdm64_machine *, uint8_t vector,
                                 const ntvdm64_interrupt_ops *, void *owner);
```

All functions return a project status enum; invalid state, invalid range,
duplicate ownership, unsupported profile feature, and guest fault are distinct
results. Memory calls are range checked and never expose a writable raw RAM
pointer. Port and interrupt callbacks execute on the Machine thread, may not
re-enter `machine_run`, and return a handled/unhandled/fault result. An
installed handler has one owner and is removed before that owner is destroyed.

DOS uses `machine_set_vector_v1` or `machine_install_interrupt_v1`; it does not
write CPU-reference macros or call legacy `deviceConnect*` functions. The
Machine never names DOS. `dos` reports guest termination through a registered
Machine-neutral stop request with an exit value; `runtime` converts it to the
future product result.

## Host Service And Trace Boundaries

M2 defines capabilities, not DOS pathname syntax or Win32 calls:

```c
typedef struct ntvdm64_host_service ntvdm64_host_service;
typedef enum { NTVDM64_HOST_OK, NTVDM64_HOST_NOT_FOUND,
    NTVDM64_HOST_DENIED, NTVDM64_HOST_INVALID, NTVDM64_HOST_IO } ntvdm64_host_status;
int host_open_v1(ntvdm64_host_service *, ntvdm64_root_id, const void *token,
                 size_t token_size, unsigned access, ntvdm64_handle_id *out);
int host_read_v1(ntvdm64_host_service *, ntvdm64_handle_id, void *, size_t,
                 size_t *read);
int host_close_v1(ntvdm64_host_service *, ntvdm64_handle_id);
```

A root capability is selected by `runtime`; an operation can affect only that
capability's exposed root. `dos` receives opaque tokens and host status classes,
not a host path, handle, drive letter, or operating-system error. M5 uses only
an in-memory fixture implementation. M6 specifies DOS path grammar,
canonicalization, reparse/race handling, Windows handles, and CLI drive policy.

Trace is the append-only, null-by-default `ntvdm64_trace_sink` V1 described in
`docs/governance/differential-debug-policy.md`. Machine emits reset, run
boundary, step, interrupt, port, IRQ, device reset, text snapshot and stop
records. DOS later emits loader/service/filesystem/exit records. Trace callbacks
receive copied event data, never a mutable Machine pointer; a full sink cannot
change guest behavior.

## M1 Observation Mapping And M3 Regression

| M1 observation | M3 owner | Required regression/checkpoint |
| --- | --- | --- |
| Reset begins at `F000:FFF0` | Machine lifecycle and legacy BIOS adapter | FDD/HDD trace records reset vector. |
| FDD inserts and CPU reaches DOS `INT 21h` | legacy disk/BIOS profile | Fixture identity plus `Floppy disk inserted.` and trace checkpoint. |
| HDD connects and CPU reaches DOS `INT 21h` | legacy disk/BIOS profile | Fixture identity plus `Hard disk connected.` and trace checkpoint. |
| CPU made progress until watchdog | Machine budget/stop state | finite instruction, wall-clock and no-progress budgets yield declared stop. |
| Console `exit` returns 0 | developer-console adapter | focused Console lifecycle test; this is not product CLI behavior. |
| Win32 console/window create separate threads and touch `device.flagRun` | Platform adapter replacement | platform code has no include or symbol reference to Machine internals; mock-input/snapshot test. |

M3 must run `cmake --preset mingw-gcc-x64` and `cmake --build --preset
m1-baseline-gcc` until its own target presets replace the baseline preset. Its
FDD/HDD regression uses the recorded M1 fixture hashes and ten-second maximum,
then compares the reset and DOS checkpoints above. It is a watchdog verdict,
not a claim that either guest exits naturally. CPU, RAM, port, reset, stop, and
profile-selection tests are added as deterministic no-media tests. Every test
sets instruction, wall-clock, and no-progress budgets.

## Deferred Decisions

M4 owns COM layout, PSP/environment/DTA, initial registers, DOS error table,
handle semantics, blocked input protocol and the approved `INT 20h`/`INT 21h`
subset. M6 owns `run` grammar, program-path mapping, host-drive and reparse
policy, Windows-version matrix, Console/window ownership, graphics switching,
debugger grammar, signals, cancellation and product exit statuses. M8 admits
further hardware or DOS features only through a corpus requirement.

## M3 Gate

M3 starts only from the bounded task/subtask breakdown in
`docs/planning/m3-machine-refactor-breakdown.md`. An uncertain device behavior,
undocumented Windows mechanism, or new DOS service remains an evidence question;
it does not enlarge M3.
