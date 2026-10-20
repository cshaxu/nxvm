# T4 S11 Sprite-Protection Upload L2

## Scope

S11 resolves only the selected set's bounded Board-visible receiver at
`0x6a0--0x6bf`. It does not emulate a sprite-protection algorithm, add a
general control-RAM window, or claim original-hardware accuracy.

## L2 Basis

The inspected MAME Raiden II address map at revision
`8d457b2ecd241b63cab1d9c0c07024365a261398` assigns the complete interval to
write-side sprite-protection upload receivers. That address-ownership fact is
used only as an L2 boundary: the local model is a reset-cleared, opaque,
write-only Board aperture. No MAME source, algorithm, table, ROM content or
protected runtime data is imported.

Reference consulted: `src/mame/seibu/raiden2.cpp` at that revision.

## Local Contract

- `0x6a0--0x6bf`: contained writes are accepted and retained privately until
  Board reset.
- reads and inspection remain rejected.
- neighbouring `0x69f` and `0x6c0` accesses remain rejected.
- no public getter, external ABI, Shared component, second DMA path or
  execution path was added.

## Dual-width Forward Progress

Before S11, the selected-set probe stopped during COP command `0x5205` at
`0x6a0`. With the aperture, both x64 and x86 reach the same independent next
stop at `0x600`:

```text
reason=7 detail=2147483648 fault-code=1536
cop-operation=0 cop-value=0 cop-physical=0 cop-status=0
executed=71260 ticks=1130450
```

`0x600` is a separate Video/CRTC receiver. It is not accepted by this S and
must be designed and qualified under the Video owner.

## Verification

- Focused `myraidenii-core-board-bus-smoke`: passed on x64 and x86. It covers
  contained write acceptance, read/inspection refusal, both outer boundaries
  and reset-admission.
- Selected-set startup probe: passed identically on x64 and x86 with the
  bounded result above.
- Complete repository-only unit suites: 642/642 passed on x64 (80.34s) and
  642/642 passed on x86 (90.16s), with no failures or skips.
