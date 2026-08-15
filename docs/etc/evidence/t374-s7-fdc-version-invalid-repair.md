# T374 S7: Model-339 FDC Version-Command Removal

## Change

The selected controller is Intel 8272A-compatible. Its
[primary command table](https://cpctech.cpcwiki.de/docs/i8272/8272sp.htm)
defines fifteen commands and states that any other opcode produces a
one-byte, non-interrupting ST0 `80h` invalid result. `10h` is not among those
commands. The explicit `VERSION` command length and execute cases that returned
`90h` have therefore been removed. `10h` now reaches the existing default
invalid-command route; no new FDC branch, public interface, profile capability
or enhanced-controller model was added.

The source was consulted as controller semantics only. No controller timing,
drive/board property, ROM, guest media, external implementation or local asset
was imported.

## Focused Proof

`core-machine-fdc-smoke` now sends `10h` after `Specify` and proves all of:

1. exactly one result byte is available and it is ST0 `80h`;
2. no IRQ is asserted;
3. after consuming that byte, the FDC is back in command phase.

Fresh Git-Bash CMake build/replay of the changed and selected-topology targets
produced:

```text
M5:T283:S2:CORE-FDC-MEDIA:OK
M5:T347:S2:FDC-SERVICE:OK
M5:T290:S1:FDC:PORT:OK
M5:T291:S1:FDC:PORT:OK
M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK
```

## Similar-Issue Sweep And Transfer

The sweep covered all explicit FDC opcode cases, command-length determination,
the default invalid result route, all in-repository `90h`/`VERSION` references,
profile descriptors and focused tests. No consumer depended on the removed
enhanced response. The default still supplies the 8272A invalid behavior for
other unrecognized opcodes.

This corrects only the over-admitted command. Read Deleted, Write Deleted,
all Scan commands, multi-track/skip/`N`/`DTL`/`EOT`/`GPL` semantics and their
necessary media/compare contracts remain explicit T374 functional work.
Controller clocks, drive mechanics, DMA service and board timing remain with
the later 5170 timing candidate. **No FDC or Model-339 L3 closure is claimed.**
