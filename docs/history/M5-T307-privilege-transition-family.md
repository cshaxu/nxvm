# M5 T307: Privilege-Transition Family

## Closure Summary

T307 completed the admitted 80386 protected, non-virtual-8086 CPL3-to-CPL0
entry family in the single core CPU executor. It added 32-bit IDT interrupt and
trap-gate entry through `TSS32 SS0:ESP0`, and 32-bit call-gate entry with the
admitted bounded parameter-copy forms. Both planners preflight old and new
stacks, descriptors, selector/cache candidates, and frame bounds before
publishing state. No second executor, state owner, public ABI, product UX, or
host shortcut was introduced.

## Semantics And Corrections

Intel's *80386 Programmer's Reference Manual* is the semantic authority.
Read-only comparisons used Bochs 2.6 and PCjs 2.00.0 behavior paths recorded
in the [T307 evidence](../etc/evidence/t307-privilege-transition-admission.md).
No reference source was copied.

The IDT planner establishes the five-dword 32-bit privilege-change frame and
distinguishes software DPL checks from external-origin delivery, with the gate
defined IF/TF result. The call-gate planner validates the gate, target code,
TSS-provided stack, parameter source, and destination frame before commit. Its
S5 correction preserves the Intel classification for an invalid target SS
type or DPL as `#TS(SS selector)`, while target SS non-presence remains
`#SS(SS selector)`. Focused probes preserve the first fault and entry
CS/SS/EIP/ESP/EFLAGS/CPL plus relevant descriptor accessed bytes for rejected
entries.

## Verification And Observation

- The current artifact is `build/output/nxvm_0_5_0307.exe`.
- SHA-256:
  `933A06EAED336A79A7855A8B5D9A93C53DBDB1CB1600994E3B27CA141D9235DA`.
- `current-gates-gcc` passed all 51 static/governance targets and 137/137
  CTests.
- The selected T307 focused and retained T304/T305/T306/T288/T260/T261 probe
  run passed 9/9 CTests.
- Documentation governance and `git diff --check` passed during S7 closure
  preparation.

One bounded, no-input, no-media observation launched the task artifact hidden.
After ten seconds it was alive with no discoverable host window handle; the
observer stopped only that process. No guest command or checkpoint was
obtained. This host-automation limitation is not product progress or a guest
regression.

## Deferred Boundaries

T307 does not integrate error-code or hardware-NMI privilege delivery, task or
virtual-8086 paths, paging policy, or new fault-delivery origins. Those remain
future unnumbered Queue admission work.
