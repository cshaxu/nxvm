# T429 S3: CECG Aperture Wait Skeleton

`M5:T429:S3:CECG-APERTURE-WAIT:OK`

Tier: `generic-at`. Original CECG material establishes the selected aperture
surface, not a measured wait duration or IOCHRDY/NOWS/BUSRDY behavior.

Core retains one commit-gated external-access window mechanism. Model-40 now
selects a seventh one-tick memory-space window for `A0000h--AFFFFh`; S2 keeps
that range outside D4 conventional-RAM page timing. Core owns lifecycle,
cancellation, reset and retirement; VM only selects the range. The six CECG
port windows remain unchanged and 5170 selects no CECG window.

Focused Core proof covers aperture commit, cancellation and reset; Model-40
and 5170 composition smokes cover selection/isolation. This is not a physical
CECG timing, raster, monitor, firmware or L3 claim.