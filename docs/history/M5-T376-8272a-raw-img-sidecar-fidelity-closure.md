# M5 T376: 8272A Raw-IMG Sidecar Fidelity Closure

## Task Record

T376 closes the selected 8272A Deleted-Data/Control-Mark and Scan command
routes while retaining raw `.img` files as independent sector-payload media.
An optional same-basename JSON sidecar may persist only metadata that raw IMG
cannot express. The task consumes T375's FDC timing ledger and precedes the
final IBM PC/AT 5170 Model-339 L3 audit.

## Active Progress

### S1: media owner and sidecar lifecycle inventory

S1 audits the existing provider, raw-IMG mount/save/remove, FDC command and
atomic-persistence routes before implementation. It must produce an
implementation-ready single-owner contract without a test-only media API or a
second image format.

### S2: FDD sidecar schema and paired persistence

S2 implements the optional sidecar only at the VM FDD/media-save boundary. It
must preserve ordinary raw IMG use, validate a bounded metadata schema before
publishing media, and preserve the currently mounted guest-visible pair on any
mount or save failure.
