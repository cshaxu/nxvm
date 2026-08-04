# M5 T193 VADP Text-Controller Verification

## Closed Scope

T193 implements one CGA text-controller slice only. `core/machine/vadp.*`
owns text CRTC/mode/color/status state, B8000 visible-window capture, dirty
state, and copied snapshots. QDCGA remains INT 10h/BDA firmware. Composition
converts the copied core snapshot to the platform frame; platform never sees
guest VADP or VRAM.

## Evidence

- `core-machine-vadp-text-smoke` emitted `M5:T193:S2:VADP-TEXT:OK`.
- `vm-qdcga-boundary-smoke` emitted `M5:T40:S1:QDCGA-BOUNDARY:OK`.
- `vm-platform-presentation-mailbox-smoke` emitted
  `M5:T80:S4:PRESENTATION-MAILBOX:OK`; `vm-platform-mode-context-smoke`
  emitted `M5:T80:S5:MODE-CONTEXT:OK`.
- Local FDD checks emitted `M5:T70:S2:DOS-PROMPT:OK` and
  `M5:T151:S2:DOS-KEYBOARD:OK`.
- FDD/HDD session checks emitted `M5:T7:S1:NXVM-SESSION:OK` and
  `M5:T13:S8:VM-SESSION:OK`; two-session isolation emitted
  `M5:T73:S1:TWO-SESSION-ISOLATION:OK`.
- `cmake --build --preset current-gates-gcc --parallel 1` passed.
- `nxvm_0_5_0193.exe` printed `Neko's x86 Virtual Machine [0.5.0193]`,
  accepted `EXIT`, and returned zero.

Artifact SHA-256:
`D355B2875782932398BB11CB109878D107D6778F39B72EB17EAFF91291954884`.

## Exit Review

The core owns one VADP and its copied snapshot cache. Default-profile firmware
has no mutable VADP pointer and cannot capture display data. Composition is the
sole core-snapshot-to-platform-frame conversion. The text slice preserves the
retained NXVM Console, boot, and keyboard behavior.

No graphics claim is made. T193 S3 defers CGA `320x200x4` until an owned
graphics probe and its device/frame contract exist; EGA/VGA are later separate
admissions in `TODO.md`.
