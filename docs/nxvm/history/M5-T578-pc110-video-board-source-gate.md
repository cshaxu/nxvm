# M5 T578 — PC110 Video And Board Initialization Source Gate

## Closed Task

T578 begins after closed T577 classified the first firmware reset as an
ordinary 8042 request and corrected its two App-local prerequisites.  The
first bounded post-reset observation instead reaches the firmware's 61h
status-bit edge wait before C000/video-ROM or the formerly assumed
`F000:4CA9` INT 10h checkpoint.  Available sources do not prove the required
PC110 status source or period, so T578 closes as a source gate rather than
fabricating an App behavior.

The retained [task proposal](../proposals/m5-pc110-video-board-source-gate.md)
and [S1 evidence](../etc/evidence/t578-s1-pc110-video-board-source-gate.md)
record the exact disposition.  The next MyPC110 qualification receiver must
explicitly admit any L2 compatibility decision; T578 makes no product, boot or
release claim.
