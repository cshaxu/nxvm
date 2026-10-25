# M5 T580 S20 — RAM-Test Return Checkpoint

## Question

The existing default real-firmware diagnostic stops at an execute watchpoint
near `F000:6060`.  This S tested whether that observation proves a completed
RAM-test helper and can select the first subsequent hardware receiver.

## Method

The temporary MyPC110 real-firmware diagnostic removed that watchpoint.  It
required all of the following before reporting success:

1. retirement at `F000:6068`, the helper's `ret` instruction;
2. execution at a later point; and
3. one later port operation observed through the existing Core trace route.

The diagnostic did not treat its eight-million-instruction containment limit
as success.  It used the existing TFT BIOS and font from the external BYOB
asset set; neither input was copied or changed.

## Result

Both x64 and x86 ended without a Core fault, with one firmware reset, in the
already-established `F000:EA86..EA90` port-61 delay loop.  Neither run
retired the proposed `F000:6068` return boundary or observed a subsequent
port event under this checkpoint.  The relevant common result was:

```text
status=0 reason=1 pc=000fea90 executed=8000133 resets=1
fault=00000000/00000000
```

Consequently the prior execute-watch observation is not sufficient to select
a RAM, panel, power, storage, or controller receiver.  The temporary CLI
mode and CTest registration were removed.  No source, firmware, production
or public-interface file remains changed by this S.

## Limit

This is an inconclusive semantic checkpoint, not proof that the firmware
cannot leave the helper.  A later receiver needs an independently sourced
event boundary, not a larger retirement budget.
