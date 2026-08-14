# T357 S6: 80286 Instruction-Timing Ledger

## Authority and allocation

S6 uses the Intel *80286 and 80287 Programmer's Reference Manual*, Appendix B,
as the authority for every selected opcode with an Appendix-B row.  Its clock
counts describe the maximum execution rate under the manual's no-bus-delay
assumption; they do not allocate memory-read wait states, prefetch starvation,
HOLD/DMA, device service, or exception/interrupt delivery.

| Selected form | Appendix-B allocation | Construction at the sole retirement publisher |
| --- | ---: | --- |
| `NOP` | 3 | formal opcode-table row |
| `CLC` | 2 | direct row |
| `MOV r,imm`; register-to-register `MOV` | 2 | direct rows |
| `MOV r/m,r` memory | 3 | plus applicable EA and odd-word additions |
| `MOV r,r/m` memory | 5 | plus applicable EA and odd-word additions |
| `MOV AL/AX,moffs` | 5 | plus odd-word addition for `AX` |
| `MOV moffs,AL/AX` | 3 | plus odd-word addition for `AX` |
| `MOVSB` | 5 | direct row |
| `REP MOVSB` | `5 + 4 * CX` | first retired primitive receives 5+4; continued primitives receive 4 |
| conditional short branch | taken 7; not taken 3 | post-execution fall-through comparison |
| immediate-port `IN`; DX-port `IN` | 5; 5 | direct rows |
| immediate-port `OUT`; DX-port `OUT` | 3; 3 | direct rows |

Appendix B directs one additional clock for a 16-bit base-plus-index-plus-
displacement effective address and two additional clocks for each odd physical
16-bit memory-based operand reference.  S6 represents those additions only on
the selected 16-bit ModRM/moffs forms.  Segment override, wait-state, and
unallocated-prefix timing are not inferred; they receive the existing explicit
one-tick unallocated disposition.

The manual prose states a conflicting two-clock value for `NOP`.  The owner
selected the formal Appendix-B table row (`NOP = 3`) for S6.  The later
independent-source reconciliation is recorded in
[`TODO.md`](../../states/TODO.md).

## Construction and sweep

`core_machine_80286_source_instruction_cost()` is intentionally distinct from
the shared 8086/80186 construction.  The latter has a different EA table,
segment-override addition, and odd-word addition.  The 80286 helper owns only
the Appendix-B base-index-displacement and odd-word rules, while all profiles
continue to publish elapsed ticks at the same successful CPU-retirement owner.

The profile dispatch and maximum budget are both updated: the largest selected
single 80286 retirement is the first `REP MOVSB` primitive at nine ticks.
The retained generic timing and real-mode tick smokes now deliberately use the
80286 source ledger rather than a compatibility cadence; their formerly
synthetic expected values were reconciled to the ledger and they retain their
fault, stop, reset, and publication checks.

## Focused proof

`core-machine-80286-instruction-timing-ledger-smoke` uses deliberately
non-source configuration surcharges and a bound execution provider.  It proves
the selected rows above, including direct memory, base+index+displacement plus
odd-word memory, odd moffs, a three-primitive `REP MOVSB`, real port-provider
calls, fallback, pre-386 prefix fault/no publication, reset, requested stop,
budget preflight, elapsed-tick overflow, and one successful post-retirement
publication path.  The historical generic timing and real-mode-tick smoke
independently prove their reconciled source expectations.

The S6 owner smoke is target-local strict under GCC/Clang and is classified in
the T344 historical-fixture verifier.  No public ABI, second time writer,
device scheduler, or external source asset is added.

## Transfers

Unallocated opcode forms, segment-prefix timing, memory wait states, prefetch,
HOLD/DMA, port/device service, physical bus ownership, and exact
fault/interrupt timing remain transfers to the existing timing and bus debt.
The Appendix-B-versus-prose `NOP` discrepancy remains a later source
reconciliation task; it does not reopen the selected Appendix-B allocation.
