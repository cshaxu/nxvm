# T437 S6 80386DX protected-control execution

S6 executes and observes the complete protected-control partition through the
sole Core retirement publisher.  It covers all 89 generated protected,
privilege, gate, task and VM86 transfer keys; it does not claim S7 protected
segment/descriptor/system forms or any board/bus timing.

## Manual-first correction

The retained Intel *80386DX Microprocessor Programmer's Reference Manual*
(order 230985-003, 1990), Chapter 17 `IRET` algorithm, makes protected return
to VM86 a 32-bit frame: it reads EFLAGS at `SS:[ESP+8]`, then performs the
doubleword return and segment-frame pops.  The former successful
`I386-IRET-PM-VM86-SIZE16` context was therefore invalid.  Under the owner's
manual-is-oracle direction, S6 removed that key rather than manufacturing a
16-bit success result.  The manifest now has 450 base keys plus 960 legal
contexts, or 1,410 canonical keys.

## Execution and timing ownership

The runner supplies concrete protected descriptors, selectors, stacks,
call-gate parameters, task-state segments, task gates, VM86 state and IDT
entries.  Every observed key has a single classified Core control-stack
retirement observation, including the next-lexeme component where applicable.

- Protected direct and memory far CALL/JMP, same- and outer-privilege RETF and
  IRET execute with real 16/32-bit encodings and frames.
- Same- and outer-privilege call gates execute with zero and nonzero copied
  parameters; the decoder accepts 32-bit call-gate targets.
- Protected and VM86 INT3, INT imm8 and INTO exercise same/inner privilege
  delivery, including VM86 IOPL prerequisites; all three also exercise IDT
  task-gate delivery.
- TSS and task-gate CALL/JMP, memory forms and operand-size contexts execute
  against real task descriptors.  Source timing distinguishes TSS from task
  gate and direct from memory selection.
- IRET task and protected-to-VM86 paths execute with their manual-required
  frames; task-gate direct SIZE16 and SIZE32 are individually observed.

The scope predicate in the runner emits every unobserved S6 key.  The final
review found two task-gate direct operand-size contexts and three IDT task-gate
software-interrupt contexts, added their real encodings and then reached full
S6 coverage.  The partition verifier's separate `IRET-REAL` classification was
also corrected to S5, matching the retained S1 boundary:

```text
M5:T437:S2:I386-RESULT-PRODUCER:PASS:observed=1307:canonical=1410
M5:T437:S6:I386-PROTECTED-CONTROL-OBSERVED:89:canonical=89
M5:T437:S6:I386-PROTECTED-CONTROL-COVERAGE:PASS:canonical=89
M5:T437:S2:I386-INCOMPLETE-RESULT-REFUSED:PASS
```

Focused command:

```powershell
cmake --build build/t437-winlibs-gcc --target core-machine-80386-timing-manifest-runner -- -j4
& .\build\t437-winlibs-gcc\core-machine-80386-timing-manifest-runner.exe
```

## Similar-issue sweep and transfer

The sweep is mechanically enforced by `timing_80386_manifest_key_is_s6()` over
the complete generated 80386DX catalog.  It found no remaining S6 production or
recipe omission after all five task-gate contexts were covered.  Direct selector
parsing, task-gate descriptor selection, call-gate
parameter timing and 32-bit call-gate decoding were all corrected in the one
Core control-stack timing path.

S7 retains the 20 unallocated protected segment, descriptor, system and
special-register keys.  S8 retains complete-result publication, cross-profile
regression and final task closure.
