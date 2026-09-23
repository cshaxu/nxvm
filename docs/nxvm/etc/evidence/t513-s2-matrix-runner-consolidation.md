# T513 S2: Matrix Runner Consolidation

`test/integration/dos/vm_profile_floppy_boot_matrix.c` is the sole normal
T513 scenario runner.  Its one static data table contains the 20 S1 rows; one
configuration factory maps a selected row to the existing `vm_session_config`
and passes BYOB manifests unchanged to session construction.  It does not
reimplement a profile, CPU contract, FDD geometry, BIOS, DOS parser, Core
deadline, trace provider, or guest state.

The runner owns only test policy:

- `--validate` proves the finite table is structurally valid and has 20 unique
  row IDs without external files.
- a normal invocation checks owner-provided input availability first, then
  reports the existing copied display terminal as `dos-prompt`, `date-input`,
  or `installer-ready`.
- missing input returns CTest skip code 77; a present but invalid image or a
  failed session construction remains a test failure.

The retained BYOB boot probe stays an unregistered diagnostic tool.  Its
trace/private-Core diagnostics are not normal matrix policy and are not called
by the new runner.  The matrix runner only reads the VM-owned copied
presentation mailbox after the existing session lifecycle publishes it.

Focused proof:

- table validation reports `T513:PROFILE-FLOPPY-MATRIX:VALID:20`;
- a missing owner input reports `UNAVAILABLE` and returns 77.

The changed source/test surface is one CTest executable declaration and one
integration runner.  It adds no production owner, public ABI, duplicated
profile/FDD selector, or external asset material.  CTest registration and
actual external execution remain S3 and S4 work.
