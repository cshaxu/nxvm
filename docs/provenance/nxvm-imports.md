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
when needed, `git log --all -- docs/provenance/`. New imports must record the
same facts in their commit and in this index's next bounded update. The current
distributable notice set is [`THIRD_PARTY_NOTICES.md`](../../THIRD_PARTY_NOTICES.md).
Independent third-party notices still require a separate review before import.
