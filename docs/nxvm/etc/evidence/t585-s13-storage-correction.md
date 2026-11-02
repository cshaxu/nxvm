# T585 S13 — correction of the internal ATA attribution

The preceding S12 used the Open-Source-PC110 `Discovery/Storage/readme.md`
statement that internal CF reaches ATA through Ricoh. A deeper read of the
same reference checkout `02ccf2e8e19b298a2cf04c5d430278eb087cc368` found
an explicit correction in `Discovery/Pluto/readme.md` and
`Discovery/Live-Dump/README.md`: in the observed real configuration the
internal SanDisk disks are ATA master/slave, while the populated Ricoh socket
is a separate sound/SCSI card. The updated Service Manual identifies U22's
ATA path. The two notes are inconsistent, so an old statement and a BIOS
window example do not justify moving the existing internal ATA behind Ricoh.

Current PC110 keeps its single directly registered ATA and its verified DOS
boot. Ricoh's I/O-window decoder remains without an emulated external I/O
card receiver; that is a real separate scope, not proof that internal ATA is
wrong. A draft generic Core dynamic-port API passed focused tests but was
withdrawn before commit because no concrete production receiver had yet been
established. No second bus or unused Shared API is delivered.

The draft dual-width build was intentionally stopped; it is not a pass or a
failure. The draft source, test and the eight products rebuilt against it were
restored byte-for-byte to the verified S11 commit `9df4fa5dc`; MyPC110,
MyNES, MyRaidenII, media and configuration were not changed. S11's complete
dual-width 712/712 unit suites, 48/48 registered integration suites, three
additional PC-profile suites and 14 checked release artifacts remain the
applicable source baseline. Future builds will refresh the affected cached
objects from that restored source before use.
