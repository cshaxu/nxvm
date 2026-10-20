# T4 S10 COP Failure Attribution

## Scope

S10 adds a bounded MyRaidenII Core diagnostic only. It does not accept a new
Board route, alter COP execution, expose Shared state, or retain guest data.

## Observation Contract

The COP records a failed pending operation's kind, command-or-mode value, first
failed DMA physical address, and existing `lib_status`. The Board copies that
single observation when it reports the already-existing machine fault. Reset
clears both COP and Board observations. The selected-set probe prints only the
four scalar facts; it never prints ROM contents, instruction addresses,
registers, or a trace.

## Dual-width Result

The owner-supplied archive was verified and only its two program lanes were
staged transiently. Both release probes reached the same already-existing
fault:

```text
reason=7 detail=2 fault-code=0
cop-operation=1 cop-value=20997 cop-physical=1696 cop-status=2
executed=68255 ticks=1076837
```

`cop-operation=1` is a command, `20997` is `0x5205`, and `1696` is `0x6a0`.
`2` is the existing `LIB_STATUS_INVALID_STATE`. This is a local runtime fact,
not an assertion that the missing receiver or timing is hardware-accurate.

## Verification

- x64/x86 `myraidenii-core-board-bus-smoke`: passed. Its existing COP DMA
  failure now proves operation/status capture and reset clearing.
- x64/x86 selected-set startup probe: identical bounded result above.
- Complete repository-only unit suites: 642/642 passed on x64 and 642/642
  passed on x86. The `repository.build-cleanup` test requires a temporary
  junction, so the full suites used normal host permissions; its isolated
  x64 rerun also passed.

## Disposition

The next unresolved work is COP command `0x5205` at Board-visible physical
address `0x6a0`. It remains unmodelled until a separately admitted L2 design
establishes its owner and behavior. S10 makes no playable-game claim.
