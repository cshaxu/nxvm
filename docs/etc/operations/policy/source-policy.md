# Source, License, And Research Detail

> Supporting procedure for the source/research constraints summarized in
> [rules/ARCHITECTURE.md](../../../rules/ARCHITECTURE.md). It cannot create a new
> product dependency, distribution commitment, or architecture authority.

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
and verification in its import commit and the compact
[`provenance/nxvm-imports.md`](../../research/provenance/nxvm-imports.md) index. New code must
not extend NXVM's global state or legacy coupling when a bounded interface is
practical. If a selected file carries an independent third-party notice or
license, stop and record a separate review before importing it.

## C Vocabulary Boundary

[rules/CODING.md](../../../rules/CODING.md) defines the `type.h` vocabulary, ISO C
header/call boundary, platform type exposure, and input-flush safety. The
[C-Library Facade Detail Record](../../history/m5/c-library-facade.md)
preserves supporting M5 inventory and rationale. Imported or derived source
must comply with the coding rules; this source policy does not restate them.

## Other Open Source

DOSBox, DOSBox-X, DOSEMU, ReactOS, WineVDM, QEMU, and similar projects are
design and behavior references unless a separate license review authorizes
copying. Important observed behavior becomes a project requirement only when a
probe, documentation source, or reproducible reference test validates it.

## Hardware Manuals

The owner-managed external `assets/manuals` archive is the canonical location
for every acquired original hardware-manual copy. PDFs, scans, OCR sidecars,
and derived renderings do not enter this repository or its release artifacts.
A repository evidence record names the source, edition/order number,
acquisition date, and SHA-256 so that the archived original remains
independently verifiable.

## Historical NTVDM Research

Historical third-party NTVDM implementations may inform research leads,
component inventory, startup hypotheses, and test design. They are not copied,
transliterated, or treated as the ntvdm64 specification. They do not authorize
a formal Microsoft backend, ABI, source import, or runtime dependency. A
trusted external report may be used as a requirements reference even while its
separate research proceeds; its proprietary details must be reduced to neutral
capability statements before they reach a shared contract. Research conclusions
require binary behavior, experiments, or tests where feasible. Published
product code is organized and maintained here.

## Microsoft Binaries

Microsoft binaries never enter this repository or the default release. BYOB is
a future research possibility, not a current feature or promised `ntvdm import`
workflow. Any future profile and hash policy follows a separately approved
research decision.

## Third-Party Firmware Images

Award, Phoenix, IBM, Compaq, and other third-party firmware images are never
committed, bundled, listed in a project/release hash catalog, or made a default
runtime dependency. With explicit owner authorization, they may be acquired,
organized, and used only in an external owner-managed research archive. That
local authorization does not make a vendor image redistributable, a repository
asset, a project download mechanism, or a product dependency. A future
external-ROM provider may read a user-supplied local bundle only after an
owner-approved machine-profile design gate defines its manifest, validation,
mapping, and legal boundary. A local manifest hash validates the owner's
supplied file; it does not make a ROM redistributable or authorize copying
vendor code.

The approved profile-admission boundary requires a future BYOB manifest to
name an explicit local path, profile ROM slot, exact size, SHA-256, requested
read-only mapping, and user provenance statement. It is consumed only by a
separately admitted VM-composition implementation before machine creation.
The repository must not retain the path, ROM bytes, a vendor catalogue, or a
download/discovery mechanism. A profile declares only an abstract ROM-slot
constraint; `core` receives only a generic immutable mapping provider.

## Bochx And Bochs Research

The historical NXVM Bochx bridge may be imported only as provenance-recorded
NXVM code into an optional research tool. Bochs source, patches, binaries,
firmware, guest media, and generated traces are separate local research inputs
pending their own license review; none enter this repository, default build,
release, or runtime dependency.

Bochs may inform a bounded device-state or behavior design only after the task
names its primary hardware/documentation contract and project-owned probes.
For example, controller queues/register state and the separation of video core
from host presentation are legitimate design lessons. Its global device
management, direct GUI integration, C++ plugin architecture, source text, and
build structure are not ntvdm64 architecture and must not be copied,
transliterated, or introduced as a dependency.
