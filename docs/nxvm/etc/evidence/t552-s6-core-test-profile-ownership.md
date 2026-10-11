# T552 S6 — Core Test Profile Ownership

## Reviewed Edge

`test/core/setup/model40_profile.h` is test-only support for two Core-owned
host tools. It reads a selected `vm_machine` construction and uses the
MyDeskPro386 profile observation API only to expose Model 40 diagnostic data.
No production file in `src/core` includes an App header or links an App
target; `src/core/verify_corpus.cmake` continues to enforce that boundary.

## Disposition

| Receiver | Predicate owner | Disposition |
| --- | --- | --- |
| `pc_profile_floppy_boot_matrix` | Generic Core INI/floppy boot matrix | Repaired: removed its optional Model 40 FDC-terminal print. It was not an assertion, did not change exit status and gave a generic integration tool an unnecessary App observation edge. |
| `nxvm_byob_dos_boot_probe` | Generic Core diagnostic boot probe | Retained: its conditional Model 40 memory/D4 reporting is diagnostic-only and selected only after inspecting the already-built machine. It neither creates a second composition path nor decides test success. |
| `model40_profile.h` | Core diagnostic support | Retained and narrowed: the unused whole-profile observer was removed. The remaining two helpers identify Model 40 and capture its D4 diagnostic observation only. |

The remaining edge is a test-only inward observation of the selected concrete
profile. It is not a production Core-to-App dependency and does not make an
App test depend on another App's support. Moving it into MyDeskPro386 would
incorrectly make the generic Core BYOB diagnostic own an App-specific runner.

## Verification

Both x64 and x86 build the two affected Core receivers:

- `vm-profile-floppy-boot-matrix`;
- `vm-byob-dos-boot-probe`.

Core test manifest and corpus gates also pass. No artifact input changes;
the S5 PC artifacts remain current.
