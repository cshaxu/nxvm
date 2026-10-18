# T2 S21 Native PSW And Flag Control

Baseline `09e4f6518`, after accepted S20. This consumes V10's eleven native
PSW/flag instruction forms and existing public CPU flag patch/prepared-entry
boundary. General BRK/priority, emulation-sensitive PSW/IRET/mode changes,
physical timing, Machine/board and playable game remain original receivers.

Shared P1 `8c0e1d9dc` is pushed. The product-local evidence delivery retains
the complete sixteen-field S packet before coordinator actual-change review;
no receiving NXVM/MyNES P is warranted for unchanged executable inputs.

## Primary Facts And Retained Models

The T1-pinned NEC V20/V30 October 1986 manual remains primary, SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`.
Printed 3-3 defines the PSW stack image, including fixed bits 14--12 and 1
as one and bits 5/3 as zero. Printed 12-10/11 define AH transfers,
12-148/151 define PUSH/POP PSW, 12-89/94/99 carry/direction controls and
12-172 DI/EI. Section 6 describes IE inhibition through the instruction
following EI and separately describes BRK entry.

Rendered PDF pages 16, 48, 134, 139 and 144 were inspected, not only OCR.
Table 12-4 on printed 12-3 explicitly defines U as Undefined and a blank
as No change. The three carry controls mark V/S/Z/AC/P undefined, so tests
mask only those five flags, not CY or control flags. MOV AH,PSW similarly
masks its undefined AH bits 5/3/1. Deterministically retaining the old values
is allowed but is not promoted to specified NEC behavior. All other defined
state is checked. No production change is warranted merely to choose another
undefined value or to match an Intel convention.

The 12-151 POP PSW footnote keeps MD set in ordinary native execution, but
allows different behavior inside calls originating in emulation mode and
warns against modifying MD there. This batch explicitly uses ordinary native
MD=1 only. It does not certify current MD-preservation helpers for that later
mode domain or introduce a second emulation-history flag.

Initial BRK is clear. POP PSW can load BRK; its resulting state is checked
under the existing completion-samples-previous-BRK model. This neither
certifies the exact NEC first-trap boundary nor retires the general step/entry
receiver. The pushed fixed bits are primary-defined, not an invented timing
model. Canonical writable live state, S8 contiguous data tails and unrelated
source/model grades retain their existing ownership.

## Sole Owners And Similar-Issue Sweep

The current native handlers, `_e_real_flags_defined_mask/load_16/image_16`,
`_e_eflags_load`, public Debug patch/read and prepared-entry paths already
produce the independent expected results. No production defect is reproduced.
CPU remains the sole PSW/mode/interrupt-shadow owner. No source, public API,
layout, persistent state, executor, provider, Lib/Emulator or private App path
is changed. The new test directly consumes S20's actual-memory transfer plan;
no duplicate plan/checker, modulo RAM or test-only production entrance is added.

The sweep queries common PSW writers/images and their callers in cpu.c and
cpu_instructions.c, all eleven handlers, reset/native admission and completion
sampling. Native ordinary PUSH/POP, AH and carry/direction/IE have this proof.
Intel protected/32-bit flags, task/IRET/SMM callers retain their own distinct
existing units and architecture; they are not rewritten into a V30 abstraction.
MD-sensitive emulation/native-return and general step/entry remain explicit
V10/V11/V13 receivers. No caller is silently treated as equivalent merely
because it invokes the same load helper.

```text
rg -n '_e_real_flags_(defined_mask|load_16|image_16)|_e_eflags_load\(|X86_CPU_NEC_PSW_MD' src/x86-core/cpu
rg -n 'PUSHF|POPF|SAHF|LAHF|CLC\(|STC\(|CMC\(|CLD\(|STD\(|CLI\(|STI\(' src/x86-core/cpu/cpu_instructions.c
```

## Finite Proof

`cpu_v30_flags_smoke.c` checks 406,800 input cases, plus every observed
provider refusal for the form/context matrix:

- 256 PUSH PSW flag states; 131,072 POP PSW cases, every word against two
  opposing input flag states; 65,536 each AH direction, every AH byte against
  every live non-BRK flag combination; and 1,792 seven-control flag cases.
  These 264,192 cases include all defined native non-BRK input flags.
- 11,000 form/context cases: all eleven forms, absent/four segment overrides,
  four code positions, five odd/even/wrapped stacks, five raw values and two
  opposing input flag states. Full CPU/defined flags, MD, SP/high halves,
  PC, EI shadow and exact physical stack access/order/phases/final bytes are
  independently checked. PUSH executes a real POP continuation and restores
  the original full CPU except following PC.
- 131,072 public Debug flag writes: every word, both zero/FFFF high-half
  patterns, exact resulting full CPU and readback without execution/bus effects.
- 512 prepared-entry flag combinations, checking uncommitted preparation,
  abort, a fresh commit, copied public flags/PC and specified segment/stack
  fields. Twenty-three forbidden flag bits are individually rejected without
  a prepared object or CPU mutation. One mixed Debug register/flags/unsupported
  CR0 patch proves rejection leaves the entire CPU unchanged.

Every context proves pure nonmutating preview, then refuses each successful
data-read, data-write, code-fetch and observational-fetch ordinal separately.
CPU rolls back/no guest exception or shadow is created on host failure, and
only the actually accepted stack-write prefix remains. No retry/replay or
stronger physical-bus/tail claim is introduced. Source-mandated undefined
outputs are the only result masks; rollback still compares the unmasked CPU.

The first strict compile caught a test assumption that the compact public
execution-state snapshot contains SP. It does not; the CPU-owned fixture now
checks its own stack state without changing the public snapshot. A primary
PDF review then tightened the result oracle to exclude undefined carry-control
arithmetic flags before full qualification. Neither adjustment changes source,
weakens a defined result or converts a failing production witness into a pass.

## Verification And Delivery

Original x64/x86 Release Ninja/ccache graphs build all targets, then run the
complete repository unit suites and existing specialized gates. The finite new
unit has a pre-admitted 90-second containment budget; no old timeout is changed.
The focused x64 matrix passes all 406,800 inputs in 1.27 seconds. Both first
complete unit suites pass 671/671 (184 X86 Core) in 96.64/107.31 seconds.
The new matrix takes 0.770766/1.01660 seconds in those suites; S20's 463,982
and S19's 1,796,404 input matrices remain intact. Both specialized targets
retain 498 strict rows, plus manifests, Types, inward dependencies,
ownership/negative and documentation/whitespace gates. The tracked-path
global Types check also includes the staged new unit. Existing warning
exceptions are retained, not claimed as removed. All handles have exited.

[Results](t2-s21-results.json) and [artifact identities](t2-s21-artifacts.json)
retain both widths and fresh ten-release PE/name/width/size/SHA checks against
S20. All source/executable inputs, PC 0564/MyNES 0044 identities, INIs/media/
ROMs are unchanged. No artificial rebuild/deployment or receiving App P is
necessary. Both original graphs retain default profile and persistent cache.

Production is +0/-0/net 0; tests/registration +254/-0/net +254 (252-line unit
and two registration lines); manifest +2/-1/net +1. Counts exclude records
and artifacts. The test increase is below the initial 300--500-line estimate
because the existing fixture/transfer checker is reused without modification.
No production helper/state/interface or test framework is added. No physical
desktop/RDP, audio, fresh full external integration suite, complete V30
eligibility or game execution is claimed.
