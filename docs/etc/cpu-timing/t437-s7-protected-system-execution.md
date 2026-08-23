# T437 S7 80386DX protected system execution

S7 executes and observes all 115 generated protected segment, descriptor,
system and special-register keys through the sole Core retirement publisher.
It does not claim S8 final-result publication.

The retained Intel *80386DX Microprocessor Programmer's Reference Manual*
(230985-003, 1990), Chapter 17, is the timing authority. Independent 80386
protected fixtures provide real code/data descriptors, LDT and TSS descriptors,
and a loaded task register before observing each relevant instruction.

## Manual-first corrections

- LSL byte-granular timing is `21/22`, not `20/21`.
- CLTS is 6 clocks; MOV CR0,r32 is 11 clocks.
- LMSW is `11/14`; LLDT is `20/24`.
- Protected LDS/LES/LSS are `26/28`; LFS/LGS are `29/31` by operand size.

The runner observes register, memory, legal segment-override and operand-size
contexts for ARPL; LAR/LSL; VERR/VERW; descriptor-table, LDT and task-register
operations; MOV special registers; and all five long-pointer loads. STR is
observed only after the fixture executes a real, unobserved LTR success path.

```text
M5:T437:S7:I386-PROTECTED-SYSTEM-OBSERVED:115:canonical=115
M5:T437:S6:I386-PROTECTED-CONTROL-COVERAGE:PASS:canonical=89
M5:T437:S2:I386-RESULT-PRODUCER:PASS:observed=1409:canonical=1410
```

The one unobserved canonical key is ESC, deliberately handled by the separate
CPU/FPU handoff assertion because Chapter 17 delegates its execution clocks to
the selected coprocessor.
