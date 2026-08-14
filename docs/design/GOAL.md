# Project Goals

NXVM is the current product of a portable, evidence-led x86 codebase. The
repository evolves its NXVM machine foundation toward a future, non-invasive
NXVDM DOS application product. These are strategic outcomes, not present
compatibility claims or implementation instructions.

1. **Multi-profile NXVM.** Preserve a capable bootable `nxvm.exe` while making
   it able to select reproducible machine profiles. The target family includes
   default PC/AT, Compaq DeskPro 386 informed by PCjs behavior research, and
   IBM PC 110 with its separately evidenced hardware requirements.
2. **M5 L3 and Windows completion.** Finish M5 with source-backed L3 timing
   for the selected PC/AT device corpus, then make that profile capable of
   evidence-backed Windows 3.1 Standard Mode and 386 Enhanced Mode install,
   normal start, named ready checkpoint, and normal shutdown. Each claim is
   gated by an owned corpus rather than an aspirational CPU label or boot
   attempt. Windows 95 remains a longer-horizon compatibility target.
3. **Multi-runtime VDM foundation.** Make the NXVM-derived `core` and `mantle` able to support
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
6. **Intel 80386DX architectural completeness.** Before M6 admission, complete
   the approved CPU-side 80386DX program through both instruction-form evidence
   and architecture-state closure. This means real/protected/VM86 state
   transitions, privilege and exception delivery, paging, and remaining system
   state are reconciled against the Intel architecture; it is not a claim that
   Windows boots, that every hardware device is complete, or that 80387 numeric
   execution exists.

[System Architecture](ARCHITECTURE.md) defines the concrete component shape,
[Product UX](UI.md) defines product interaction, and [Roadmap](ROADMAP.md)
defines staged delivery. Architecture, coding, source, and execution constraints
belong to [rules/](../rules/).
