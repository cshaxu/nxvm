# M5 NXVM PC/AT Hardware Convergence

## Scope

This is the ROI-ordered queue for remaining M5 NXVM PC/AT work. It does not
redefine the roadmap, start a later milestone, or define NXVDM completion.
Completed M5 evidence is summarized in [M5 History](../history/m5.md); this
document contains only the current baseline and remaining queue.

The retained baseline is **T252 S3**: `nxvm.exe` preserves its full-PC boot,
Console, debugger, and current GCC/CTest gate. The current target is
`vm-0-5-0251`, with static/ownership checks and 86/86 CTest cases passing.
Checked memory, immutable ROM mapping, atomic entry plans, and the sole VM
composition lifecycle remain owned boundaries. Each admitted task follows the
hardware-device verification template and must preserve that baseline.

## Remaining Queue

| Task | Dependency | Deliverable and stop condition |
| --- | --- | --- |
| T253 | T233 | Extend ATA/IDE only through a declared feature matrix and failing corpus. IDE DMA, ATAPI, LBA48, and cache remain separate admissions. |
| T254 | T219, T220, T228 | Complete digital CGA only through bounded modes and 6845-visible behavior required by a declared corpus. Composite rendering remains optional. |
| T255 | M5 profile boundary | Define machine-profile admission, optional user-provided ROM manifests, topology/capability contracts, fixture corpus, and provenance before additional machine profiles. |
| T256 | T217--T225 evidence | Admit a higher timing-fidelity level only when a real corpus requires it. Instruction, bus, and cycle timing remain separate work. |
| T257 | Real-mode device baseline | Add bounded 286 protected-mode descriptors, exceptions, and control transfer with focused probes. |
| T258 | T257 | Add bounded 386 paging, CRx, CPL/IOPL, and TSS I/O-map behavior with focused probes. |
| T259 | T258 | Add task switching and remaining admitted 286/386 instruction families. |
| T260 | T257--T259 as applicable | Add present-FPU state, operations, exceptions, and `FWAIT`; FPU-none escape handling is not FPU support. |

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
