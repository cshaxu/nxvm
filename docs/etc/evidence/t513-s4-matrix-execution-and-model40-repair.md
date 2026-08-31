# T513 S4: Matrix Execution And Model 40 Repair

## Observed Matrix

The complete 20-row owner-managed matrix was replayed from the Release build.
Every row reached a declared terminal: twelve `installer-ready` and eight
`date-input`.  The selected IBM 5160 row reached `installer-ready`; both IBM
5170 rows reached `installer-ready`; and the Model 40 row reached the real
firmware `resume-f1` checkpoint followed by `installer-ready`.  No row was
made synthetic or unavailable.

## Sole-Owner Repairs

- Model 40 now provides its immutable default CMOS material through its profile
  contract.  Core RTC/CMOS copies it into each session, owns the checksum, and
  retains subsequent BIOS changes across ordinary reset.  The runner only
  sends the real host F1 event; it does not write CMOS or bypass firmware.
- Model 40 declares both documented 1.2 MB floppy mechanisms while media
  remains independently optional per drive.
- The VADP EGA text snapshot uses the EGA Attribute Controller output gate,
  rather than the unrelated CGA mode-control bit.
- The Compaq WD1003 path retains one HDC state machine.  The controller's
  Drive/Head bit 4 selects its frozen C:/D: media slot; single-disk Model 40
  reads use either bit-4-clear value.  Reset publishes the WD1003 diagnostic,
  count and sector defaults.  The WD1003 behavior was cross-checked against
  the local 86Box controller source; no external code was imported.

## Verification

- Focused Model 40 CMOS, FDD, VADP, direct Compaq HDC and Model 40 HDC tests
  pass.
- Debug repository-only unit suite: 316/316 passed in 17.27 seconds.
- Release external integration suite: 40/40 passed.  It includes the complete
  20-row matrix and retained DOS, ATA and Windows checks; the Model 40 matrix
  row completed in 61.48 seconds.

The external ROMs and media remain owner-managed inputs; no path, hash or
binary is tracked by this evidence.
