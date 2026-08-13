# M5 T336: Four-Profile CPU Architecture Coverage

## Task Record

T336 is the first task in the M5 four-profile CPU program. Its S1 audit creates
the form/state ledger and exact implementation breakdown for Intel 8086, 80186,
80286, and 80386DX. It does not itself claim CPU completeness or change runtime
behavior.

The active S1 packet in [CURRENT](../states/CURRENT.md) was the contract. Its
retained [proposal](M5-T336-four-profile-coverage-proposal.md) records the
original candidate boundary.

## Completion Boundary

T336 closes only after its audit has made every in-scope instruction and
architecture-state row either evidenced, explicitly bounded, or transferred to
one precise next owner. The resulting ordered candidates, not this record,
govern later implementation admission.

## S1 Audit Result

S1 reconciled the sole primary and `0F` dispatch tables, profile metadata,
current-gate owner-smoke inventory, T309/T316/T322 and T320--T331 evidence,
the 80386DX closure map, histories, and CPU TODO ledger. The retained
[coverage ledger](M5-T336-four-profile-coverage-ledger.md) classifies every reachable
form family and vertical state transition as bounded evidence, partial with a
unique Queue owner, outside-80386, or an external-coprocessor boundary.

The next Queue candidate owns the common producer/frame/restart gaps, beginning
with real-mode vector-6 delivery. The later profile candidates then own exact
profile form reconciliation; no former ordinary-form smoke is promoted to a
four-profile claim merely by reachability.
