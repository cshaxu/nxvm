# M5 NXVM PC/AT Hardware Convergence

## Scope

This is the ROI-ordered queue for remaining M5 NXVM PC/AT work. It does not
redefine the roadmap, start a later milestone, or define NXVDM completion.
Completed M5 evidence is summarized in [M5 History](../history/m5.md); this
document contains only the current baseline and remaining queue.

The retained baseline is **T262**: `nxvm.exe` preserves its full-PC boot,
Console, debugger, and current GCC/CTest gate. The current target is
`vm-0-5-0262`. Checked memory, immutable ROM mapping, bounded 80386 paging,
bounded 16-bit protected privilege/IDT delivery, bounded 16-bit far-JMP task
switching, the exact-8087 finite baseline, and the sole VM composition
lifecycle remain owned boundaries. Static/ownership checks and 97/97 CTest
cases passed. Each admitted task
follows the hardware-device verification template and must preserve that
baseline.

## Remaining Queue

| Task | Dependency | Deliverable and stop condition |
| --- | --- | --- |
| T263 | Corpus-specific 286/386 gap | **S1 active:** the first admitted family is 80286 protected-mode `ARPL r/m16,r16`, selected because its metadata says 80286 while the executor currently faults it below 80386. Complete its focused corpus before admitting another family; do not attach it to task-switch work. |

## Constraints

- Keep one active subtask. Each task begins with a bounded S1 contract, owner,
  probe, deferred behavior, and stop condition.
- `core/machine` owns generic guest state and elapsed time; `vm/machine` owns
  VM-only devices; profiles provide frozen topology and ROM/firmware contents;
  platform never mutates guest state.
- Do not add a second executor, machine/session, device-state mirror,
  VM-side instruction loop, host-clock guest shortcut, global/TLS selector, or
  unapproved NXVM Console/debugger/startup/boot behavior change.
- A behavior-changing task records its focused evidence, retained regression
  matrix, source commit, and verified task artifact before completion.
