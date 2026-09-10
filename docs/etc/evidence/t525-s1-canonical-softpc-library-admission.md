# M5 T525 S1: Canonical SoftPC Library Admission

## Frozen Candidate

- **Repository:** owner-controlled `O:\repos.hobby\softpc`.
- **Commit:** `140af7ab9b988ef5613c1702036f387b282603c3`.
- **Corpus:** exactly its 70 tracked `src/lib` paths.
- **Manifest file SHA-256:**
  `665CA0E5AD9A9C9F3B5CB105E3082E3FDDECD80A553A75BD94236FC7A8DDEC37`.
- **Manifest check:** `cmake -P src/lib/verify_manifest.cmake` passed in the
  candidate checkout.

The owner authorized importing this project-owned corpus.  A full source-tree
notice sweep found no independent copyright, licence, SPDX or permission notice
under the candidate `src/lib`; no third-party source, firmware, media, binary,
SoftPC application or MVDM source is admitted.

## Vocabulary And ABI Disposition

The candidate has no `SoftPC`, `Insignia`, `MVDM`, `CCPU` or `CVID` token in
its library source.  It has no native SDK type in a public root header.  Win32
types occur only below `*/win32/`; `FILE` occurs only below `storage/internal/`.

The remaining `VM`, `guest` and `monitor` prose identifies generic copied
presentation/input semantics or explicitly rejects product meaning.  It does
not expose a machine/session pointer, controller, ROM, BIOS, profile or
product lifecycle.  The standalone CMake project label
`nxvm_shared_library` remains the owner-approved build-only naming exception;
it is not a public contract, source dependency or runtime identity.

The owner-approved raw-Console replacement discard is an explicit capability:
the old native input buffer is cleared only while replacing the logical Console
owner.  It neither adds a debugger flush API nor clears input during ordinary
product operation.

## Complete Root Mapping

| Current NXVM surface | Canonical disposition |
| --- | --- |
| `lib/base/base.h` | Replace with `base/base.h` plus neutral logical Console implementation. |
| `lib/host/{clock,sync}` | Replace with canonical host clock/sync and add canonical host Console broker. |
| `lib/storage/*` | Replace byte-identically; retain NXVM media topology only above lib. |
| unified `lib/ux/*` presenter, native runners and mailbox | Delete; replace with `ux-base`, `ux-window` and `ux-console` roots. |
| VM `ux_binding`, platform host-input/presenter wiring | Retain only as NXVM product adapter and rewrite against canonical public APIs. |
| VM display/session runner and Console ownership policy | Retain product ownership; remove calls to retired unified-presenter API. |
| lib and VM UX tests/CMake source lists | Replace or delete retired-path tests/list entries; add public canonical and NXVM coordinator proofs. |

The candidate CMake entry, verifier and all 70 listed paths are S2's exact
copy boundary.  The 52-path current NXVM corpus is not merged with it.  S2 may
therefore replace `src/lib` byte-for-byte; S3 alone owns NXVM binding cutover.
