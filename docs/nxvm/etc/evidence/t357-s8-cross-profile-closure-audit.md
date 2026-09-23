# T357 S8: Instruction-Timed Execution Closure Audit

## Closure decision

T357 closes its explicitly finite, instruction-timed corpus. This is a
profile-aware CPU form-cost prerequisite for physical timing; it is not an
assertion of full opcode timing, PC/AT bus timing, or cycle-exact execution.
All selected costs are chosen after successful CPU refresh and are published
only by `core_machine_run()` before its existing retire trace and scheduler
advance. The only production increment of `machine->elapsed_ticks` remains
that owner.

## Profile and mode reconciliation

| Profile | Accepted selected corpus | Source/evidence owner |
| --- | --- | --- |
| 8086 | `NOP`, `CLC`, selected register/memory/moffs `MOV`, direct/indexed EA and odd-word additions, `MOVSB`/`REP MOVSB`, short Jcc, and immediate/DX ordinary I/O. | S4, Intel Tables 2-20/2-21. |
| 80186 | The corresponding selected baseline, `MOV`, EA/odd-word, string, short-Jcc, and ordinary-I/O rows with its independent table values. | S5, Table 1-16. |
| 80286 | Appendix-B `NOP`, `CLC`, selected `MOV`, EA/odd-word, `MOVSB`/`REP MOVSB`, short Jcc, and ordinary I/O rows. | S6, Appendix B. |
| 80386 | Selected baseline/`MOV`/moffs, `MOVSB`/`REP MOVSB`, short Jcc lexical-preview, and real ordinary I/O rows; protected `CPL <= IOPL` plus protected/VM86 permission-map rows for immediate/DX ordinary I/O. | S3 and S7, 80386 PRM tables. |

For every profile the ledger has a private immutable form lookup, a bounded
maximum for preflight, a deterministic one-clock unallocated-form transfer,
and no selected-form dependency on the compatibility timing configuration.
Focused profile-ledger smoke covers source rows, fallback, profile-prefix
fault/no-publication, reset/replay, requested stop, budget preflight,
overflow, and the one post-retirement provider/scheduler publication path.

S7 additionally reconciles the one shared I/O-permission owner: permitted
VM86 ordinary and string I/O reaches the existing busy-32-bit-TSS bitmap;
denial has no successful timing or provider publication. This removes the
previous classifier/helper contradiction without creating a second map or
time writer.

## Appendix-B and consumer disposition

The selected 80286 Appendix-B opcode rows control S6 allocations. The only
observed table/prose conflict is `NOP`: Appendix B gives three clocks while
manual prose gives two. It remains the precise
`80286 Appendix-B timing-source reconciliation` TODO; S6/S7 do not silently
average or substitute the prose value.

Every current consumer that assumed a one-clock 80286 `NOP` was reconciled in
S7. Scheduler/timeline/trace/PIT/RTC/DMA observations now reflect the
three-clock source value without modifying a device-clock domain or adding a
second elapsed-time writer. The full gate exercises those consumers.

## Explicit transfers

The following remain outside the finite corpus and are still owned by the
named Queue/TODO receivers: unselected instruction forms and prefix/branch/
string breadth; memory and I/O wait states; physical CPU/DMA bus ownership,
HOLD and arbitration; prefetch/cache and phase/pin behavior; device service
latency; fault/interrupt cycle costs; x87 timing; VME/PVI; host time; and
Windows compatibility. The next queue candidate, Cross-Mode Mechanism
Coherence, audits CPU mode-classifier/helper consistency before bus-timed
PC/AT work; it does not expand T357's timing corpus.

## Verification record

- Focused S7 marker: `M5:T357:S7:80386-PROTECTED-IO-TIMING:OK`.
- Static T344 fixture-shape verifier: 60 direct constructors, 22 shared tails,
  38 retained shapes.
- Full direct `ctest -L current-gate --output-on-failure -j 4`: 240/240 passed.
- Documentation governance and `git diff --check`: passed.
- Developer artifact: `build/output/nxvm_0_5_0357.exe`, SHA-256
  `83A249015C03CF875896A440D5B43A430C2D875A63F0BE4B9709014A753CAD95`.
