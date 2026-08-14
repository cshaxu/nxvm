# T361 S4: 80286 NOP source and consumer audit

## Primary-source decision

The primary source is Intel, *80286 and 80287 Programmer's Reference Manual*,
order number 210498-005 (1987), available as the
[scanned manual](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf).
The review used the source only as a read-only research input; no source page,
binary, code, or derived third-party material is committed.

| Manual location | Semantic role | Observation | Disposition |
| --- | --- | --- | --- |
| Chapter 3, section 3.4.3, printed p. 3-15 (PDF p. 72) | Basic-instruction overview and illustrative example | After describing `NOP` as a one-byte filler that changes only IP, the example states two clock cycles. | Retained as the conflicting overview statement; it is not a per-opcode timing table and creates no second runtime owner. |
| Appendix B, `NOP--No Operation`, printed p. B-78 (PDF p. 286) | Per-instruction opcode record | Opcode `90`, instruction `NOP`, has a `Clocks` column value of three, together with its operation and exception entries. | Selected timing authority for the existing profile-local opcode allocation. |

The two statements remain textually inconsistent, but they have different
roles inside the same manual. The owner-approved T360 rule selects the
available per-instruction opcode record instead of combining it with a prose
example. Therefore the allocation is retained as **80286 `90 NOP = 3` CPU
clocks**. This is a source-backed instruction-retirement value only; it does
not claim bus, prefetch, DMA/HOLD, device, host-time, or cycle-exact behavior.

## Consumer and owner sweep

The sweep used:

```text
rg -n -i "80286.*nop|nop.*80286|NOP = 3|NOP = 2|CORE_MACHINE_SOURCE_TIMING_NOP" src tests CMakeLists.txt cmake docs
```

| Consumer / record | Result | Disposition |
| --- | --- | --- |
| `src/core/machine/machine.c`, `core_machine_80286_source_timing_ledger` | One `CORE_MACHINE_SOURCE_TIMING_NOP` ledger entry holds `3u`; the comment now cites Appendix B p. B-78 and captures the Chapter 3 context. | Retained; sole numeric 80286 NOP source. |
| `src/core/machine/machine.c`, `core_machine_80286_source_instruction_cost` | `0x90` routes to the named source-timing enum; a prefixed form is not admitted by the shared primary classifier. | Retained; no duplicate scalar or fallback. |
| `tests/machine/core_machine_80286_instruction_timing_ledger_smoke.c` | The direct 80286 `90` case expects three elapsed ticks, alongside distinct non-NOP cases and budget/reset checks. | Retained as the focused runtime proof. |
| `cmake/verify_t360_timing_source_inventory.cmake` | Names the 80286 source-cost owner but introduces no NOP number. | Retained as static inventory coverage. |
| Generic instruction, ledger, and real-mode tick smokes named by T360 S2 | Their existing 80286 NOP paths consume the source ledger; no independent two-clock source exists. | Retained; run through the current gate when a runnable path changes. |
| Scheduler, trace, and device records | Search found only evidence descriptions of the source-derived elapsed-tick consumer boundary. No scheduler or device assigns a NOP duration. | Not an independent allocation; retained boundary. |
| Historical T357/T359/T360 records and the T361 proposal | Reference the former unresolved conflict or its three-clock provisional decision. | Historical records are preserved; current TODO debt is removed and this audit is the current resolution record. |

No 8086, 80186, 80386, bus, device-service, exception, or dynamic-arithmetic
consumer was changed. Those forms remain owned by their respective ledgers and
Queue candidates.

## Verification record

- `ninja -C build\\mingw-gcc-x64 core-machine-80286-instruction-timing-ledger-smoke core-machine-instruction-timing-ledger-smoke core-machine-real-mode-tick-smoke verify-t360-timing-source-inventory verify-documentation-governance` passed. The inventory and documentation checks passed during the target build.
- `build\\mingw-gcc-x64\\core-machine-80286-instruction-timing-ledger-smoke.exe` passed with `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`; its direct `90` case retains the three-tick assertion.
- `build\\mingw-gcc-x64\\core-machine-instruction-timing-ledger-smoke.exe` passed with `M5:T357:S3:INSTRUCTION-TIMING-LEDGER:OK`.
- `build\\mingw-gcc-x64\\core-machine-real-mode-tick-smoke.exe` passed with `M5:T218:S2:REAL-MODE-TICKS:OK`.
- `git diff --check` passed before staging. The change modifies a source comment and documentation only; it does not alter a runnable path, so the packet's conditional full-current-gate requirement is not triggered.

The task does not add a new runtime marker because it intentionally leaves the
selected instruction behavior unchanged.
