# T374 S3: Model-339 8042 Pulse-Reset Repair

## Change

`core_machine_kbc_write_command()` now recognizes IBM 8042 command-port
`F0h`--`FFh` pulse commands. When command bit 0 is clear, it calls the already
owned `core_machine_cpu_execution_request_reset()` boundary. It does not route
through D1h, alter the persistent output-port byte, or modify A20. A command
whose bit 0 is set makes no reset request. Other pulse bits have no selected
NXVM consumer and remain unobservable/no-op.

The change stays in the KBC owner. The existing D1h data-write path remains
the only persistent output-port/A20 mutation route. No duration is assigned:
the IBM approximately-six-microsecond pulse remains a board/device timing
question for the later 5170 timing candidate.

## Focused Proof

`core-machine-kbc-controller-smoke` now establishes, after setting D1h output
state to `03h`, that command-port `FFh` makes no reset request, `FEh` makes one
reset request, A20 remains set, and a D0h read still returns `03h`. Its existing
coverage continues to prove D0h/D1h, keyboard IRQ1, command responses, reset,
and mixed FIFO behavior.

Fresh Git-Bash Ninja verification:

1. clean and rebuild `core-machine-kbc-controller-smoke` and
   `vm-ibm-5170-model-339-composition-smoke` (95 build steps), then run both;
2. rebuild and run `core-machine-kbc-aux-port-smoke`.

Observed results:

```text
M5:T227:S3:KBC-CONTROLLER:OK
M5:T366:S5:MODEL339-COMPOSITION:OK
M5:T267:S1:AUX:PORT:OK
```

No ROM, guest media, external runtime, profile descriptor, timing value or
machine-local asset entered the repository.

## Similar-Issue Sweep

The command-port switch has one newly handled range, `F0h`--`FFh`; D1h remains
the only caller of `core_machine_kbc_apply_output_port()`. The only CPU reset
request callers remain that D1h output-port handling and the new transient
pulse command path. The KBC AUX path was rebuilt and replayed unchanged.
No duplicate reset owner, persistent pulse mutation, or generic-device
expansion was found.

## Transfer

This closes only the selected pulse-reset functional gap. Remaining selected
8042 status/error/keyboard completeness, FDC, CGA and board-controller
functional matrices remain T374 work. Pulse duration and every board phase
remain exclusively with the queued 5170 board/device phase-timing closure.
