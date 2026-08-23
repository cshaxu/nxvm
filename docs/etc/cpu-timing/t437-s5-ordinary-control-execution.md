# T437 S5 80386DX ordinary control execution

S5 executes and observes every corrected ordinary-control key through the
existing sole Core retirement publisher.  It does not claim protected gate,
task, VM86 or board timing.

## Corrected partition and execution

The frozen 232-key figure omitted two manifest keys already selected by the
legal stack-memory segment-override policy: `PUSH M-SEGMENT` and
`POP M-SEGMENT`.  S5 therefore owns 234 keys.  The four protected/VM86/task
`INTO` keys remain S6.

The runner supplies real-mode stack and IVT mappings, concrete direct and
indirect targets, return frames, and `INTO` overflow state.  Each Jcc form is
run in both outcomes with the corresponding FLAGS; JCXZ/JECXZ/LOOP forms use
their actual counter/FLAGS preconditions.  Width contexts run with real prefix
encodings and continue to require a classified, non-unallocated form/origin.

```text
M5:T437:S5:I386-ORDINARY-CONTROL-OBSERVED:234
M5:T437:S5:I386-ORDINARY-CONTROL-COVERAGE:PASS:canonical=234
M5:T437:S5:I386-ORDINARY-CONTROL-INPUTS:PASS
M5:T437:S2:I386-INCOMPLETE-RESULT-REFUSED:PASS
```

The 80386 PRM RET table gives real-mode/same-level `RETF` and `RETF imm16`
their `18+m` row.  `m` remains the existing Core next-lexeme component input;
no host, prefetch or bus value is introduced.

## Transfer

S6 retains protected transfer, gate, privilege, task, VM86 and protected
interrupt paths.  S7 retains protected segment/descriptor/system forms.
