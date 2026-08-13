# M5 PC/AT Digital Video Completeness

Reconcile the VADP CGA/EGA register, CRTC, memory, raster, and copied-frame
contract against a selected digital display matrix.  Complete only the modes
and register behavior demanded by that matrix; preserve one VADP owner and the
copied presentation boundary.  VGA/VBE, composite simulation, and host
renderer timing are separate decisions, not implied scope.

