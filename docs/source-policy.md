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
and verification in its import commit and the compact
[`provenance/nxvm-imports.md`](provenance/nxvm-imports.md) index. New code must
not extend NXVM's global state or legacy coupling when a bounded interface is
practical. If a selected file carries an independent third-party notice or
license, stop and record a separate review before importing it.

## C Vocabulary Boundary

[C-Library Facade](architecture/c-library-facade.md) is the sole authority for
the `type.h` vocabulary, ISO C header/call boundary, platform type exposure,
input-flush safety, and approved compact code names. Imported or derived source
must comply with that authority; this source policy does not restate it.

## Other Open Source

DOSBox, DOSBox-X, DOSEMU, ReactOS, WineVDM, QEMU, and similar projects are
design and behavior references unless a separate license review authorizes
copying. Important observed behavior becomes a project requirement only when a
probe, documentation source, or reproducible reference test validates it.

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
committed, bundled, downloaded, listed in a project/release hash catalog, or
made a default runtime dependency. A future external-ROM provider may read a
user-supplied local bundle only after an owner-approved machine-profile design
gate defines its manifest, validation, mapping, and legal boundary. A local
manifest hash validates the owner's supplied file; it does not make a ROM
redistributable or authorize copying vendor code.

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
