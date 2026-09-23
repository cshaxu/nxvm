# T527 S2: Shared Debug Contract Inventory

## Purpose

This is the frozen migration contract for later T527 work. It records the
current Debug surface and its approved receiver; it does not claim that
`common/debug`, `common/session`, `common/machine`, or `common/ui` exists yet.

## Shared Ownership

| Concern | Final owner | Boundary |
| --- | --- | --- |
| x86 assembly/disassembly | `common/xasm32` | `lib/types` only; bounded copied text/code values |
| monitor ordering and output | `common/session` | one session FIFO; registered CLI providers receive lines and return copied text, prompt, and optional lifecycle request |
| ordinary product command grammar | product CLI provider | registered peer of Debug; product selects raw-console/monitor policy and prompt |
| Debug command grammar and continuation | `common/debug` | registered CLI provider; it owns Debug prompts, `A/E/R/V/XA/XE/XR`, and DOS-style trace grammar |
| paused machine observation/mutation | `common/machine` | synchronous bounded paused-Debug operations through its product driver; no Core pointer crosses the boundary |
| NXVM target adaptation | `vm/machine` | binds Core machine to the common-machine driver only |
| Core machine enforcement | `core/machine` | validates paused access and owns breakpoints/execution-plan enforcement |

The only intended NXVM route is:

```text
core/machine -> vm/machine -> common/machine -> common/debug
```

`common/debug` never calls Core or VM directly. `common/session` never learns
Debug command types; it only invokes the registered provider callback.

## Current Continuation Universe

The existing Debug monitor has eight line-continuation families: `A`, `E`,
`R`, `V`, `XA`, `XE`, `XR`, and the main prompt/`Q`. Later migration must move
all eight through the registered common Debug provider; leaving a nested raw
Console read would create a second input route.

## Command Classes

| Class | Final route |
| --- | --- |
| Paused register/memory/port reads and writes | common Debug synchronously calls common-machine paused operations; the product driver reaches the target owner |
| Breakpoint and execution-budget configuration | same paused operation route; Core machine enforces the resulting plan |
| `G`, `T`, `XG`, `XT` lifecycle continuation | Debug returns an ordinary copied lifecycle request to common session; session orders it like every other lifecycle request and later supplies the machine fact back to the provider |
| `N`, `L`, `W` paths and files | injected product file service built over `lib/storage`; Debug owns neither host path nor file handle |
| DOS Debug trace/step behavior | mandatory shared capability; target owner enforces the configured execution budget |
| NXVM raw instruction recorder | optional product capability, retained only if it cannot be expressed by the shared trace sink without changing semantics |

## Migration Constraint

No later S may add a Debug-specific queue, polling loop, raw Console access,
host handle, Core/VM pointer, or duplicate assembler path. SoftPC may bind its
own machine driver and register the same common Debug provider without taking
an NXVM dependency.

## S2 Delivery Proof

- Production xasm source moved from `core/debug/xasm32` to
  `common/xasm32`. Its sole non-C-standard include family is
  `lib/types/types_interface.h`; the corresponding source/test sweep found no
  `type.h`, Core, VM, legacy `type_*`, `STD_*`, `C_*`, or `TYPE_*` dependency.
- Both direct callers now use `common_xasm32_*`; no old xasm production symbol,
  source path, CMake target, or test registration remains. The former mixed
  `core/debug/utils` owner was deleted: its private text helpers had one
  caller and now live as static helpers in `core/debug/debug.c`.
- `common-xasm32` is a standalone CMake library that links only `types`.
  Its two owner-local unit targets are `common-xasm32-smoke` and
  `common-xasm32-contract-smoke`.
- MinGW Release configuration and affected target build passed. Both xasm
  smoke tests passed; the complete serial unit suite passed 299/299. Relevant
  Core Debug, debugger-capability, unit-registration, unit-separation and
  documentation-governance gates passed. An earlier parallel run exposed a
  pre-existing shared-storage test isolation issue; the same storage test and
  the complete serial suite passed, so it is not attributed to this change.
- The staged `git diff --numstat` records source/build changes of
  +5,052/-4,999 and owner-test changes of +103/-142 (documentation
  excluded): +14 net. The positive source delta is the one bounded public
  xasm contract and its private text/trace replacement; it removed three old
  Core Debug helper files and every old production/test route.
