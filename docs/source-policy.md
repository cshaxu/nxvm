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
They may inform BOP hypotheses, component expectations, startup order, and
test design. They are not copied, transliterated, or treated as the ntvdm64
specification. Published ntvdm64 code is organized and maintained here.

## Microsoft Binaries

Microsoft binaries never enter this repository or the default release. Optional
guest-mode support accepts only user-supplied local files through an explicit
profile and hash policy. Mixed versions are rejected.
