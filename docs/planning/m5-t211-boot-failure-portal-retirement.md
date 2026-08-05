# M5 T211: Boot-Failure Portal Retirement

## Goal

Retire the default profile's private `INT F0h` boot-failure stop portal without
changing the retained NXVM startup, Console, or debugger experience.

## Contract

The default ROM owns the failure UI and reports an acknowledged failure through
the profile-defined BDA POST work-area byte. Core owns that RAM as ordinary
guest state; it has no boot-failure policy. At a normal instruction-budget
boundary, the VM session runner consumes the report and is the sole actor that
requests core stop. No callback, queue, fake port, or direct host-side BDA
mutation is introduced. Ordinary guest `INT F0h` consequently follows the IVT.

## Breakdown

### S1: Report Contract

Define the reset-cleared BDA report values and retain a focused consumption
probe. The report is single-use: the runner clears it before requesting stop.

### S2: ROM And Session Boundary

Replace ROM `INT F0h` with the BDA report write, remove F0 registration and
dispatch from the profile portal, and make the runner consume the report before
the next guest instruction. Core, Console, debugger, platform cancellation,
and normal IVT handling remain unchanged.

### S3: Regression

Run the report-consumption smoke, the static closure gate, and the full current
matrix, including FDD prompt, keyboard, display, debugger, and two-session
coverage. The task has no DMA or optional Bochs differential surface.

## Known Follow-Up

At T211 completion, an exploratory no-media end-to-end run remained at
`F000:028E` during the existing text-video error display. This was not evidence
against the new report boundary or a T211 regression. T212 subsequently retired
the video portal and added bounded no-media ROM-video coverage through the key
wait/report checkpoint.

## Verification Evidence

Applicable rules: profile ROM owns firmware semantics; core remains
product-neutral; session composition owns product lifecycle; no new bypass or
second stop path enters the graph.

| Surface | Verdict | Evidence |
| --- | --- | --- |
| S1 report consumption | run | `vm-boot-failure-lifecycle-smoke` writes the profile report, verifies session stop and one-shot clearing |
| S2 closure | run | `verify-boot-failure-portal-closure` proves no default-profile `INT F0h` or stop portal remains and runner consumes the report |
| F0 IVT semantics | covered by | T209 generic portal smoke retains the matching-vector/nonmatching-origin IVT fall-through contract; the default profile no longer registers F0 |
| Retained product behavior | run | all current FDD prompt, keyboard, display, Console/debugger, session, and platform smokes pass |
| Optional Bochs differential | not used | no device behavior changed |

`cmake --build --preset current-gates-gcc --parallel 4` passed all static gates
and executed 46/46 current CTest smokes. `cmake --build --preset current-gcc
--parallel 4` produced `build/output/nxvm_0_5_0211.exe` with SHA-256
`0198B04D66DA3898FA8E354783181D924A08B736301672A00F640B772AE758A1`.
The artifact prints `Neko's x86 Virtual Machine [0.5.0211]` and enters the
retained Console.
