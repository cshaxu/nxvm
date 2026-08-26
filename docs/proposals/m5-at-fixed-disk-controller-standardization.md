# M5 PC Fixed-Disk Controller Standardization

## Purpose

Replace the misleading assumption that one ATA controller represents every PC
fixed disk with one standards-led Core HDC owner and a finite set of frozen
controller personalities.
The selected controller owns the only guest-visible task-file, command-phase,
DRQ, IRQ and deadline state.  A frozen profile personality selects its
source-qualified command and drive-side behavior; a logical sector-image
provider remains only the media owner.  This avoids one HDC implementation per
drive while preserving the real difference between ST-506, ESDI, early
Compaq-integrated and ATA equipment.

## Required Scope

The common owner may share an operation only when the primary documents and
bounded reference comparison prove the semantics identical.  It is not named
ATA and exposes no live generic setter.  Construction receives one copied,
validated immutable discriminated personality: its protocol determines the
host register grammar and I/O route, then its frozen data supplies the command
capability, IRQ/DRQ route, geometry policy and source-qualified phase-time
inputs.  The 5160 byte-stream/DMA protocol is therefore not forced through an
AT task-file-shaped configuration merely to reuse a struct.

The initial selected personalities are:

| Personality | Required disposition |
| --- | --- |
| IBM PC/XT 5160-268 Fixed Disk Adapter | Reconcile the prior no-disk baseline claim against IBM material that lists XT-268 for the 20 MB adapter. Select only the documented controller/drive configuration and implement its distinct 8-bit host/controller semantics through the one HDC owner. |
| IBM PC/AT 5170 fixed-disk configuration | Keep the accepted no-disk Model 339 unchanged. Select a separately named, source-defined fixed-disk configuration using the IBM fixed-disk/diskette adapter and its MFM/ST-506 behavior; RLL is a distinct documented variant, not a new HDC family. |
| DeskPro 386 Model 40 Compaq/WD integrated controller | Implement only the documented pre-standard integrated-controller behavior required by this profile; do not call it ATA merely because it has a 40-wire cable. |
| ATA PIO | Preserve T468's accepted ATA-3 behavior as an explicit later personality; it is not a 5170 substitute. |
| ESDI | Select one concrete controller with an available primary manual, establish its matching frozen profile/personality data, and implement it through the same front end. |

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
one HDC state owner and one media owner; construction-only profile selection;
selected 5160, 5170 and DeskPro parity; explicit Default-AT ATA regression
retention; and every exact remaining ESDI or physical-media gap transferred
without a false L3 claim.

## Planned Subtasks

1. **S1 -- source and interface ledger.** Verify the relevant IBM 5170 and
   Compaq DeskPro controller documentation, including text/OCR quality and
   gaps; reconcile it with the retained ATA-3 source and read-only reference
   implementations.  Produce one row-complete ledger separating common
   host-visible facts from ST-506, ESDI, early integrated-controller and ATA
   facts, with Manual-L3, bounded Other-L3, L2 or unsupported disposition.
2. **S2 -- current-owner and selected-profile audit.** Trace every ledger row
   through the existing HDC, logical media, DMA/PIC routes, guest-time/deadline
   path and the 5160, 5170, DeskPro and Default-AT declarations. Reconcile the
   5160-268 and Model-339 baseline facts; select only separately named fixed
   configurations where required. Record the one current owner, universal-ATA
   assumptions, observable parity corpus and each source-blocked ESDI fact.
3. **S3 -- immutable HDC personality contract.** Replace the universal-ATA
   identity with one validated construction-time discriminated personality and
   one HDC state machine. The Core public boundary remains opaque; no profile,
   VM or media code gains mutable controller access. Delete superseded
   construction paths in the same change.
4. **S4 -- retained Default-AT and DeskPro closure.** Route ATA PIO and the
   Model-40 Compaq/WD integrated controller through the one owner and frozen
   personality data. Preserve T468/T386 observable behavior, including the
   Model-40 shared `3F7h` read semantics; do not reimplement either controller.
5. **S5 -- IBM 5170 fixed-disk closure.** Implement the selected source-qualified
   IBM fixed-disk/diskette adapter MFM/ST-506 personality and its separately
   named fixed-disk profile configuration. Keep Model 339's accepted no-disk
   descriptor intact. Retain explicit L2 or unsupported boundaries where a
   controller or board fact is absent.
6. **S6 -- IBM 5160-268 fixed-disk closure.** Implement the selected IBM/Xebec
   adapter through the same HDC owner and bind it only to the confirmed frozen
   5160-268 configuration. Do not make the XT host grammar an AT compatibility
   shim or invent an XT profile outside its approved profile receiver.
7. **S7 -- selected ESDI closure.** Implement one primary-backed ESDI
   personality through the same owner only where its documented host contract
   is compatible with the chosen immutable configuration; otherwise transfer
   the exact non-isomorphic receiver rather than add a second controller.
8. **S8 -- proof and release closure.** Run focused personality/profile proofs
   and full current gates, prove no second controller/media state remains, and
   publish stripped Release `nxvm_0_5_0479.exe`. Transfer only exact
   physical-media residuals.

## Architecture And Minimalism

The one selected HDC owner is the only mutable owner of registers, commands,
data phase, IRQ/DRQ publication and its pending deadline. The
logical-media provider owns bytes and durable media state only.  Core owns bus
dispatch, DMA, PIC and time progression; VM/profile code supplies one copied
immutable declaration before construction and never writes controller state.

Do not introduce a stateful HDC base class plus parallel per-protocol shadow
objects, a dynamic plug-in framework, a second CHS truth, a host-latency
callback, profile setters, or an ATA-to-MFM/XT compatibility shim. Extract only
the demonstrated shared operation; a source-proven difference stays in the
selected personality's branch inside the one owner. If a candidate abstraction
does not delete duplicated state or command-phase code, it is out of scope.

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
