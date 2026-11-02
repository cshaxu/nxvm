# T585 S17 — PC110 OPL2 register and timer receiver

Open-Source-PC110's ES488 and live-dump research identifies a separate YM3812
at 388h. The local 86Box reference checkout `4fef696a4eead1d55a28d6ac0e5bd2864e5454da`
models timer quanta of 80 and 320 microseconds, timer reload values, overflow,
masking and control reset in `src/sound/snd_opl2_nuked.c`. This is an L2
reference model, not a PC110 physical timing measurement or a claim of exact
silicon behavior. No third-party implementation is copied.

The App-owned OPL2 device consumes address/data writes at 388h/389h. Timer
state is evaluated against the existing Core guest clock on port access; no
host timer, worker or second scheduler is introduced. The timer status can be
polled, but no unverified physical IRQ wiring is claimed. Reset clears device
state. Non-timer register writes are retained for the subsequent FM synthesis
work; they do not yet produce audio. The already delivered ES488 PCM path is
separate and is not evidence of YM3812 music.

The existing machine-construction test now exercises both timers, status
reset, a masked overflow and the actual PC110 port route using guest CPU time.
This confirms those bounded paths, not all OPL2 commands or real software
compatibility. The full PC110 tests on x64 and x86 cover firmware boot,
configuration and product composition; no physical audio or user RDP session
is asserted. An initially invoked aggregate integration run was stopped
before qualification; only the separately run isolated-desktop PC110 test
results are counted. The default NXVM pair was incidentally rebuilt and its
only changes are normalized PE timestamp/checksum differences.

Both x64 and x86 passed 20/20 PC110 tests. After the full runs, the
actual-machine 388h route assertion was added and the affected unit was
rebuilt and passed separately on both architectures.
