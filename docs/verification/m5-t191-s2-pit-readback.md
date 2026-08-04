# M5 T191 S2 PIT Read-Back Verification

## Scope

`core/machine/pit.*` now implements the bounded 8254 read-back contract:
active-low counter selection, independent count/status latches, status-first
reads, preserved unread latches, and programmed RW count-byte order. This does
not broaden the retained timer into a cycle-accurate waveform model.

## Evidence

- GCC configure/build passed with `cmake --preset mingw-gcc-x64`.
- `core-machine-pit-readback-smoke` emitted `M5:T191:S2:PIT-READBACK:OK`.
- FDD prompt and keyboard regressions emitted `M5:T70:S2:DOS-PROMPT:OK` and
  `M5:T151:S2:DOS-KEYBOARD:OK`.
- FDD/HDD session/reset-vector regressions emitted `M5:T7:S1:NXVM-SESSION:OK`
  and `M5:T13:S8:VM-SESSION:OK`.
- `cmake --build --preset current-gates-gcc` passed, including dependency,
  executor, lifecycle, session-readiness, provider-composition, and retained
  Console-lifecycle gates.
- The task artifact printed `Neko's x86 Virtual Machine [0.5.0191]`, accepted
  `EXIT`, and returned zero.

Artifact SHA-256: `267A03293A5F76B37AAA7281F7A83B6B1D09EA634F9332576CD11B91AFC3D833`.

## Exit Review

The T191 contract is met without a second PIT state owner, VM/profile
dependency, timing-model rewrite, or Console behavior change. Per-mode output
waveforms, GATE behavior, BCD edges, and count-zero semantics remain explicitly
deferred in `TODO.md`.
