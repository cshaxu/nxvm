# M5 AT Fixed-Disk Controller Standardization

## Purpose

Replace the misleading assumption that one ATA controller represents every
PC/AT fixed disk with one standards-led `AT fixed-disk` controller front end.
The selected controller owns the only guest-visible task-file, command-phase,
DRQ, IRQ and deadline state.  A frozen profile personality selects its
source-qualified command and drive-side behavior; a logical sector-image
provider remains only the media owner.  This avoids one HDC implementation per
drive while preserving the real difference between ST-506, ESDI, early
Compaq-integrated and ATA equipment.

## Required Scope

The common front end may be shared only for host-visible PC/AT fixed-disk
semantics that the primary documents and bounded reference comparison prove
identical.  It must not be named ATA, and it must not expose a live generic
setter.  Construction receives one copied, validated immutable personality
covering command capability, drive-side transport, I/O route, IRQ/DRQ route,
geometry policy and source-qualified phase-time inputs.

The initial selected personalities are:

| Personality | Required disposition |
| --- | --- |
| IBM 5170 WD1003/ST-506 | Implement the documented PC/AT controller and its MFM/ST-506 selected behavior. RLL is a distinct documented variant, not a new HDC family. |
| DeskPro 386 Model 40 Compaq/WD integrated controller | Implement only the documented pre-standard integrated-controller behavior required by this profile; do not call it ATA merely because it has a 40-wire cable. |
| ATA PIO | Preserve T468's accepted ATA-3 behavior as an explicit later personality; it is not a 5170 substitute. |
| ESDI | Define the typed boundary and ledger only. Admit a concrete WD1005/WD1007 or other ESDI personality only when a selected profile and primary-controller source freeze it. |

The task compares each finite row against primary IBM, Compaq and ATA material,
then treats 86Box, MAME, PCjs, Bochs and QEMU only as corroborating
implementations.  No third-party code, firmware, media or source text enters
the product.

## Dependencies And Completion

Consumes T468's closed ATA contract, T474's Core deadline contract, and the
accepted 5170/DeskPro resolver paths.  It follows the active Default-AT profile
cutover and precedes YAML selection, because YAML must select an already valid
immutable controller personality rather than define device semantics.

Completion requires a finite source/cross-model ledger and code-gap ledger;
one shared front-end state owner; one media owner; construction-only profile
selection; selected 5170 and DeskPro parity; explicit ATA regression retention;
and every exact remaining ESDI or physical-media gap transferred without a
false L3 claim.

## Planned Subtasks

1. **S1 -- source and interface ledger.** Verify the relevant IBM 5170 and
   Compaq DeskPro controller documentation, including text/OCR quality and
   gaps; reconcile it with the retained ATA-3 source and read-only reference
   implementations.  Produce one row-complete ledger separating common
   host-visible facts from ST-506, ESDI, early integrated-controller and ATA
   facts, with Manual-L3, bounded Other-L3, L2 or unsupported disposition.
2. **S2 -- current-owner and profile-gap audit.** Trace every ledger row through
   the existing HDC, logical media, DMA/PIC routes, guest-time/deadline path and
   the 5170, DeskPro and Default-AT profile declarations.  Record the one
   current owner, duplicated or misnamed ATA assumptions, observable parity
   corpus, and each source-blocked ESDI fact.
3. **S3 -- immutable standard controller contract.** Replace any universal-ATA
   identity with one validated construction-time AT fixed-disk personality and
   one shared front-end state machine.  The Core public boundary remains
   opaque; no profile, VM or media code gains mutable controller access. Delete
   superseded construction paths in the same change.
4. **S4 -- selected 5170 and DeskPro controller closure.** Implement the
   source-qualified WD1003/ST-506 and DeskPro Model-40 behavior using the one
   front end and their frozen personality data.  Connect DRQ/IRQ and real
   phase/deadline facts through existing Core owners; retain an explicit L2 or
   unsupported boundary where a controller or board fact is absent.
5. **S5 -- ATA retention, proof and closure.** Retain T468 ATA-3 behavior only
   through the ATA personality, run focused controller/profile and full current
   gates, prove no second task-file/media state remains, and publish a release
   artifact.  Transfer unselected ESDI controller work only as a source-gated
   future candidate.

## Architecture And Minimalism

The one selected HDC front end is the only mutable owner of registers,
commands, data phase, IRQ/DRQ publication and its pending deadline.  The
logical-media provider owns bytes and durable media state only.  Core owns bus
dispatch, DMA, PIC and time progression; VM/profile code supplies one copied
immutable declaration before construction and never writes controller state.

Do not introduce a stateful HDC base class plus parallel per-protocol shadow
objects, a dynamic plug-in framework, a second CHS truth, a host-latency
callback, profile setters, or an ATA-to-MFM compatibility shim.  Extract only
the demonstrated shared host interface; a source-proven difference stays in
the selected personality or a narrowly owned transport operation.  If a
candidate abstraction does not delete duplicated state or command-phase code,
it is out of scope.

## Non-goals And Stop Conditions

This is not raw-flux, MFM/RLL encoding, physical-sector/ECC, drive-mechanics,
vendor-controller catalogue, SCSI, ATAPI, arbitrary ISA-card, or complete ESDI
emulation.  It does not alter user request grammar, YAML, firmware/media asset
policy, host wall-clock pacing or any L4 claim.

Stop and transfer a row when the selected controller identity, command or
timing fact lacks a primary source and cannot be made a bounded Other-L3
corroboration.  Stop rather than widen the shared state machine if a profile
requires a non-isomorphic port map, command state, lifetime or DMA/IRQ
mechanism.
