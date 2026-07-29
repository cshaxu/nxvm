# Source And License Boundary

## Project Code

Project-owned code and documentation are released under the MIT License in the
root `LICENSE` file. No third-party source import occurs until its terms are
reviewed against this license and the intended distribution model.

## NXVM

The local NXVM checkout declares LGPL-3.0-or-later. It is a permitted research
and potential derivation source only when a provenance record identifies the
exact commit and copied/derived paths, preserves notices, and the resulting
distribution meets LGPL requirements. Reimplementation from observed behavior
is preferred for BIOS, POST, DOS, and platform code because NXVM itself labels
parts of those areas as quick-and-dirty.

## NTVDMx64

NTVDMx64 is a behavioral reference. Its public documentation, issue history,
and user-observable behavior may produce requirements and tests. This project
does not copy its source, patches, private-symbol strategy, loader injection,
or Windows internal integration techniques.

## WineVDM

WineVDM/otvdm is GPL-2.0. M4 may execute a user-installed or separately
distributed WineVDM process using a documented command contract. It is not
linked, embedded, copied, or bundled by default. Any later bundling or IPC that
creates a combined work requires an owner-approved GPL-compatible distribution
decision and a dedicated license review.

## Assets And Test Programs

DOS system files, BIOS images, disk images, commercial applications, and
unlicensed test executables are local inputs. Their identity and hash may be
recorded; bytes and screenshots containing protected content are not committed
without explicit redistribution approval.
