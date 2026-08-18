# M5 T410: CPU External-Cycle Observation

T410 adds a Core-owned observation lifecycle around classified CPU physical
memory accesses. P1 (`92b3bad4`) proves begin/commit ordering without changing
transaction ownership, HOLD/HLDA, retirement time, or D4 waits. This boundary
is not a prefetch queue or overlap model; the original 2 KiB sequential-request
condition remains the next receiver.