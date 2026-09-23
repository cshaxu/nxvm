# T523 S6 Generic File-Mechanics Owner Ledger

## Disposition

| Former path | Replacement | Reason |
| --- | --- | --- |
| `core/platform/file.*` owned read, exclusive create, replace and remove | `lib/storage/file.*` | The operations are host byte/file mechanics, carry no Core state and were duplicated verbatim. |
| Core-local reader/writer opaque handles | `lib_storage_file_reader` / `lib_storage_file_writer` | Line iteration and text recording are generic file mechanics; handles remain opaque and expose no native `FILE`. |
| BYOB, variable session assets and session YAML | `lib_storage_file_read_owned` | VM retains validation, provenance and configuration policy; library owns only the bytes. |
| VM debug recording | `lib_storage_file_writer` | VM retains debugger/record format policy; library owns open/write/close mechanics. |

## Retained Owners

FDD/HDD geometry, sector selection, write policy and paired persistence remain
VM media semantics. Core has no direct host file caller. The product-owned
status conversion in debug maps the library's bounded I/O result to the
pre-existing VM public status; it does not create a second file API.

## Similar-Issue Sweep

The repository-wide production sweep finds no remaining `core_platform_file_*`
symbol or `core/platform/file.h` inclusion. `type` still exposes its existing
standard-C facade for process streams and product debugger commands; it is not
a Core file-mechanics owner and is outside this replacement. The migrated
asset, media and debugger-record paths do not use that facade.

## Verification

- `storage-file-smoke`: pass (`M5:T523:S6:STORAGE-OWNER:OK`), covering opaque
  text writer/reader, owned byte read and cleanup.
- Focused VM debugger recording, session catalog and Model 40/BYOB paths:
  pass.
- Repository-only unit: 311/311 pass.
- Refreshed file-resource and atomic-media boundary checks: pass.
- 56-file manifest: exact; product-vocabulary and peer-root sweeps: no hits.
