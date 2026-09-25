# Source, License, And Research Detail

> Supporting procedure for the source/research constraints summarized in
> [rules/ARCHITECTURE.md](../../../../rules/ARCHITECTURE.md). It cannot create a new
> product dependency, distribution commitment, or architecture authority.

## Historical NXVM Foundation

The following records the pre-consolidation ntvdm64 import context, not a second
current repository or product line. Current NXVM lives in `src/app-nxvm` under
the root MIT license; the VDM direction is retired. The original authorization
and per-unit provenance obligations below remain preserved.

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

[rules/CODING.md](../../../../rules/CODING.md) defines the shared C vocabulary, ISO C
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

The owner-managed external `nxvm-assets/manuals` archive is the canonical
location for every acquired original hardware-manual copy. PDFs, scans, OCR
sidecars, and derived renderings do not enter this repository or its release
artifacts.
A repository evidence record names the source, edition/order number,
acquisition date, and SHA-256 so that the archived original remains
independently verifiable.

## Historical NTVDM Research

Historical third-party NTVDM implementations may inform research leads,
component inventory, startup hypotheses, and test design. They are not copied,
transliterated, or treated as the current NXVM specification. They do not authorize
a formal Microsoft backend, ABI, source import, or runtime dependency. A
trusted external report may be used as a requirements reference even while its
separate research proceeds; its proprietary details must be reduced to neutral
capability statements before they reach a shared contract. Research conclusions
require binary behavior, experiments, or tests where feasible. Published
product code is organized and maintained here.

## Microsoft Binaries

Microsoft binaries never enter this repository or a release. The owner-approved
whole-machine integration route may use lawfully supplied external guest media.
This does not authorize a DOS/VDM host, redistribution or an import/download
workflow. Asset masters and local manifests remain in `nxvm-assets`.

## Third-Party Firmware Images

Award, Phoenix, IBM, Compaq and other third-party firmware bytes are never
committed, bundled or made a project download. The owner-approved runtime
and integration route loads user-supplied external ROMs; missing firmware is
an explicit error, not permission for an embedded BIOS fallback. This local
use does not grant redistribution rights.

The external `nxvm-assets/profiles-nxvm` archive owns vendor payloads and manifests
with slot, size, SHA-256, read-only mapping and provenance. Following the completed
cutover, CMake receives one local `NXVM_PROFILE_ASSETS_ROOT` for the selected
machine build. It validates the selected Profile's manifest and generates an
ignored local asset-root binding; it never copies firmware into source control
or embeds it in the EXE. The retired session-YAML loader is historical only.
NXVM.ini resolves only runtime-media paths from the adjacent INI. Neither
configuration format embeds hashes/catalogues, firmware paths or protected
binary payloads. Repository templates use portable references, never tracked
machine-local absolute paths. The loader supplies immutable bytes through the
single Core ROM path.

For XT, AT, DeskPro, default and PC110, `src/app-nxvm/profiles` owns board C,
ROM slot/mapping declarations and authorized ROM source if separately admitted.
Protected `.rom` and `.cmos` payloads stay in the corresponding external profile
archive. A CMOS seed initializes documented writable configuration, not a
second live register owner. Acquiring an original manual or locating a BIOS
download is not proof of a complete or redistributable machine asset set.

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
build structure are not current NXVM architecture and must not be copied,
transliterated, or introduced as a dependency.
