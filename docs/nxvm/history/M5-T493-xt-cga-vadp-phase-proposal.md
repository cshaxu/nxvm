# IBM 5160 CGA VADP Phase Contract

## Purpose

Complete selected XT CGA function and timing at VADP, the sole video
port/memory/mode/frame-snapshot owner.

## Required sequence

S1 validates original IBM/CGA sources. S2 freezes List 1 for selected ports,
B8000 mapping, CRTC/mode geometry, frame publication, reset and timing rows.
S3 maps the complete List 1 universe to VADP and copied presentation consumers
as List 2. S4 immediately closes every implementation gap recorded by that
finite List 2 batch through the one VADP path, including removal of any
superseded guest-video route. It must not transfer a discovered in-scope
implementation defect to TODO or a later task.

## Immediate-repair rule

S4 is a complete owner-local correction, not a sequence of opportunistic
patches. A List 2 row is either already proved at the retained VADP owner,
implemented and covered by a focused regression in S4, or excluded by this
proposal's explicit boundary. The S4 evidence records the final disposition of
every row and names the single retained state, ingress and snapshot-publication
path. Any newly discovered capability that changes the selected CGA universe
requires owner approval before it can enter a later T; it cannot leave a known
row from this universe unfinished.

## Boundaries

No renderer-owned guest state, EGA/VGA expansion, composite model or parallel
video path.
