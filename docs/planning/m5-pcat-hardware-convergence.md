# M5 NXVM PC/AT Hardware Convergence

## Scope

This is the ROI-ordered queue for remaining M5 NXVM PC/AT work. It does not
redefine the roadmap, start a later milestone, or define NXVDM completion.
Completed M5 evidence is summarized in [M5 History](../history/m5.md); this
document contains only the current baseline and remaining queue.

The retained baseline is **T257 S4**: `nxvm.exe` preserves its full-PC boot,
Console, debugger, and current GCC/CTest gate. The current target is
`vm-0-5-0255`, with static/ownership checks and 91/91 CTest cases passing.
Checked memory, immutable ROM mapping, atomic entry plans, and the sole VM
composition lifecycle remain owned boundaries. Each admitted task follows the
hardware-device verification template and must preserve that baseline.

## Remaining Queue

| Task | Dependency | Deliverable and stop condition |
| --- | --- | --- |
| T258 | T257 | Add only bounded 80386 CPL0 4 KiB paging and narrowed `CR0`/`CR2`/`CR3` forms. `#PF` remains a core diagnostic stop; CPL3, IDT delivery, and TSS I/O are deferred. |
| T259 | T258 | Add the bounded protected-privilege and exception-delivery prerequisite: admitted CPL/RPL/DPL rules, an outer-privilege entry/return corpus, and minimal protected IDT-gate delivery. No task switch. |
| T260 | T259 | Add TSS I/O-permission-map behavior only through the real CPL3 corpus. It validates TSS/bitmap bounds and I/O allow/deny; it does not switch tasks. |
| T261 | T260 | Add bounded hardware task switching, task-state save/restore, and busy-state behavior through a separate corpus. |
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
