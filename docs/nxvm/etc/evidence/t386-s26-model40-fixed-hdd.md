# T386 S26: Model-40 Fixed 40 MB Startup Media

`M5:T386:S26:MODEL40-HDC-STARTUP:OK`

`M5:T386:S26:MODEL40-HDC-FIXED-MEDIA:OK`

## Delivered Boundary

S26 publishes the fitted Model-40 fixed-disk route only during construction of
its fixed BYOB session. The VM composition loads exactly one RAW-IMG whose
logical capacity is the selected Compaq 925-cylinder, 5-head, 17-sector,
512-byte geometry. The public Model-40 session operation rejects later fixed
media insertion. The catalog also rejects Model-40 non-ROM boot declarations,
so it does not imply an unimplemented generated-BIOS boot policy.

The existing Core Compaq WD 40 MB controller remains the single owner of
normal CHS/PIO, IRQ14 acknowledgement, command error and SRST behavior. The
shared VM HDD backing now owns a checked geometry configuration whose capacity
must equal the already loaded image and whose values survive a cold reset.
Model-40 composition declares the selected geometry through that reusable
owner; it does not retain a profile-local reset workaround. Default ATA and
Model-339 no-disk behavior remain separate.

## Focused Proof

`vm-model40-hdc-s26-smoke` uses only project-owned synthetic ROM chips and a
temporary synthetic 40 MB RAW-IMG. It proves startup construction, 925/5/17
geometry after cold reset, Compaq CHS read PIO, IRQ14 status acknowledgement,
unsupported Identify error, SRST state, and rejection of post-publication
fixed-media replacement. A short image rejects construction.

The shared-owner `core-machine-media-provider-smoke` directly proves exact-capacity geometry validation rejects a mismatch, selected geometry survives reset, and every new media candidate restores generic 16/63 geometry before its own capacity is published. The retained S5 Compaq HDC route and machine-composition smokes, S24 FDC
smoke, and S25 D4/refresh smoke pass. The configured current smoke gate,
specialized gates and documentation governance gate pass from the rebuilt
current graph.

## Transfers

This is a logical RAW-IMG startup contract only. It neither proves external
firmware reaches a fixed-disk boot consumer nor implements Long/format/ECC,
recovery, physical-sector identity, drive/controller service duration, board
waits, arbitration or Model-40 L3 timing. Those physical-media and timing
boundaries remain explicit debt and later DeskPro receivers.