# T592 S3 — Restore the external OmniBook media

The published files are byte-identical to the S2 verification inputs. In the
repository deployment directory, both executables returned exit code 1 before
composing the monitor. The external System Card and C image expected by the
adjacent INI layout were absent. The S2 native boot evidence used a populated
disposable layout and did not establish that the owner's local published
directory contained its runtime inputs.

The offline NXVM tool restores the owner-supplied C ZIP to an active
FAT16 partition on a 32-cylinder raw disk with 16 heads and 63 sectors per
track. The tool is independent of build, Product startup and ATA execution.
It reads the backup, preserves the 8.3 hierarchy and contents, and creates a
new output exclusively. It does not alter the archive or install the System
Card. The [external input guide](../../../assets/myob300/README.md) names the
matching archives, geometry, startup and access-mode boundaries.

Independent readback checked the signed MBR/BPB, the active partition,
both identical FATs, allocated cluster chains, directory parent links,
all 13 directories and all 43 files byte-for-byte. The System 1.0 ABA backup
SHA-256 stayed `4d3f6fbadfabfb056ad42d1dc82007f9d7ca57026fffb648903ae6135bf5715d`.
The restored raw image SHA-256 is
`1b146fb6d17c3025a571699217715f21d80dcd4e23005319595ef5cb76d2b325`.
A second restore to the same path refused to overwrite it, and a backup with
a traversal entry was rejected before creating an output.

The x64 and x86 Release copies launched as ordinary processes in a separate
local asset layout with the unmodified published INI. A foreign working
directory did not affect configuration lookup. Each ran `info`, reported
the 16,515,072-byte disk connected, and accepted `exit`. The source archives
and the sibling asset checkout were unchanged.

With the exact external System Card present, both binaries rejected missing,
511-byte and zero-byte C images and an invalid INI access mode before the
monitor appeared. The valid source image remained byte-identical. These
start-time cases did not exercise a storage fault after guest startup.

The local setup verification reaches the monitor with attached media. The
original, unmodified C backup proceeds into a separate ROM Windows path after
`start`. The original-C Windows boot, Window/RDP interaction and guest power
qualification remain outside this launch result.
