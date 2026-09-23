# M6 T39 S1 Start-Path Diagnosis

The owner-local Mapper-004 input was used only through the existing
environment-gated probe. Its path, bytes, digest and raw execution trace are
not repository material.

| Candidate | Bounded observation | Disposition |
| --- | --- | --- |
| Driver to controller | A project-owned serial-pad fixture now reads A through Start. A physical `KVM_KEY_ENTER` make, paired break, and subsequent guest latch leaves serial Start set once, then clear on the next latch. | Accepted: Enter routing and transient preservation are not the reported defect. |
| Owner-local controller consumption | After a finite warm-up, a temporary ignored probe delivered Enter and observed a `$4016` serial Start bit before continuing. A matched no-input/start-input pair then reached different guest RAM states without a trap. | Accepted diagnostic: the game consumes Start. This is not a gameplay-completion oracle. |
| CPU and cartridge mapping | The existing bounded run reaches hundreds of frames without a CPU trap; the owner-local header is within the existing Mapper-004 profile. | Not selected: this excludes only immediate CPU/header failure. |
| MMC3 IRQ/A12 and PPU publication | The generic Mapper-004 fixture proves register and qualified-edge behavior, but the owner-local Start observation does not establish scanline-exact A12 behavior or a title-to-play display checkpoint. | Open receiver: S2 must isolate the PPU's per-dot A12 level and retain a project-owned timing regression before any visual-path claim. |

The retained mechanism boundary is therefore Core PPU-to-Cartridge A12 level
publication. S2 must decide its exact per-dot contract; it may not treat the
finite owner-local state difference or a frame count as a compatibility proof.
