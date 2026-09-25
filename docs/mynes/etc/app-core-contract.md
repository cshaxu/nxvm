# App/Core Contract

Derived detail under [Architecture](../design/ARCHITECTURE.md) and
[Source Layout](../design/CODING.md). This is a design for owner review, not
implemented source. Hardware facts and reference priority are in
[References](hardware-references.md); product choices below are MyNes decisions.

## Scope And Assembly

M2 implements one NTSC-oriented RP2A03 CPU, its RAM/bus, mapper-0 cartridge,
cooked management and debugging. All 151 documented opcodes are included;
the other 105 bytes trap. M2 has no PPU/APU/controller device implementation.
These are final M2 requirements. The owner-approved implementation packages
deliver a truthful partial execution profile first, then complete CPU/debug and
timing/reliability coverage as mapped in the
[package proof ownership](app-core-verification.md#m2-package-proof-ownership).
Access to those unavailable devices traps explicitly, so this is an execution
foundation, not a claim to run commercial games. The full product's device,
graphics/text, input and audio boundaries are fixed in
[Hardware And Presentation](hardware-presentation.md). That design admits no
later milestone tasks. Save states, rewind, networking, PAL/Dendy and expansion
peripherals remain outside the approved product scope, not unfinished M2 work.

Construction order is App config/provider storage -> Core adapter -> Common
machine -> Common session -> Common UI -> bind UI and machine sinks -> run
session. Session must exist before UI can deliver events. Sink callbacks map
Common enum values explicitly and enqueue runtime/frame facts only. A failed
enqueue latches a thread-safe composition failure and requests signal-only Core
stop; no callback destroys a dependency or processes a command inline.

Session options are always `display=COMMON_SESSION_DISPLAY_CONSOLE` and
`console_control=LIB_TRUE`. This is Common's automatic frame-kind route:
graphics means Window plus cooked monitor; text means raw Console; no ready frame
means cooked monitor. It does NOT mean the product forces ASCII. App's independent
`display` setting selects Core's published representation. No Common display setter
or session reconstruction is needed for presentation switching.

On exit: stop accepting App operations; `common_machine_shutdown` with session,
UI and Core alive; destroy UI (joins its producers); detach machine sinks;
destroy session; destroy machine; destroy Core; release providers/config. Partial
construction unwinds only completed objects in this dependency order. Any failed
join retains every reachable callback dependency and returns terminal exit code;
never continue normal recovery or free the object after a failed destroy.

## Public Interface And Thread Ownership

Signatures below are proposed C11 contracts, not header scaffolding. Every
`lib_status` function leaves ordinary outputs unchanged on failure; create sets
its output null before allocation. Null destroy succeeds. No reentrant call is
allowed except the driver's documented signal hooks. Borrowed values last only
for the call unless the table explicitly states otherwise.

| Proposed interface | Ownership and allowed caller |
| --- | --- |
| `lib_status core_driver_create(core_driver **out, const core_driver_options *options)` | Composition, unpublished; copies options and allocates complete NES driver context, initially no cartridge. core_driver is opaque, declared in core/driver_interface.h. |
| `lib_status core_driver_make_driver(core_driver *instance, common_machine_driver *out)` | Composition before Common creation; fills the adopted driver by value; driver borrows this context until machine shutdown/destroy. No allocation or machine execution. |
| `lib_status core_driver_destroy(core_driver *instance)` | Composition only after successful worker join and machine destruction; frees accepted hardware and owned adaptation buffers. |
| `void core_driver_request_input_reset(core_driver *instance)` | Composition's runtime sink on PAUSED/STOPPED/ERROR; signal-only atomic latch, no hardware access. Consumed by executor adaptation before debugging/resumed guest work. |
| `lib_status core_machine_create(core_machine **out, const lib_u8 *rom, lib_size size, const core_machine_options *options)` | Core machine driver only; validates/copies bytes; caller may immediately release input. Creates deterministic hardware and cartridge, not a worker. |
| `void core_machine_destroy(core_machine *machine)` | Executor during safe replacement or composition after join; no host callbacks. |
| `lib_status core_machine_reset(core_machine *machine, core_reset_kind kind)` | Executor; POWER reconstructs volatile initial state, WARM performs reset sequence preserving RAM. Accepted ROM is retained. |
| `lib_status core_machine_run(core_machine *machine, lib_u32 instruction_limit, lib_u32 cycle_limit, core_run_result *out)` | Executor; finite limits, completes at a CPU instruction/interrupt boundary unless a guest fault interrupts a bus operation. |
| `lib_status core_machine_observe(const core_machine *machine, core_observation *out)` | Executor only, copied observation; no implied synchronization or control-thread access. |
| `lib_status core_machine_peek(const core_machine *machine, lib_u16 address, lib_u32 count, lib_u8 *out)` | Executor, 1..256 bytes, non-wrapping address range; validates entire range before copying; no bus reads or time advance. |
| `lib_status core_machine_poke(core_machine *machine, lib_u16 address, const lib_u8 *bytes, lib_u32 count)` | Executor, 1..64 bytes entirely in RAM/mirrors; validate then commit, no MMIO/ROM writes; clears neither faults nor counters. |

The runtime sink signals input reset before enqueuing PAUSED/STOPPED/ERROR facts.
This is a product adaptation signal, not a Core lifecycle call or second run
state. Core remains alive through all sinks, including construction unwind.

`core_driver_options` contains copied feature profile, output settings and controller
bindings; no App callback, Core pointer or borrowed path. M2 accepts only the
execution profile and no gameplay input. `core_machine_options` contains profile
and deterministic initial RAM byte (zero); it contains no host time/surface value.
Feature profiles describe compiled capabilities, not separate emulation engines.

`core_observation`: A/X/Y/S/P (u8), PC (u16), CPU cycles and retired instructions
(u64), cartridge-present/trap-valid (u8), trap reason (u32), trap PC/address (u16),
opcode/access kind (u8). Padding is never transported. Public P reports bit 5 set
and B clear; B exists only in pushed status values. `core_run_result` reports
retired instructions, elapsed cycles, stop reason and copied trap detail. Reasons
are BUDGET, BREAKPOINT, UNSUPPORTED_OPCODE, UNSUPPORTED_DEVICE and BUS_FAILURE.
Operation errors use `lib_status`; a successfully reported guest trap returns OK.

## Driver Execution Contract

`reset` performs POWER reset for all Common cold starts/resets; Common supplies
no reset-reason parameter. Thus product `reset` explicitly means power reset,
including zeroing volatile RAM. WARM exists in hardware contracts/tests and is
available as paused `debug reset` through the debug transport. No fake Common
RESET_COMPLETED event is emitted for that synchronous operation.

`run` calls the supplied executor callback before any instruction, then repeatedly
executes at most 256 instructions or 1024 CPU cycles per slice. The cycle limit
may overshoot by at most the final admitted instruction's remaining cycles
(at most 6 in M2); instruction count is exact. Callback boundaries include a
stop/wake check. Never invoke that callback recursively from `execute_debug`.
No hardware pointer survives the callback: paused media replacement may retire it.
`request_stop` sets a Lib atomic stop latch; `request_wake` sets a separate wake
latch and signals a host wait when present. Reset clears old latches before run.
Heartbeat enables callback service; no unbounded tight loop suppresses it.

An unsupported opcode/device or an execution breakpoint produces a copied
guest stop, then `take_debug_stop` requests Common PAUSED at the next callback.
The trap stays latched and observable. Resume with a fatal guest trap is rejected
by App; reset/reload clears it. Step has the same rule. Host allocation/callback/
publication failures return false/non-OK and enter Common ERROR. ERROR supports
help/exit only; recovery is orderly process restart in this design.
This distinction replaces the earlier S1 rule that every hardware fault enters
ERROR; guest traps should remain inspectable without a second debugger path.

`deliver_input` never mutates hardware from another thread. In M2 it accepts
source cleanup and otherwise ignores non-controller events because no game input
is advertised. Future behavior is specified in Hardware And Presentation.
`copy_frame` in M2 returns OK with `window.valid=0`; no frame allocation is needed
in Core. State-read/write hooks are null (unsupported), as is `frame_published`
unless an actual product consumer is introduced. Debug stop is one-shot;
`cancel_debug` clears pending breakpoint bypass plans, not cartridge/hardware.

## Cartridge Transaction

App calls `common_machine_set_removable_media` on its one control thread, only
STOPPED or PAUSED. `rom insert <file_path>` passes the path with READ_ONLY;
`rom eject` passes a null path. Core implements the existing driver's
`set_removable_media`; no separate cartridge-loading transport is introduced.
The only accepted storage mode is READ_ONLY. Null path unloads; empty path is
rejected by App so it cannot accidentally become Common's null-media operation.
Core machine driver uses `lib_storage_file_read_owned(path, 65552, ...)`, closes the file
before acceptance, and passes bounded bytes to hardware construction. At most
old accepted hardware plus one candidate plus one temporary file buffer coexist.
Parsing/construction failure frees candidate/input and preserves old state.
Successful swap invalidates trap, breakpoints, controller-held state and output
cache. Shared media bool exposes generic failure; do not invent detailed errors
that Common did not return. Unit parser tests still assert typed internal errors.

After interactive insertion, provider returns RESET; only RESET_COMPLETED reports loaded/paused.
Startup attachment instead remains STOPPED until an explicit lifecycle command.
Reset failure after swap retains the new cartridge in faulted context; insertion does
not promise multi-operation rollback. While pending, no new lifecycle/media/debug
command is dispatched. STOPPED media works on Common's existing idle worker.
PAUSED media works inside its callback loop; the run stack must not retain the
discarded hardware. Ejection while PAUSED returns STOP after media succeeds;
ejection while STOPPED completes synchronously. App prints completion on the
matching STOPPED fact, or immediately when already STOPPED.

## Product Debug Protocol Version 1

Only App command/debug includes Core's value-only protocol header. Encode/decode
bytes explicitly; no cast to host structs, pointers, enum-width ABI or packed
layout. All integers below are unsigned little-endian. Transport max request 128,
response 1536. Header: u16 version=1, u16 operation, u32 payload bytes (8 bytes).
Response has that header plus u32 domain result, followed by operation data;
declared lengths must exactly equal initialized bytes. Unknown version/operation,
wrong length, reserved nonzero fields and inadequate capacity reject before work.
Use INVALID_ARGUMENT/UNSUPPORTED/LIMIT_EXCEEDED as appropriate and zero response
size. Transport failure leaves App response storage unusable, not a zero snapshot.

| Op | Request payload | Success data and limit |
| --- | --- | --- |
| 1 OBSERVE | Empty | 40 bytes: A/X/Y/S/P at 0..4; present at 5; PC u16 at 6; cycles u64 at 8; instructions u64 at 16; reason u32 at 24; trap PC u16 at 28; address u16 at 30; opcode/access/valid at 32..34; zeros at 35..39. |
| 2 PEEK | u16 address, u16 count | u16 count, u16 zero, then count bytes; 1..256, whole range readable. |
| 3 POKE | u16 address, u16 count, count bytes | Empty; count 1..64, RAM only, all-or-nothing. |
| 4 STEP | u32 instruction count | u32 retired, u32 cycles, then OBSERVE data; count 1..1000, cycle ceiling 8000. Guest stop is a domain result with partial progress, never transport failure. |
| 5 BREAK_SET | u16 address, u16 enabled (0/1) | Empty; up to 16 unique execution addresses; idempotent set/remove; no condition expression. |
| 6 BREAK_LIST | Empty | u16 count, u16 zero, sorted u16 addresses (max 16). |
| 7 WARM_RESET | Empty | OBSERVE data after 7 reset cycles; retains ROM/RAM, clears guest trap and breakpoint bypass. |
| 8 OUTPUT_SET | u16 mode (0 pixels, 1 text), u16 columns, u16 rows, u16 color (0/1) | Empty; gameplay profile only; fixed constraints in Hardware And Presentation. |
| 9 BIND_SET | u16 button (0..7), u16 scan_code, u32 key, u32 flags | Empty; gameplay profile only; applies one validated binding and clears all held inputs. |

Domain result numbers: 0 COMPLETE, 1 BUDGET, 2 BREAKPOINT, 3 UNSUPPORTED_OPCODE,
4 UNSUPPORTED_DEVICE, 5 BUS_FAILURE. Read/config operations return COMPLETE.
Transport OK does not mean the requested number of instructions all retired.
Maximum STEP response is 60 bytes; PEEK is 272; all fit the existing capacity.
Every call acquires a fresh Common paused lease; no lease survives resume/reset/
media. Validate output capacity and request in full before hardware mutation.
STOPPED/ERROR is not a back door to direct debug calls. Host ERROR status is
Common state plus App's last accepted media fact, explicitly not live CPU data.

Breakpoints test PC before opcode fetch, including interrupt vector destinations.
Normal run from a breakpoint bypasses that address once, then rearms. Step ignores
execution breakpoints for its counted instructions but stops for guest traps;
interrupt entry consumes cycles but does not count as an instruction. No software
BRK patching; BRK remains a guest instruction. Disassembly uses the same immutable
opcode metadata as CPU decode; illegal bytes render `.byte`, not invented code.

## App Commands, Defaults And Parsing

Entry: `MyNes` accepts no command-line options. It loads `mynes.ini` from the
same directory as its executable; unreadable or malformed configuration exits
with code 2 before App/Common construction. The file accepts `rom = PATH` and
`display = window|console`; `display` defaults to window and a missing `rom` means
no cartridge. No autorun: an initial ROM uses the same attachment helper as
`rom insert`.
Blank or ordinary non-assignment lines are ignored; semicolon and hash begin
line comments, and quoted ROM paths may contain spaces. Unknown or malformed
assignments reject startup.
After construction/binding but before session_run, composition calls App's
startup preparation helper on the control thread; it attaches media and returns
failure directly so startup can unwind. Composition publishes Common's initial
STOPPED fact with or without a ROM. The provider's `open` issues no lifecycle
request; the initial fact enables the cooked prompt. Explicit `start` performs
the cold reset/run sequence. Do not set exit_requested
in `open` and expect it to work: existing Session honors that field only for a
submitted monitor line. Exit codes: 0 orderly exit/help, 2 invalid startup
arguments, 1 host/startup failure. A rejected interactive command keeps the session
alive and does not change process success. UI remains the only monitor writer.

Grammar is case-insensitive ASCII keywords; space/tab separate tokens. Double
quotes delimit one path including spaces; backslashes are literal, no shell
escapes, expansion, semicolon chaining or nested commands. An unmatched quote,
extra token, embedded control character or numeric overflow rejects the whole
line. Counts are decimal; addresses/byte values accept `$` or `0x` hex prefixes,
otherwise decimal. No implicit address wrapping. App line bound is 4095 bytes;
media path bound is 1023 bytes including no NUL (buffer capacity 1024). Empty
lines rearm the prompt without output. Every nonempty command result ends with
one blank line before the next prompt. Long lines reject once, never execute a truncated prefix.
Storage currently uses narrow Win32 paths: M2 guarantees printable ASCII paths
only and explicitly rejects non-ASCII; no false Unicode-path claim or Lib patch.

| Command | Allowed completed state | Action/result |
| --- | --- | --- |
| `rom insert <file_path>` | STOPPED/PAUSED | Read-only removable-media transaction then RESET; pause after completion. Quote paths containing spaces. |
| `rom eject` | STOPPED/PAUSED | Removable media null then STOP if needed; idempotent with no cartridge. |
| `start` | STOPPED with cartridge | RESET, then on RESET_COMPLETED request RESUME; no stale hardware continuation. PAUSED reports that `resume`, `reset` or `stop` is required. |
| `resume` | PAUSED with cartridge | Check guest trap then RESUME. STOPPED reports that `start` or `reset` is required; RUNNING reports already running. |
| `pause` | RUNNING | PAUSE; PAUSED is an idempotent no-op. Empty/stopped rejects. |
| `stop` | RUNNING/PAUSED | STOP; retains cartridge. STOPPED is an idempotent no-op. |
| `reset` | STOPPED/RUNNING/PAUSED with cartridge | Common RESET -> power reset -> PAUSED; reports RAM clear semantics. |
| `help` | Any, including ERROR | Shows the primary monitor grammar and gameplay controls. |
| `debug`, `debug help` | Any non-ERROR state | Lists the nested debug grammar. |
| `debug regs`, `debug mem ADDR [COUNT]` | PAUSED with cartridge | OBSERVE or PEEK; mem default 64, max 256. |
| `debug poke ADDR BYTE...` | PAUSED with cartridge | POKE, max 64 bytes; RAM/mirrors only. |
| `debug disasm ADDR [COUNT]` | PAUSED with cartridge | Peek at most 3*count bytes, count default 8/max 32; no wrap or MMIO. |
| `debug step [COUNT]` | PAUSED, no fatal guest trap | STEP, default 1/max 1000; stays PAUSED and reports actual retirement. |
| `debug break ADDR`, `debug delete ADDR`, `debug breaks` | PAUSED with cartridge | Bounded breakpoint operations. |
| `debug reset` | PAUSED with cartridge | WARM_RESET protocol; remains PAUSED. |
| `exit` | Any | Provider sets exit_requested; composition always does permanent shutdown even if session's preliminary STOP was rejected. |

While a lifecycle request is pending, other machine commands report busy; help
and exit remain available. Provider tracks one pending command intent and
accepted media identity, not an authoritative running flag. Common state_get and
matching runtime facts remain authoritative; STARTING is busy even if the last
session-visible fact was STOPPED. `note_runtime` finishes only the corresponding
intent; ERROR cancels it. `begin_external` validates hotkey/window-close actions
through the same policy. `arm_prompt` expresses readiness, never permission to
start a second native reader. Output is bounded to 16383 bytes, prompt `MyNes> `
to 63; `detail` is null. Bounded mem/disasm listings fit without dynamic logs.
Notifications queued while raw Console owns the endpoint are held in one bounded
App notice and emitted when `note_monitor_current` reports true. Notification
wording advises re-entry of any unsubmitted edit; App does not pretend to know
the native reader's partial buffer. No direct worker printing.

Defaults: no cartridge, no autorun, zero RAM, 256/1024 execution slice,
display=window, text=80x25/color, and the keyboard mapping in Hardware And
Presentation. `mynes.ini` is the only startup configuration surface; monitor
commands never alter configuration. Additional ini keys or CLI switches require
their owning capability admission.

## Build And Test Ownership

Future CMake boundaries: `mynes_core_hardware` (Lib types only), `mynes_core_driver`
(hardware + Lib + Common public contract includes), `mynes_app` (Common/Lib plus
composition link to Core). App debug's value header introduces no Core machine driver link.
Both Core targets compile explicitly selected files from core. They
enforce dependency visibility within one component directory, not extra runtime
layers. Driver owns a replaceable core_machine hardware instance so media can
commit a candidate while the Common driver context remains stable. Hardware
never includes driver.h or Common; App never obtains the hardware handle.
Independent component includes and header compile tests enforce this graph.
The first source admission creates the product target and matching x64/x86
artifact identity under Execution; this document creates none.

Warnings: C11, `-Wall -Wextra -Wpedantic -Werror`, assertions enabled in tests.
Tests belong in `test/app-mynes/unit/product`, `test/app-mynes/unit/core`, and
`test/app-mynes/integration`; shared test roots stay identical. Hardware source
is grouped under `core` following the NXVM reference and tested under
`test/app-mynes/unit/core`; driver tests live there too
and exercise its separate public contract against real or fake Common callers.
Source Layout defines this organization without changing component ownership.
Original inline/constructed fixture bytes use the production
parser/bus/decoder, never a parallel test emulator. The full acceptance matrix is
[Verification](app-core-verification.md).
