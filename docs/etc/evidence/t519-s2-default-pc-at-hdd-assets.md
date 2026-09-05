# T519 S2 - Default PC/AT fixed-disk asset closure

`hdd_50m_win31.img` is 51,609,600 bytes: 100,800 512-byte sectors.  Its MBR
has `55AA` and an active FAT16 partition at LBA 63.  The selected CHS is
therefore `100 x 16 x 63`.

The original generic 1.44 MiB ROM copied a zero BDA fixed-disk count and an
INT 41h pointer to `F000:0000`; its own INT 13h handler reads that pointer.
The HDD asset pair now provides a ROM BDA count of one and an INT 41h table at
`F000:F7E0` with cylinders 100, heads 16 and sectors 63.  Its matched CMOS
seed declares type 47 and the same geometry.  The generic no-disk 1.44 MiB
ROM/CMOS remains zero-drive, so media presence is not inferred by Core or VM.

The only YAML changes select the matched HDD ROM/CMOS pair for the two YAMLs
that already declare `hdd_50m_win31.img`.

Verification:

- `integration.vm-hdc-hdd-boot-smoke`: MBR and partition VBR handoff, ATA
  command `20h`.
- `integration.vm-windows31-int13-trace-probe`: AH=08 reports 100/16/63 and
  DL=1; two AH=02 reads succeed.
- `integration.vm-windows31-checkpoint`: DOS transitions `A:\\>` to `C:\\>`;
  `DIR` performs ATA reads.
- `integration.vm-product-default-profile-smoke`: the no-disk and HDD YAMLs
  both remain catalog-valid.
