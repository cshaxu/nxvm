# M5 T226: KBC Guest Input Protocol, Phase 1

## Scope

T226 makes the existing input path guest-visible end to end:

```text
host key down/up -> platform ingress -> default-profile set-1 mapper
  -> core KBC FIFO/60h -> PIC IRQ1 -> ROM INT 09h -> BDA -> ROM INT 16h
```

Core KBC owns the only guest byte FIFO, OBF and IRQ1 lifecycle. The profile
maps host identity to set-1 byte sequences but owns no guest queue. Platform
only transports host events and never changes BDA, KBC, or BIOS state.

## S1: Contract And Probes

**Status:** complete.

- A normal key has one make byte on press and `make | 80h` on release.
- An extended key has `E0h, make` on press and `E0h, make | 80h` on release.
- Pause admits the fixed set-1 `E1h 1Dh 45h E1h 9Dh C5h` press sequence only.
- The KBC FIFO preserves byte order across rapid typeahead. Reading `60h`
  acknowledges exactly its head byte; it deasserts IRQ1 and reasserts it for a
  successor through the existing PIC source boundary.
- ROM INT 09h consumes exactly one 60h byte per IRQ1. It alone updates BIOS
  modifier/BDA state. ROM INT 16h observes only the BDA buffer.
- The phase-1 supported BIOS behavior is set-1 modifier make/break plus E0
  prefix state and extended-key storage. It does not claim scan-set selection,
  translation, LEDs, typematic, resend/error timing, or AUX mouse.

Focused evidence is a core KBC port/FIFO/IRQ1 probe, a default-ROM INT 09h/16h
probe, mapper sequence coverage, and the retained FDD prompt/EDIT.COM/Console
and window input paths.

## S2: Owner-Local Implementation

**Status:** complete.

The platform transport gains an explicit pressed/released event, passed through
the existing session ingress transport. The profile mapper returns a bounded
set-1 sequence. The session submits that sequence only to the core machine;
the core KBC is responsible for all guest bytes and IRQ1 transitions. Replace
the profile's direct host-state-to-BDA mutation with ROM processing of the same
make/break bytes.

Stop for a second queue, direct host/profile BDA mutation, direct PIC/CPU
interrupt delivery, a host-timing dependency, or changed retained NXVM UI
semantics.

## S3: Exit Evidence

Run focused port, ROM, mapper, and rapid-typeahead probes; FDD prompt typing;
the retained `EDIT.COM` keyboard interaction; Console/window input; and the
current GCC/CTest gate. Record the task artifact/hash and preserve the T225
time baseline.

**Status:** complete.

Evidence:

- `core-machine-kbc-controller-smoke` locks FIFO order, IRQ1 acknowledgement,
  and rapid-typeahead capacity; `vm-keyboard-set1-mapper-smoke` locks normal,
  E0, and Pause sequences.
- `vm-dos-prompt-smoke` and `vm-dos-keyboard-smoke` pass against the retained
  FDD image; the retained `EDIT.COM` exercise remains in the current matrix.
- `cmake --build --preset current-gates-gcc --parallel 4` passes 59/59 CTest
  smokes and all static ownership/boundary gates.
- Historical artifact withdrawal: `nxvm_0_5_0226.exe` was created from a
  worktree that was not the docs-only `934a3cb` task commit. Its SHA-256
  `2FE4D3833409BC9037FD13CFC7EB4DDC6F14CD83CE8CA75A623FDB7DE050B292` is kept
  only to identify the reported repeated-key regression; it is not reproducible
  T226 completion evidence and must not be used as a baseline.
