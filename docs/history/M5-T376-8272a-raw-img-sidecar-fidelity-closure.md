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

### S3: Deleted-Data controller path

S3 binds Read Deleted Data, Write Deleted Data and ordinary-read Control Mark
result behavior to the existing FDC transfer owner and T375 cadence. It does
not admit Scan commands.

### S4: Scan controller path

S4 implements Scan Equal (`11h`), Scan Low-or-Equal (`19h`) and Scan
High-or-Equal (`1Dh`) through the existing write-direction DMA2 and non-DMA
`3F5h` transfer owners. It compares only guest-supplied bytes, ends on the
first satisfying sector, records ST2 Scan Equal Hit or Scan Not Satisfied, and
does not modify sector payload. Deleted-Data marks are queried through the
frozen provider: an unskipped mark is reported through ST2 Control Mark and
the `SK` command bit skips it without consuming comparison bytes. The S4
evidence replays the retained 500-kbit/s byte gate and DOR cancellation.
