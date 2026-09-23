# T388 S6: 80286 Appendix-B Context

`M5:T388:S6:80286-APPENDIX-B-CONTEXT:OK`

## Reconciliation

The current 80286 classifier already consumes completed post-refresh state for
selected Appendix-B forms; it is not a blank system-instruction implementation.
The retained owner smoke proves successful register/memory, protected/real,
effective-address and odd-word cases for `62`, `63`, `8C`, `8E`, `C4/C5`,
`0F 00`, `0F 01`, `0F 02`, `0F 03` and `0F 06`. The classifier uses completed
ModR/M, `flagMem`, old protected mode and CPL where those are sufficient.

| Family | Current successful context | S6 disposition |
| --- | --- | --- |
| `62 BOUND`; `8C/8E MOV Sreg`; `C4/C5 LES/LDS`; Group-2 shifts | Legal ModR/M and register/memory shape; EA and odd-word state where applicable. | Existing exact owner/row retained. Invalid register forms remain non-retiring or unallocated. |
| `63 ARPL` | Protected successful register/memory form. | Existing `10/11` classifier selection retained; real-mode result is not backfilled. |
| `0F 00` selectors | SLDT/STR, LLDT/LTR, VERR/VERW with successful protected/CPL form. | Existing ModR/M, old mode and CPL capture selects current Appendix-B rows. |
| `0F 01` table/control; `0F 06 CLTS` | Legal memory/register shape and real/protected CPL condition. | Existing classifier selection retained; no delivery/mode-transition time is manufactured. |
| `0F 02 LAR` | Successful protected selector query, register/memory form. | Existing `14/16` selection retained. |
| `0F 03 LSL` | Successful selector query but descriptor byte/page granularity can select distinct Appendix-B rows. | Explicit receiver: current retirement state has no safe descriptor-granularity capture. |
| Any nonzero prefix, residual `0F`/default lookup route | Successful execution outside a selected exact source context. | Explicit prohibition; no generic prefix/default source value. |
| `WAIT`/ESC, fault/delivery, waits/HOLD/device service | No CPU-only Appendix-B retirement value. | Existing x87/physical/device receivers. |

## Decision

No new scalar or classifier branch is admissible in S6. The candidate forms
that have complete state were already routed by the current Core owner and
verified by `core-machine-80286-instruction-timing-ledger-smoke`. The one
remaining named Appendix-B context gap is `LSL` descriptor granularity; it
requires a later nonpublishing descriptor capture rather than a local timing
fallback. Prefix/default success remains prohibited under T388 S3.

## Source Anchors

The partition is bound to `core_machine_80286_source_instruction_cost` and
its `case 0x0fu:`, `case 0x62u:`, `case 0x63u:`, `case 0x8cu:`,
`case 0x8eu:`, `case 0xc4u: case 0xc5u:` and `prefixes != 0u` paths.
Every **residual unallocated** success uses
`CORE_MACHINE_SOURCE_UNALLOCATED_TICKS` only as the explicit nonphysical
receiver; retained exact rows do not pass through that marker.
## Static Boundary

`verify-t388-80286-appendix-b-context` binds the existing classifier,
80286 owner smoke and this decision to the `0F`, `62`, `63`, `8C`, `8E`,
`C4/C5`, `LSL`, prefix and source-unallocated anchors. It is a recurrence
detector, not a timing proof or physical-clock claim.