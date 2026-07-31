# Source Policy

## NXVM Foundation

Before M1 imported NXVM source, ntvdm64 contained only project-owned material
under the root MIT License. Referencing or pinning an external NXVM checkout
does not import its source or attach that checkout's license metadata to this
repository.

NXVM is the formal machine-code foundation and a continuing product surface.
Its CPU, memory, interrupt, BIOS, device, debugger, assembler, and disassembler
assets may be copied, adapted, reorganized, or removed as the shared-core
architecture requires. The NXVM copyright holder authorizes NXVM code imported
into this repository under the root MIT License. The separate NXVM repository's
license metadata is maintained there and is not a precondition for this
authorization.

Each imported or substantially derived unit must record the exact NXVM commit,
source and destination paths, copyright notices, MIT authorization, changes,
and verification in `docs/provenance/`. New code must not extend NXVM's global
state or legacy coupling when a bounded interface is practical. If a selected
file carries an independent third-party notice or license, stop and record a
separate review before importing it.

## Other Open Source

DOSBox, DOSBox-X, DOSEMU, ReactOS, WineVDM, QEMU, and similar projects are
design and behavior references unless a separate license review authorizes
copying. Important observed behavior becomes a project requirement only when a
probe, documentation source, or reproducible reference test validates it.

## Historical NTVDM Research

OpenNT and NTVDMx64 are historical architecture and problem-location sources.
They may inform research leads, component inventory, startup hypotheses, and
test design. They are not copied, transliterated, or treated as the ntvdm64
specification. They do not authorize a formal Microsoft backend design before
M11 T1 and T2. Research conclusions require binary behavior, experiments, or tests
where feasible. Published product code is organized and maintained here.

## Microsoft Binaries

Microsoft binaries never enter this repository or the default release. BYOB is
a future research possibility, not a current feature or promised `ntvdm import`
workflow. Any future profile and hash policy follows an M11 Go decision.

## Third-Party Firmware Images

Award, Phoenix, IBM, Compaq, and other third-party firmware images are never
committed, bundled, downloaded, listed in a project/release hash catalog, or
made a default runtime dependency. A future external-ROM provider may read a
user-supplied local bundle only after an owner-approved machine-profile design
gate defines its manifest, validation, mapping, and legal boundary. A local
manifest hash validates the owner's supplied file; it does not make a ROM
redistributable or authorize copying vendor code.
