# T357 S7: 80386 Protected And VM86 Port-I/O Timing

## Authority and selected allocation

For every selected row that appears in the Intel 80386 PRM Appendix B opcode
clock table, S7 uses the Appendix-B value. The instruction-reference pages
are retained only as navigable copies of the same `pm` row and its privilege
footnotes: [IN](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/IN.htm)
and [OUT](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/OUT.htm).
No compatibility timing configuration supplies these values.

| Selected ordinary form | `CPL <= IOPL` | permission-map / VM86 | retirement ledger forms |
| --- | ---: | ---: | --- |
| `E4`/`E5` immediate-port `IN` | 6 | 26 | `IN_IMMEDIATE_PROTECTED`, `IN_IMMEDIATE_PERMISSION` |
| `EC`/`ED` DX-port `IN` | 7 | 27 | `IN_DX_PROTECTED`, `IN_DX_PERMISSION` |
| `E6`/`E7` immediate-port `OUT` | 4 | 24 | `OUT_IMMEDIATE_PROTECTED`, `OUT_IMMEDIATE_PERMISSION` |
| `EE`/`EF` DX-port `OUT` | 5 | 25 | `OUT_DX_PROTECTED`, `OUT_DX_PERMISSION` |

The largest selected successful retirement is therefore the 27-clock
permission-map `IN DX` path. The profile maximum and its preflight test are
raised from 22 to 27; a 26-clock budget publishes nothing and a 27-clock
budget retires exactly that form. If a later source reconciliation finds an
Appendix-B/main-text disagreement for one of these rows, Appendix B remains
the allocation authority and the disagreement must receive a bounded TODO
revisit rather than an inferred compromise.

## One permission owner and VM86 repair

`_kpa_test_mode()` already sends both `CPL > IOPL` and VM86 I/O through
`_kpa_test_iomap()`. Before S7, that owner immediately raised `#GP` whenever
`EFLAGS.VM` was set, so no VM86 instruction could reach a permitted TSS map.
That contradicted the 80386 PRM protection rule: virtual-8086 I/O consults the
map without regard to IOPL. S7 removes only that premature VM86 rejection.
The profile gate, valid busy-32-bit TSS requirement, TSS-limit and map-bound
checks, per-byte span checks, and `#GP` atomicity remain in the same owner.

The caller sweep covers `_p_input()` and `_p_output()` and therefore both
ordinary `IN`/`OUT` and string `INS`/`OUTS`; it introduces no second map,
TSS-layout change, provider ABI, or fault-delivery path.

## Focused proof

`core-machine-80386-protected-io-timing-smoke` has an intentionally unrelated
compatibility timing configuration and a bound port provider. It proves all
eight selected ordinary forms at protected `CPL <= IOPL`, protected
permission-map, and VM86 permission-map dispositions; provider publication
and the exact selected elapsed-tick cost; denied ordinary I/O zero cost and
zero provider publication; and the 26/27 budget boundary.

The same owner smoke executes VM86 `INSB` and `OUTSB` with an allowed bitmap
and verifies the corresponding provider effect and index publication. It
then sets the port bit and proves synchronous denial with no provider effect
or string-index publication. This is a caller-owner regression, not a timing
allocation for string forms.

The focused marker is `M5:T357:S7:80386-PROTECTED-IO-TIMING:OK`. The new
direct-construction smoke is registered as a target-local strict current-gate
test and classified by the T344 fixture-shape verifier.

## Appendix-B timing-consumer reconciliation

S7 also closes the remaining observable consumers of S6's authoritative
80286 `NOP = 3` Appendix-B allocation. No source row was changed back to a
configuration-derived one-clock value. The affected tests now assert the
source-derived result instead:

- scheduler, competition, transaction lifecycle, rational clock, elapsed-time,
  timing-checkpoint, timeline, RTC-storage, and input/display traces account
  for a three-clock `NOP`; trace fixtures retain all due-tick records rather
  than silently truncating them at their former one-clock capacity;
- PIT-divider observations retain their `1/4` domain ratio and assert the
  resulting three and six PIT advances after four 80286 `NOP`s, rather than
  changing the clock ratio to preserve old transition counts;
- the DMA/RTC authority path records `NOP(3) + HLT(1) = 4` elapsed clocks and
  its visible RTC second value `04`; and
- the firmware timer smoke was rebuilt and rerun unchanged: its stepped
  execution path is not an 80286 `NOP` assumption and remains valid.

These are test-contract and trace-capacity repairs only. They do not create a
second elapsed-time writer, alter device clocks, or weaken a timing assertion.

## Transfers

This S closes the S3 transfer only for selected ordinary port I/O in its
successful real/protected/permission-map/VM86 forms. `INS`/`OUTS` timing,
near and `JCXZ` branches, non-`MOVSB` strings, unselected CPU forms, memory
wait states, prefetch, physical bus/HOLD/DMA arbitration, device latency,
fault/interrupt-cycle timing, VME/PVI, and cycle-exact timing remain outside
T357's finite corpus.
