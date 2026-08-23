# T437 S8 80386DX result closure

S8 publishes the complete 1,410-key 80386DX result artifact from the accepted
S3--S7 partitions.  The normal 1,409 keys are real classified CPU retirement
observations.  `I386-ESC` is the one explicit non-CPU clock-domain record.

The retained Intel *80386DX Microprocessor Programmer's Reference Manual*
(230985-003, 1990), Chapter 17 processor-extension row states that ESC clock
counts come from the 80287/80387 data sheets.  Therefore the final ESC record
has `timing_domain: mcp` and `ticks: null`; it retains a real
`CPU_FPU_COMMAND` handoff, the selected 80387 profile, and the verified FADD
range of 12--26 MCP clocks.  Its executed `D8 C0` bytes are recorded too. It
is not a zero-clock CPU result and its MCP interval is not added to ESC CPU
retirement.

The shared result verifier now selects the canonical catalog for the result
file's profile and checks profile, level, source rule and context as well as
key uniqueness and timing-domain validity.  Its CPU default preserves the
existing 8086, 80186 and 80286 result artifacts; only `I386-ESC` may use the
MCP-domain exception.

```text
M5:T437:S2:I386-RESULT-PRODUCER:PASS:observed=1409:canonical=1410
M5:T437:S8:I386-RESULT-CLOSURE:PASS:canonical=1410:cpu=1409:mcp=1
M5:T437:S3:I386-ESC-HANDOFF:PASS
CPU timing results verified: conforming_keys=1410
CPU timing results verified: conforming_keys=1053
CPU timing results verified: conforming_keys=616
CPU timing results verified: conforming_keys=771
```

The generated [result artifact](t437-s8-80386-timing-results.json) contains
exactly one record for each canonical key.  No public ABI, board/event policy,
third-party source, firmware or guest media changes.
