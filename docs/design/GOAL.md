# Project Goals

ntvdm64 evolves the NXVM foundation into a portable, evidence-led x86 runtime
with a bootable VM product and a non-invasive DOS application product. These
are strategic outcomes, not present compatibility claims or implementation
instructions.

1. **Multi-profile NXVM.** Preserve a capable bootable `nxvm.exe` while making
   it able to select reproducible machine profiles. The target family includes
   default PC/AT, Compaq DeskPro 386 informed by PCjs behavior research, and
   IBM PC 110 with its separately evidenced hardware requirements.
2. **Windows compatibility progression.** Make the shared machine foundation
   capable of evidence-backed Windows 3.1 Standard Mode and 386 Enhanced Mode,
   then pursue Windows 95 as a longer-horizon compatibility target. Each claim
   is gated by an owned corpus rather than an aspirational CPU label or boot
   attempt.
3. **Multi-runtime VDM foundation.** Make `core` and `mantle` able to support
   the owned DOS/VDM implementation and separately admitted external VDM or
   DOS implementations. The owned path remains the default; an external
   adapter never becomes an implicit dependency, shared public ABI, or release
   input merely by being supported.
4. **Web delivery.** Make `core` and `mantle` portable to WASM and provide a
   TypeScript web product layer so the VM can be delivered in browsers wherever
   the required legal assets and host capabilities are available.
5. **Trustworthy continuity.** Retain established NXVM boot, Console, and
   debugger value while improving compatibility, hardware fidelity, and host
   integration through bounded, reproducible evidence.

[System Architecture](ARCHITECTURE.md) defines the concrete component shape,
[Product UX](UI.md) defines product interaction, and [Roadmap](ROADMAP.md)
defines staged delivery. Architecture, coding, source, and execution constraints
belong to [rules/](../rules/).
