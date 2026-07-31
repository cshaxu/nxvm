# M5 Firmware And NXVM Implementation Breakdown

M5 implements the M4 design only. Every task keeps the transitional
`nxvm.full_pc` adapter bootable while introducing only
`nxvm.machine.pc_at_builtin`, builds a usable `nxvm-m5_t<T>.exe` task artifact,
and records its exact source commit, SHA-256, banner, and verification result.
Each subtask is activated only after it receives its own scope record.

## Shared Gates

- Windows x64 GCC/CMake/Ninja is the required build path; new project code
  builds with `-Werror`.
- M1 FDD/HDD fixture identities remain local, read-only inputs. Each full-PC
  regression uses the recorded ten-second wall-clock and no-progress budgets;
  reaching the recorded DOS `INT 21h` checkpoint is success, not a claim of
  guest natural exit.
- `core` must not include firmware, product, DOS, platform, or baseline source.
  Firmware must not include concrete host APIs. Product Console and platform
  adapters must not mutate guest state outside the runtime command boundary.
- No task adds guest media, Microsoft binary, external-ROM loading, an
  additional machine profile, a process CLI for nxvm, a DOS ABI, or ntvdm64
  product behavior.

| Task and subtasks | Approved decision | Implementation and acceptance evidence | Stop condition |
| --- | --- | --- | --- |
| T1 S1: implement profile/provider registry; T1 S2: register `nxvm.machine.pc_at_builtin` through the transitional adapter | M4 T5 profile/provider requirements. | Family, ABI, duplicate, late-change, capability, and provider selection tests pass; no external-ROM provider is loadable; existing adapter still reaches reset vector. | The registry names DOS, host paths, ROM bytes, or an additional machine profile. |
| T2 S1: migrate PIC/PIT/DMA contracts; T2 S2: migrate keyboard and QDX snapshot contracts | M4 T1 assigns optional generic PC-compatible devices to `core`. | Typed `core` device registration replaces selected baseline global registrations; deterministic IRQ/timer/DMA and keyboard/text-snapshot tests pass; the built-in profile still reaches reset vector. | A device needs BIOS policy, direct platform access, or a DOS decision. |
| T3 S1: implement Firmware V1 registry; T3 S2: compose built-in PC/AT ROM, POST, CMOS, and BIOS services | M4 T2 and M4 T5 provider contracts. | Composition/freeze/duplicate/failure tests pass; reset trace reaches `F000:FFF0`; ROM/BDA and required INT 10h/13h/16h/1Ah service identities are traceable. | A service requires a host handle/path/UI, external ROM, or unsupported BIOS compatibility. |
| T4 S1: implement NXVM media policy and block-provider adapter; T4 S2: migrate FDC/HDC/FDD/HDD built-in PC/AT composition | M4 T1 assigns media policy and storage composition to `products/nxvm`. | Unit tests reject media mutation while running; fixture insertion/connection markers, hashes, and bounded FDD/HDD checkpoints match M1. | Fixture identity changes, a write policy is ambiguous, or a host filesystem rule from M8 is required. |
| T5 S1: implement retained Console command adapter; T5 S2: implement synchronized presentation and debugger adapters | M4 T3 Console contract. | Scripted Console tests cover help, invalid command, media configuration, start/reset/stop/resume/debug/exit state rules; mock event/snapshot tests prove no cross-thread guest mutation; manual Windows console/window smoke passes. | A command requires direct global access, a host-specific ntvdm64 rule, or an unbounded UI wait. |
| T6 S1: compose first-class `nxvm.machine.pc_at_builtin`; T6 S2: retire covered baseline adapter paths and close M5 | M4 ownership and all shared gates. | New product composition boots both M1 fixtures under budgets, all focused tests pass, forbidden-dependency scans pass, and each removed adapter path has equivalent evidence. The final task artifact is revalidated. | Any M1 checkpoint regresses, an active baseline global path has no owner, or cleanup changes observed behavior without a focused test. |

## Task Artifact Rule

Each task's artifact starts the retained interactive NXVM Console, not a smoke
harness. Its build preset and test command are added by that task. T1 through
T5 artifacts may use the retained adapter while their replacement is incomplete;
T6 must use the first-class composed path. The Console banner follows the
task-level pre-cutover form `0.4.015d.m5t<T>` until version policy changes in an
approved task.

## M5 Exit Review

M5 closes only when the first-class `nxvm.exe` path owns the profile/provider
registry, built-in PC/AT firmware, PC/AT device composition, media policy,
Console, and presentation/debug adapters through the M4 contracts; both M1
fixture paths retain bounded checkpoint evidence; and the verified `main`
commit receives an immutable M5 snapshot before M6 starts.
