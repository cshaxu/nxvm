# M5 PC/AT Digital Video Completeness

## Objective

Reconcile the selected PC/AT digital CGA/EGA display matrix through one VADP
owner: register programming, CRTC state, video-memory interpretation, raster
and retrace state, deterministic timeline publication, and copied presentation
frame.  The work establishes the full selected display-state graph before it
repairs individual modes or ports.

## Dependencies And Completion Standard

This candidate consumes T346's deterministic timeline and T351's completed
input/KBC graph.  It precedes port-topology and L3 bus-timing convergence
because display port and memory transactions must have one truthful state and
readiness owner first.

Completion requires a primary-source and source/proof ledger for every
selected CGA/EGA state, register, memory aperture, raster/retrace transition,
and copied-frame boundary.  Each supported row must be implemented and proven,
or transferred to one named later receiver with an admission condition.  It
does not imply arbitrary VGA compatibility or host-renderer timing fidelity.

## Proposed Subtask Sequence

| Subtask | Bounded outcome |
| --- | --- |
| S1 | Create the complete selected digital-video ledger: manual/profile row, VADP state owner, writer/reader, timeline event, focused proof, and S2--S5 or transfer disposition. |
| S2 | Reconcile CRTC/register programming and deterministic display-enable/retrace/raster lifecycle for the selected matrix. |
| S3 | Reconcile selected CGA memory/port/mode interpretation and copied-frame publication. |
| S4 | Reconcile selected EGA planar memory/port/mode interpretation and copied-frame publication. |
| S5 | Compose the selected digital state graph, prove cross-mode/reset/timeline/presentation isolation, produce the task artifact, and close or precisely transfer every residual row. |

The ledger may collapse S3/S4 only if it proves they share the same owner,
state, validation, and publication boundary.  It must not manufacture a
generic renderer abstraction merely because both paths produce pixels.

## Non-goals And Transfers

VGA/VBE/SVGA registers and modes, composite/NTSC simulation, light pen,
unselected CGA geometry, arbitrary 6845 breadth, host-window pacing, host GPU
acceleration, firmware additions, and Windows guest execution are outside this
candidate unless a revised packet receives owner approval.  Existing complete
digital-CGA and composite TODOs remain explicit receivers until S1 reclassifies
them from a selected source/probe contract.

## Mechanism Discipline

One VADP state owner retains register, aperture, raster, and copied-frame
publication.  Equivalent mode variants share validation and publication; a
separate branch needs a real layout, electrical, timing, or selected-profile
difference.  Each repair records writers, readers, reset/finalize behavior,
timeline order, failure/nonpublication, and caller impact before code changes.
