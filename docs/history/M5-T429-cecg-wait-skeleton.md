# M5 T429: CECG Wait Skeleton

T429 closes three bounded generic-AT receivers: CECG port waits, explicit D4
conventional-RAM classification excluding the aperture, and a separate CECG
aperture wait. Accepted implementations are `1c02a853`, `f73cb976` and
`d54cf372`. Core owns lifecycle and waits; Model-40 selects ranges; 5170 is
isolated. Evidence: [S1](../etc/evidence/t429-s1-cecg-8bit-bus-wait.md),
[S2](../etc/evidence/t429-s2-d4-cecg-memory-class.md), and
[S3](../etc/evidence/t429-s3-cecg-aperture-wait.md).

No physical duration, IOCHRDY/NOWS/BUSRDY behavior, raster/monitor/firmware
behavior or DeskPro L3 conclusion is accepted. Those retain their explicit
TODO receivers.