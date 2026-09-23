# T497 S3 IBM 5170 Floppy Current-Code Gap List 2

`M5:T497:S3:IBM5170-FLOPPY-LIST-2:OK`

| List 1 rows | Current owner/path | Gap | S4 result and proof |
| --- | --- | --- | --- |
| F1 | `src/vm/profile/default_profile/pc_at_profile.c` Model-339 descriptor and its validator | It declares field-upgrade true and CMOS `0x40` (1.44 MB). | Change only the immutable descriptor/validator to no field upgrade and `0x20`; composition/topology smoke asserts it. |
| F2, F5 | Model-339 branch in `src/vm/composition/session/session.c` | It leaves `session->floppy_kind` at enum zero, currently 1.44 MB. | One Model-339 selection helper maps absent/`1200k` to existing 1.2 MB geometry before the common FDD constructor. |
| F3, F6 | Existing `session->floppy_kind` to `vm_session_machine_devices_initialize_media()` path | The FDD already owns 360 KB geometry, but Model 339 has no request path to select it. | The same helper maps explicit `360k` to existing geometry, while the descriptor CMOS is untouched. |
| F4 | `src/vm/composition/session/session_factory.c` and Model-339 construction | Factory rejects all non-default profile format requests; direct construction has no Model-339 validation. | Permit only Model-339 `360k`/`1200k`; reject `720k`/`1440k` through the same helper, including direct config. |
| F7 | Existing Core FDC and common FDD initialization | No gap. | Static sweep and retained FDC topology smoke prove no controller behavior change. |
| F8 | No current physical-drive timing receiver | No implementation claim. | Retain absence; no timing literal or deadline is added. |

## Single Repair Boundary

S4 changes the immutable Model-339 descriptor and one session-local format
selector only. The selector produces the existing `vm_profile_floppy_kind`,
which the existing common machine-device initializer copies into the FDD.
Provider validation delegates to that same selection rule instead of retaining
its present broad non-default rejection. No function infers media from image
size, and no FDC, BIOS, Core or FDD mutable state is duplicated.

Focused proof extends the Model-339 composition/topology smokes with default,
explicit-1.2 MB, explicit-360 KB and rejected-720/1.44 KB cases; retained
Default-AT four-format tests prove the generic profile is unchanged.
