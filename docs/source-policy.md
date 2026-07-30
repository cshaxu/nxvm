# Source Policy

## NXVM Foundation

NXVM is the formal machine-code foundation for ntvdm64. Its CPU, memory,
interrupt, BIOS, device, debugger, assembler, and disassembler assets may be
copied, adapted, reorganized, or removed as the ntvdm64 architecture requires.
Each imported or substantially derived unit must record the exact NXVM commit,
source and destination paths, copyright notices, LGPL obligations, changes,
and verification in `docs/provenance/`. New code must not extend NXVM's global
state or legacy coupling when a bounded interface is practical.

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
M5 and M6. Research conclusions require binary behavior, experiments, or tests
where feasible. Published ntvdm64 code is organized and maintained here.

## Microsoft Binaries

Microsoft binaries never enter this repository or the default release. BYOB is
a future research possibility, not a current feature or promised `ntvdm import`
workflow. Any future profile and hash policy follows an M6 Go decision.
