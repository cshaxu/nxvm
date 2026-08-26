# IBM 5160 CGA VADP Phase Contract

## Purpose

Complete selected XT CGA function and timing at VADP, the sole video
port/memory/mode/frame-snapshot owner.

## Required sequence

S1 validates original IBM/CGA sources. S2 freezes List 1 for selected ports,
B8000 mapping, CRTC/mode geometry, frame publication, reset and timing rows.
S3 maps List 2 to VADP and copied presentation consumers. Implementation
closes the complete selected gap batch through one VADP path.

## Boundaries

No renderer-owned guest state, EGA/VGA expansion, composite model or parallel
video path.
