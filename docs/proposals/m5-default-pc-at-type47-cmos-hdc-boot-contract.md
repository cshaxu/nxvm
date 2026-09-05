# M5 Default PC/AT Type-47 CMOS And Fixed-Disk Boot Contract

## Goal

Make every declared default-PC/AT session that attaches the selected 50 MiB
ATA image expose its DOS partition as BIOS drive C:. The result is one
external, immutable 64-byte CMOS seed whose type-47 geometry matches that
image, one complete project-derived external ROM boot template, one Core
ATA/HDC owner, and one integration assertion of BIOS-visible fixed-disk service.

## Established defect

`hdd_50m_win31.img` is a 51,609,600-byte raw disk with a valid `55AA` MBR and
an active FAT16 partition at LBA 63. Its configured 16 heads and 63 sectors
give exactly 100 cylinders. All current default-PC/AT seeds declare drive 0 as
type 47 (`12h=F0h`, `19h=2Fh`) but leave `1Bh--23h` zero.

The locally retained Bochs BIOS reference reads those bytes to construct the
type-47 fixed-disk parameter table before it services INT 13h. That is an
Other-L2 cross-check of the same BIOS-facing contract, not source to import.
The ROM also copies an all-zero BDA fixed-disk count and an empty INT 41h
parameter-table vector. Its own INT 13h code correctly uses the table and ATA
ports, but DOS receives no valid fixed-drive contract. The checkpoint
reproduces the consequence: DOS rejects C: without issuing ATA work.

## Boundaries

- The external `nxvm-assets/profiles/default-pc-at/{cmos,firmware}` files are
  the sole firmware configuration inputs. Their manifest hashes must be
  updated with the seed and project-derived ROM-template corrections; no media,
  Microsoft asset or YAML is copied into NXVM.
- A YAML names concrete matching ROM/seed and image assets. It does not carry CHS
  fields, calculate a checksum, or cause VM/Core to infer geometry at startup.
  A user-supplied different fixed disk must name its matching user-managed
  seed.
- Core continues to own ATA registers, commands, DRQ/IRQ/deadlines and media
  reads. VM does not add BIOS, BDA, INT 13h or ATA compatibility branches.
- The 64-byte MC146818 configuration range remains unchanged. This task does
  not introduce a 128-byte NVRAM container or alter user-managed session YAML.

## Planned subtasks

1. **S1 - source and asset contract audit.** Record the MBR/CHS derivation,
   CMOS type-47 field map and checksum convention. Audit every default-PC/AT
   YAML/seed pair and external manifest entry so no seed can declare type 47
   with an all-zero geometry block.
2. **S2 - firmware asset repair.** Correct the external default-PC/AT
   seed(s) that pair with the 50 MiB image, including the configuration
   checksum. Correct the same ROM's BDA template and INT 41h vector/table for
   that declared type-47 drive, then update their external manifest identities.
   Redirect only the two HDD YAMLs to those matched assets; leave every
   no-disk YAML on the zero-drive assets.
3. **S3 - BIOS-visible integration closure.** Replace the checkpoint's
   tolerated `c-drive-absent` outcome with a required C: prompt and evidence
   that BIOS issued at least one HDC command. Repair the test lifecycle only
   through its existing session control path; never manufacture BDA/INT 13h
   success. Run the complete integration suite and affected profile matrix.
4. **S4 - full audit and release closure.** Sweep every default-PC/AT fixed
   disk consumer, full unit and integration gates, actual-diff review and the
   required dual-architecture stripped developer artifact.

## Acceptance

- A default-PC/AT YAML with `hdd_50m_win31.img` reaches DOS and `C:\\>`;
  BIOS AH=08h reports the declared geometry and drive count, and BIOS has
  issued HDC work.
- Seeds, manifest identities and YAML-relative path resolution agree; no
  built-in CMOS/ROM fallback, runtime geometry inference or media copy exists.
- A YAML without fixed media remains a no-disk configuration rather than
  inheriting a phantom C:.
- Full repository unit plus external-asset integration gates pass, and the
  task emits current x64/x86 stripped Release artifacts.
