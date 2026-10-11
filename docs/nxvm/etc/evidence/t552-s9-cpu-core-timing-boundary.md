# T552 S9 — CPU/Core Compatibility-Timing Boundary

## Decision

No production repair is required.  Every timing recipe, including the retained
compatibility endpoint, has one owner: `src/core/chips/cpu`.

`core_machine_cpu_timing_select()` in `cpu_timing.c` is the only successful
retirement selection entry.  It owns candidate order, result origin, source
form, input classification and the `source_timing_unallocated` distinction.
`core_machine_compatibility_instruction_cost()` in `cpu_timing_model.c` is one
of those CPU candidates; it marks itself unallocated before returning its
deterministic cost.  The endpoint is therefore valid for deterministic
execution progress, but never claims a source-qualified physical duration.

The CPU selector calls the compatibility candidate only after the profile's
more-specific candidates decline: the 8088 branch ends after primary/control
routes; the 8086/80386 branch ends after its profile-specific fallback.  80186
and 80286 have their own profile-private chains and do not enter the generic
compatibility endpoint.  `L2_CONTROL_MODEL` remains a CPU-origin override when
the CPU cannot decode the next control-transfer lexeme; it is not a Core
estimate or a second selector.

## Core Boundary

`src/core/x86/machine.c` calls the selector once after an instruction executes.
It adds its separately-owned external-cycle round ticks, checks arithmetic
overflow, captures the copied retirement eligibility key, publishes the
retirement and advances elapsed guest time.  The Core layer neither switches
on timing origin to choose a formula nor computes an instruction cost.  Its
observation/eligibility consumer may reject a source-unallocated result for a
physical-time configuration; that is a consumer policy, not a recipe.

Consequently no API, enum, state mirror, formula, fallback or App dependency is
introduced by S9.  The current `core_machine_retirement_timing_origin` values
remain the CPU result vocabulary; the Core only transports them in copied
observation data.

## Direct Test Ownership

| Concern | Owner-local receiver | What it proves |
| --- | --- | --- |
| Exact per-profile source/fallback selection | `machine-8086/8088/80186/80286/80386-timing-manifest-runner` | CPU result tick/origin/form records for each CPU family. |
| Inaccessible next control-transfer lexeme | `cpu_transfer_boundary` | CPU selects `L2_CONTROL_MODEL` only for the unavailable boundary; mapped paths retain their exact control timing. |
| Conditional/near/far control transfer selection | `cpu_control_transfer_branch`, `cpu_control_transfer_near`, `cpu_control_transfer_far` | CPU control semantics and timing result selection remain allocated. |
| Core transport/consumer contract | `core-machine-cpu-timing-preview-smoke`, `core-machine-retirement-observation-smoke` | Core publishes the selected copied timing/origin and applies eligibility without reimplementing a recipe. |

## Verification

On both x64 and x86, all eleven direct receivers above passed: five
timing-manifest runners, four CPU transfer receivers and two Core consumer
receivers.  This is a documentation-only reconciliation; it does not rebuild
or redeploy artifacts.  Whole-task qualification remains governed by the T552
closure matrix.
