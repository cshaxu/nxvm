# M5 NXVM PC/AT Hardware Convergence

## Scope

This is the ROI-ordered queue for remaining M5 NXVM PC/AT work. It does not
redefine the roadmap, start a later milestone, or define NXVDM completion.
Completed M5 evidence is summarized in [M5 History](../history/m5.md); this
document contains only the current baseline and remaining queue.

The retained baseline is **T261 S5**: `nxvm.exe` preserves its full-PC boot,
Console, debugger, and current GCC/CTest gate. The current target is
`vm-0-5-0261`. Checked memory, immutable ROM mapping, bounded 80386 paging,
bounded 16-bit protected privilege/IDT delivery, bounded 16-bit far-JMP task
switching, and the sole VM composition lifecycle remain owned boundaries.
Static/ownership checks and 95/95 CTest cases passed. Each admitted task
follows the hardware-device verification template and must preserve that
baseline.

## Remaining Queue

| Task | Dependency | Deliverable and stop condition |
| --- | --- | --- |
| T262 | T257 | Add present-FPU state, operations, exceptions, and `FWAIT`; FPU-none escape handling is not present-FPU support. |
| T263 | Corpus-specific 286/386 gap | Admit each remaining 286/386 instruction family only through a separate failing corpus; do not attach it to task-switch work. |

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
