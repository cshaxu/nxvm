# T437 S8 80386DX result closure

S8 publishes the complete 1,413-key 80386DX result artifact from the accepted
S3--S7 partitions.  The normal 1,412 keys are real classified CPU retirement
observations. `I386-ESC` and `I386-WAIT` are the two explicit External-L2
records: the selected 80387 duration is a range, and the WAIT table gives only
a seven-clock minimum before external BUSY# deassertion.

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
M5:T437:S2:I386-RESULT-PRODUCER:PASS:observed=1412:canonical=1413
M5:T437:S8:I386-RESULT-CLOSURE:PASS:canonical=1413:cpu=1412:mcp=1
M5:T437:S3:I386-ESC-HANDOFF:PASS
CPU timing results verified: conforming_keys=1413
CPU timing results verified: conforming_keys=1053
CPU timing results verified: conforming_keys=616
CPU timing results verified: conforming_keys=771
```

The generated [result artifact](t437-s8-80386-timing-results.json) contains
exactly one record for each canonical key.  No public ABI, board/event policy,
third-party source, firmware or guest media changes.

## Final regression and review

The final S8 sweep rebuilt the existing 80386 timing smoke expectations from
the publisher's manual-selected source rules, rather than preserving stale
real-mode assumptions: privileged T359 S6 rows now execute under a CPL0
protected fixture, LSL byte-granular forms remain 21/22 clocks, and prefix or
prefetch-sensitive smoke rows use their observed retirement values. It also
repairs the current-gate inventories that had treated two auxiliary CTest
entries as canonical targets or assigned an x87 smoke a nonexistent #UD
source assertion.

Fresh Debug configuration, T337, T344 (290 canonical targets plus its two
auxiliary entries), T382, the full 292-test `current-gate`, the 80386 partition
and decoder-ledger checks all pass. The cross-profile result verifier accepts
the 80386, 8086, 80186 and 80286 artifacts at 1413, 1053, 616 and 771 records
respectively. These checks are task-closure evidence; no production CPU,
board, ABI or device route changed.
