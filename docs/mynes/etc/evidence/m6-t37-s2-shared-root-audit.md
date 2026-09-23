# M6 T37 S2 Four-Root SoftPC Audit — Superseded

This historical record compared a mutable SoftPC worktree. It is not an
import authority: S3 compares only SoftPC commit `6251f896` and records the
correct result in `m6-t37-s3-committed-corpus-import.md`.

## Inventory

The compared relative-path inventories are identical:

| Root | MyNes files | SoftPC files | Equal bytes | Different bytes |
| --- | ---: | ---: | ---: | ---: |
| `src/lib` | 97 | 97 | 88 | 9 |
| `src/common` | 23 | 23 | 21 | 2 |
| `test/lib` | 49 | 49 | 46 | 3 |
| `test/common` | 20 | 20 | 18 | 2 |

`MANIFEST.sha256` differs in each affected root only because it records the
same source/test changes below.

## Classified Differences

| Root | Files | Classification | Consequence |
| --- | --- | --- | --- |
| Lib source/test | `storage/file*`, platform file declarations/implementations, and two storage tests | Intentional MyNes atomic-replacement extension. It adds flush/replace capability and `lib_storage_file_replace_atomic`. | Do not import SoftPC verbatim now: doing so removes App's atomic snapshot save guarantee. The capability must be proposed upstream before a future exact corpus import. |
| Lib README | `src/lib/README.md` | Upstream audio documentation divergence, unrelated to executable API. | Retain MyNes copy for corpus consistency; no product repair is needed. |
| Common source/test | `session/control_state.c` and its matrix smoke | Intentional MyNes no-window-while-nonrunning invariant. | Do not import SoftPC verbatim now: doing so reopens a confirmed paused/stopped Window regression. Transfer the invariant upstream before a future exact corpus import. |

## Decision

The four roots have no unexplained paths or byte differences. They are not yet
an exact reusable SoftPC corpus, because MyNes carries two deliberate shared
contract extensions. This S does not modify either repository. A later shared
proposal must first reconcile the atomic replacement and nonrunning-window
invariants upstream, then repeat the same four-root hash audit before import.
