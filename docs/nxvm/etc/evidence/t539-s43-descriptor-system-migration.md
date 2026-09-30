# T539 S43 Descriptor-System Migration

## Scope and ownership

The former `core_machine_descriptor_system_smoke.c` mixed two independent
concerns. S43 moves its descriptor/table/cache cases to the CPU-local
`cpu_descriptor_system_smoke.c`, which links only `x86-cpu` through
`cpu_instruction_fixture`. The retained source now contains only the S45
control-state cases (`SMSW`, `LMSW`, `CLTS`, `MOV CR`) and the real-mode
transition assertion that executes `MOV CR0`; it is not a second descriptor
receiver.

| Original case family | S43 receiver | Boundary |
| --- | --- | --- |
| `SLDT` / `STR` stores, register and memory forms | CPU descriptor smoke | Decoder, selector cache and local memory fixture |
| `LLDT` / `LTR`, null/type/present/busy rejection | CPU descriptor smoke | Descriptor validation and cache/busy publication |
| `SGDT` / `SIDT`, `LGDT` / `LIDT`, 16/32-bit layouts | CPU descriptor smoke | Table-register architectural state and local memory fixture |
| CPL/VM86/LOCK/register-form rejection, table rollback | CPU descriptor smoke | Architectural fault/rejection state; protected 80386 terminal delivery retains the prior final `#DF` observation |
| `SMSW` / `LMSW` / `CLTS` / `MOV CR` and `MOV CR0` PE exit | S45 remainder | Control-state ownership, intentionally not migrated in S43 |

No production source, public ABI, Shared component, firmware, INI or executable
input changes in this package.

## Verification and acceptance

- New CPU receiver: x64 strict compile/link/run passed and printed
  `M5:T539:S43:DESCRIPTOR-SYSTEM-CPU:OK`.
- Retained S45 control source: rebuilt directly with the configured x64 strict
  compiler, linked against the existing Core libraries, and printed
  `M5:T304:CONTROL-STATE:OK`.
- The retained source and new CPU receiver pass x64 and x86 strict syntax
  compilation with `-Wall -Wextra -Wpedantic -Werror`; the new x86 receiver
  also links and runs with its S43 marker.
- Complete repository-only units pass 457/457 on both x64 and x86. The x86
  result is from a freshly configured MinGW Makefiles tree, so it includes the
  new CTest registration rather than a stale generated test list.
- Both specialized aggregates pass, including T317's 36 strict CPU receivers,
  T332's 36 fixture owners, T337, T344, T345, CPU/PIC authority, direct-matrix,
  manifest and documentation-governance gates.
- `git diff --check` and actual-change review of P1 pass; the pushed commit is
  `4ff59cd5c`.

S43 is accepted. The retained control-state source is the single S45 input;
there is no compatibility receiver or duplicate descriptor path.
