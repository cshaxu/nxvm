# T434 S1: Core Timing-Plan Declarations And Copied Construction

`M5:T434:S1:PLAN-DECLARATIONS:OK`
`M5:T434:S1:PLAN-VALIDATION:OK`
`M5:T434:S1:PLAN-COPY:OK`

## Delivered Core boundary

S1 adds `core_machine_plan`: a Core-owned copied input containing the existing
neutral `core_machine_config` construction values and exactly thirty timing
declarations.  A declaration consists of one stable capability, one
disposition, and its one consumer seam.  `core_machine_create_from_plan()`
validates the complete table before it allocates or publishes a machine, then
copies it into Core-owned storage.  It clears the only caller-owned
configuration pointer after the existing retirement-qualification copy has
completed.

The existing `core_machine_create()` remains unchanged in this batch as the
sole current production route.  It is deliberately not another plan publisher:
no VM caller uses the new entry yet.  B2 will atomically migrate the three
materializers and remove their direct publication route.

## Frozen declaration map

| T433 ID | Core enum | S1 seam | default disposition |
| --- | --- | --- | --- |
| `CPU-EXEC` | `CPU_EXEC` | `CPU_PROGRAM` | L2 |
| `CPU-EXCEPT` | `CPU_EXCEPT` | `CPU_PROGRAM` | L2 |
| `CPU-PREFETCH` | `CPU_PREFETCH` | `CPU_PROGRAM` | L2 |
| `CPU-RETIRE` | `CPU_RETIRE` | `RETIREMENT` | L2 |
| `CPU-FPU` | `CPU_FPU` | `CPU_PROGRAM` | L2 |
| `TIME-CLOCK` | `TIME_CLOCK` | `CLOCK` | L2 |
| `TIME-LIFECYCLE` | `TIME_LIFECYCLE` | `LIFECYCLE` | L2 |
| `TXN-MEMORY` | `TXN_MEMORY` | `TRANSACTION` | L2 |
| `TXN-PORT` | `TXN_PORT` | `TRANSACTION` | L2 |
| `TXN-ARBITRATION` | `TXN_ARBITRATION` | `TRANSACTION` | L2 |
| `MEM-RAM-A20-PARITY` | `MEM_RAM_A20_PARITY` | `MEMORY` | L2 |
| `MEM-ROM-FIRMWARE` | `MEM_ROM_FIRMWARE` | `MEMORY` | L2 |
| `MACHINE-CONFIG` | `MACHINE_CONFIG` | `CONFIGURATION` | L2 |
| `CTRL-PIC` | `CTRL_PIC` | `DEVICE` | L2 |
| `CTRL-DMA` | `CTRL_DMA` | `DEVICE` | L2 |
| `CTRL-PIT` | `CTRL_PIT` | `DEVICE` | L2 |
| `CTRL-RTC-CMOS` | `CTRL_RTC_CMOS` | `DEVICE` | L2 |
| `CTRL-KBC-NMI` | `CTRL_KBC_NMI` | `DEVICE` | L2 |
| `CTRL-FDC` | `CTRL_FDC` | `DEVICE` | L2 |
| `CTRL-HDC` | `CTRL_HDC` | `DEVICE` | L2 |
| `MEDIA-BACKING` | `MEDIA_BACKING` | `DEVICE` | L2 |
| `DISPLAY-VADP` | `DISPLAY_VADP` | `DEVICE` | L2 |
| `DISPLAY-PRESENT` | `DISPLAY_PRESENT` | `OBSERVATION` | NGT |
| `INPUT-HOST` | `INPUT_HOST` | `OBSERVATION` | NGT |
| `TRACE-DEBUG` | `TRACE_DEBUG` | `OBSERVATION` | NGT |
| `PLATFORM-MAILBOX` | `PLATFORM_MAILBOX` | `OBSERVATION` | NGT |
| `PLATFORM-RESOURCE` | `PLATFORM_RESOURCE` | `OBSERVATION` | NGT |
| `PLATFORM-WAIT` | `PLATFORM_WAIT` | `OBSERVATION` | NGT |
| `SESSION-COMMAND` | `SESSION_COMMAND` | `OBSERVATION` | NGT |
| `PRODUCT-DEBUG` | `PRODUCT_DEBUG` | `OBSERVATION` | NGT |

The validator requires count 30, valid/unique capabilities, the listed seam
for each capability, NGT for the eight non-guest-time capabilities, and L2 for
all remaining capabilities.  This is intentional S1 behavior: no L3 rule
registry has been admitted yet, so a plan demanding L3 is rejected atomically
rather than silently downgraded.

## Verification

The focused `core-machine-plan-smoke` was compiled from current source and
passed.  It proves default mapping, copied lifetime after caller mutation,
duplicate/missing declaration rejection, unavailable required-L3 rejection,
invalid NGT disposition rejection, and invalid seam rejection.  The existing
`core-machine-ram-create-smoke` was compiled from the same current source and
passed, preserving direct-config creation during this pre-cutover batch.

Both builds used GCC 16.1.0 with the current Core source list.  The added
machine source and smoke also passed `-std=c11 -Wall -Wextra -Wpedantic
-Werror -fsyntax-only`.  CMake/Ninja configuration was not used as final
evidence because its pre-existing build tree reported a premature `build.ninja`
end and left child processes; the owned processes were stopped and the direct
current-source compile above is retained as the focused proof.

## Code-size and retirement review

The implementation diff is `+324/-0` machine-readable lines: `+168/-0` in
`machine.c`, `+77/-0` in `machine_interface.h`, `+2/-0` in `machine.h`,
`+73/-0` for the focused smoke and `+4/-0` for its CMake registration.  The
count is `git diff --numstat` for tracked paths plus `Measure-Object -Line` for
the new smoke.  The positive change is material because the 30 frozen entries,
their exact seam map, copied-state validation and negative cases did not exist
in any owner-local boundary.  No existing path became obsolete in S1: the
direct config publisher remains the sole production publisher until B2, and no
duplicate publisher was introduced.

## B2 boundary

S1 does not add a VM production publisher.  B2 must materialize this plan for
default PC/AT, IBM 5170 Model 339 and Model-40 BYOB, move their post-create
Core configuration values into the plan-only route, remove their direct
`core_machine_create` calls, and prove all three product equivalences.  It may
not introduce a second compatibility publisher.
