# T375 S15: Model-339 Rev.3 CGA Defaults

`M5:T375:S15:MODEL339-REV3-CGA-DEFAULTS:OK`

The selected Model-339 firmware context now carries only its abstract firmware
slot to the existing QDCGA reset owner.  When that slot is IBM 5170 Rev.3,
QDCGA programs the observed 80-column mode-3 CRTC sequence through its normal
`3D4h`/`3D5h` firmware-port boundary before applying the existing BDA/cursor
state.  The generic default-PC/AT EGA route does not receive the sequence.

The owner-authorized ROM was read only and remains external.  No image bytes,
path, hash, catalogue or runtime provider enters NXVM.  This is a declarative
selected-firmware behavior result, not a ROM import, factory-wide assertion,
physical timing, contention, monitor or L3 claim.

Focused Model-339 topology proof verifies R0--R7, R9--R15 after cursor setup
and emits the S15 marker; the retained clock, VADP logical-raster and EGA
boundary regressions remain required.  Default mode tables beyond this selected
reset state, R8 interlace/skew, physical output and final 5170 audit remain
open T375 work.
