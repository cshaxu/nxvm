# T435 S2: Four-Profile Implementation-Readiness Audit

## Decision

S2 is ready to close as an evidence, current-state and implementation-handoff
stage. It does **not** claim that current timing code is conforming. Its result
is a frozen, mechanically checked implementation universe for all successful
retirements, with every present gap retained as a nonconforming key for B0--B4.

## Authority and frozen universe

Each profile's S1 ledger supplies the sole timing source: Intel manual exact
constant/formula rows are L3; 8086's 16 Group-3 keys retain the named
`L2-86BOX-8086-G3` model; 80186's 26 range-only keys retain the named fixed
midpoint; 80286 has no L2 or L1 base row; 80386DX has two explicit External-L2
rows (`WAIT` and `ESC`). The S1 ledgers separately
exclude external waits, arbitration, prefetch availability and event delivery.

| profile | base | legal contexts | legal combinations | total canonical keys | S1 level decision |
| --- | ---: | ---: | ---: | ---: | --- |
| 8086 | 246 | 406 | 401 | 1,053 | 230 L3, 16 L2:G3 base keys |
| 80186 | 279 | 235 | 89 | 603 | 253 L3, 26 L2:midpoint base keys |
| 80286 | 286 | 459 | 62 | 807 | all L3 |
| 80386DX | 451 | 962 | 0 | 1,413 | 449 L3, 2 L2 |
| **all profiles** | **1,262** | **2,062** | **552** | **3,876** | no anonymous L1 |

The profile verifiers and the shared
`tools/Verify-CpuTimingManifestContract.ps1` derive these figures directly
from the current manifests. The shared verifier rejects duplicate base/context
keys, absent required record fields and any profile total that differs from its
frozen expected base/context/combination count.

## Current-state audit result

The source audits preserve the existing routing facts rather than upgrading
selector presence to correctness. Every generated key has source rule, current
route, status, batch and regression identity. The current result is zero
conforming keys: current code still has explicit wrong-value, unallocated,
missing-input and missing-test states. This is a complete inventory of current
gaps, not a transfer or a future audit obligation.

Representative legacy smoke tests remain useful functional evidence only. They
cannot change an individual manifest key's status until B0 emits a result under
the shared result contract.

## Architecture and implementation handoff

The approved T435 proposal places the new Core-private `cpu_timing.c` between
decoder-owned inputs and the one retirement publication seam. It transfers no
CPU instruction rule to board work: CPU internal constants/formulae, operand
and mode/path inputs remain T435; READY/HOLD/BUSLOCK, DMA, prefetch, device
completion and physical cycles remain their named transaction/device/board
receivers.

The shared result contract makes B0's first deliverable testable: provisional
S2 canonical records must become registered decoder forms and real per-key
results containing ticks, formula inputs, origin and unallocated=false. It
forbids a second successful-retirement selector and derives status from results
rather than hand editing. S4--S7 each consume their complete profile row above;
S8 consumes the four result sets and can close only at zero nonconforming keys.

## S2 closure predicate

S2 is complete only because all of the following are true:

1. every S1 successful-retirement rule has a named L3/L2 disposition and no
   source-less L1 fallback;
2. every S2 base/context/combination key is finite, legal under the shared
   context contract, source-linked and assigned to an implementation batch;
3. the independent profile verifiers and shared canonical contract verifier
   agree on all 3,295 keys; and
4. B0--B4 have one architectural owner, result schema and final zero-gap
   predicate without absorbing board/transaction work.

Markers: `M5:T435:S2:FOUR-PROFILE-CLOSURE-AUDIT:OK`;
`M5:T435:S2:B0-IMPLEMENTATION-READY:OK`.
