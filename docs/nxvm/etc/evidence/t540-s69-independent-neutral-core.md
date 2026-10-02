# T540 S69: independent neutral Core receiving proof

## Scope and implementation

Receiving baseline is accepted S68 P2 `2144e4e2a`. S69 changes only NXVM
build/test registration and its evidence. No production source, public API,
Shared corpus, MyNES, owner INI or external asset changes.

The test-only OBJECT target compiles the actual sixteen production sources:
clock, cpu_bus, debug, entry_plan_interface, machine_firmware, machine,
machine_scheduler, memory_interface, port_interface, rom_mapping_interface,
trace_interface, retirement_observation_interface, timeline, port, memory and
transaction. Every object is linked into the smoke; archive extraction cannot
hide an unresolved board dependency. Configuration verifies production source
membership and the exact CPU/FPU/Types target dependencies. Runtime trace
definitions are inherited from production, not independently invented.

Compiler dependency closure contains eighteen App headers: clock.h,
debug_interface.h, device_support.h, entry_plan_interface.h,
execution_provider.h, firmware_interface.h, lifecycle_interface.h,
machine_interface.h, machine.h, memory_interface.h, memory.h,
port_interface.h, port.h, retirement_observation_interface.h,
rom_mapping_interface.h, timeline.h, trace_interface.h and transaction.h.
No board-state, profile or product adapter header is required by these objects.

The actual executable link includes all sixteen objects and only the CPU/FPU
project archives, plus ordinary toolchain system imports. It excludes the
all-chip executor aggregate and board/peripheral archives. Symbol inspection
confirms real neutral construction, entry-plan and firmware implementations,
without board construction/finalization or PIC/PIT implementations.

The smoke constructs no board and supplies only test-owned port/trace callbacks
and an inline MOV/NOP/HLT ROM. It exercises reset, RAM and immutable ROM,
port access, register read/write, bounded stepping/continuation, HLT, copied
time observations, stop and sole destruction. The ROM-write check attempts a
different byte. No Core stub, copied implementation or external input is used.
Existing board fixtures remain intact.

## Verification

- Debug and Release independent compile/link/run pass on x64 and x86; each
  reports `M5:T540:S69:NEUTRAL-LINK:OK`. Trace presence matches production's
  enabled/disabled definition. GNU strict warning flags are enabled.
- Final complete unit runs pass 470/470 on each width: x64 319.50 seconds,
  x86 78.34 seconds. Both-width specialized gates pass all 81 build steps.
- Exact dependency inventory remains 37 edges; documentation and diff checks
  pass. Receiving logs are ignored `build/s69-unit-{x64,x86}.log`,
  `s69-gates-{x64,x86}.log`, `s69-neutral-{x64,x86}-build.log` and
  `s69-neutral-release-{x64,x86}-{configure,build}.log`.
- All eight accepted S68 0540 EXE hashes match their recorded identities.
  No product executable input changed, so no product rebuild or repeated
  external boot is required. MyNES was not built. S68's eight once-only boot
  checkpoints remain historical evidence, not a fresh S69 integration run.

## Remaining boundary

This proves Core can compile and execute without PC board implementations;
it does not prove all board consumers already use only public Core contracts.
Existing board composition still accesses private Core state. Physical Shared
relocation must classify those consumers and the eighteen-header receiving
surface before claiming a complete cross-component boundary. T540 remains
open for that relocation and IBM-PC common/AT/XT extraction.

The proof target is test-only, not a second production executor. At physical
relocation it must be replaced by the real Shared Core target and the smoke
retargeted, rather than retaining duplicate production compilation paths.
