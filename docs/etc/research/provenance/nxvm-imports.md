# NXVM Import Index

The NXVM copyright holder authorizes imports into this repository under the
root MIT License. The source foundation is NXVM commit
`6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`; its original complete M1 manifest
covered 75 `src/` files copied byte-for-byte, excluding guest media, payloads,
Bochx, root build files, and upstream documentation.

M3--M5 then moved or adapted those units into the current `core/`, `vm/`, and
`vdm/` ownership model while preserving source notices. Each import/move commit
records the source and destination paths, authorization, changes, and focused
verification. The detailed per-unit manifests were consolidated by M5 T208 to
avoid treating migration paperwork as current architecture; they remain
recoverable from Git history. Future `mantle/` or `dos/` work must identify
whether it derives from NXVM or is project-owned before admission.

For an exact unit record, use `git log --follow -- <current-source-path>` and,
when needed, `git log --all -- docs/etc/research/provenance/`. New imports must record the
same facts in their commit and in this index's next bounded update. The current
distributable notice set is [`THIRD_PARTY_NOTICES.md`](../../../../THIRD_PARTY_NOTICES.md).
Independent third-party notices still require a separate review before import.

## Canonical Shared Library

M5 T530 initially imports the project-owner-provided shared library from
SoftPC commit `3cabea6a6a0ce931a5480f3549599ebb74f3681d`, source path
`src/lib/`, to this repository's `src/lib/` byte-for-byte. M5 T531 S2 then
replaces that entire 93-file corpus byte-for-byte with the clean
`e140ec55bbc4cd40956948e57e58dc678e54f699` revision after its exact source
review. Both source revisions are project-owned MIT material and carry no
independent third-party notice. These imports exclude every other SoftPC path,
including app/MVDM code, tests, firmware, media and binaries. T530/T531
evidence records consumer adaptations and verification.

M5 T531 S10 replaces the same complete 93-file corpus byte-for-byte with
SoftPC commit `2ea35ce87bb538a6cc520be1353641a99a94d427`, revision
`shared-t55-s20-p2`. The source remains project-owned MIT material with no
independent third-party notice. The import again excludes every SoftPC path
outside `src/lib/`; its unrelated working-tree asset change is not an input.

M5 T531 S11 replaces the complete corpus byte-for-byte with SoftPC commit
`987d82e5e87559a00b910a21d2c715d4b844dcae`, revision
`shared-t55-s21-p2`. This 94-file project-owned MIT corpus renames the neutral
`ui-*` component roots to `kvm-*`; it remains free of independent third-party
notices. The import excludes every SoftPC path outside `src/lib/`, including
the unrelated dirty `assets/binary/softpc.ini` worktree asset.
