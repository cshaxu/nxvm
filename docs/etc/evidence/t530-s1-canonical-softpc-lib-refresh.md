# M5 T530 S1 - Canonical SoftPC Lib Refresh

## Scope and source

This S imports only the owner-provided shared-library corpus from SoftPC
commit `3cabea6a6a0ce931a5480f3549599ebb74f3681d`, revision
`shared-t55-s13-p1`. The finite source is the 93 files below SoftPC `src/lib/`;
the destination is NXVM `src/lib/`.

The source is project-owned MIT source as recorded by NXVM's third-party
notice. No SoftPC application, MVDM, firmware, media, executable or test
source was imported.

## Exactness proof

- SoftPC `git ls-tree` at the fixed commit: 93 files.
- NXVM `src/lib/`: 93 files.
- Each NXVM file hash equals the corresponding blob hash at that commit:
  `different=0`.
- The imported library manifest and component-dependency verifier pass.

SoftPC's live worktree has later, uncommitted edits in eleven library files.
They are deliberately outside this fixed import; they neither alter the
declared source commit nor create NXVM-local divergence.

## Consumer adaptation

The imported types contract retires NXVM's former text/status facade. The
single Common owner is adapted directly to the new public Lib vocabulary:

- `common/machine` and `common/session` report their generic stale state as
  `LIB_STATUS_INVALID_STATE`; `NOT_CURRENT` remains Console-specific in the
  canonical library.
- `common/xasm32` and table-driven `common/debug` retain their parsing and
  command bodies, while using the canonical types declarations and
  `types/file.h` ISO-C stream declaration vocabulary instead of retired Lib
  formatting/tokenization wrappers.
- `common/ui` performs its bounded title copy directly with canonical copied
  value/memory helpers.

`types/file.h` is the canonical Lib-provided external C-runtime declaration
header. It carries no product state and is the one documented types-vocabulary
exception to the `*_interface.h` contract naming convention.

No Core, VM, profile, firmware, controller or guest production source required
an API adaptation. The only non-Common changes are: current T530 artifact
identity in root CMake/presets, a Lib types smoke update for the retired public
facade, and task/provenance governance records.

## Verification

- `cmake -DCORPUS_ROOT:PATH=src/common -DCORPUS_NAME:STRING=Common -P src/common/verify_manifest.cmake`
  - pass.
- `cmake -DLIBRARY_ROOT:PATH=src/lib -P src/lib/verify_manifest.cmake`
  - pass.
- `cmake -DLIBRARY_ROOT:PATH=src/lib -P src/lib/verify_component_dependencies.cmake`
  - pass.
- `verify-current-specialized-gates` - pass.
- `ctest -L unit -j 8 --output-on-failure` - 299/299 pass.
- `ctest -L integration -j 4 --output-on-failure` - 42/42 pass (487.37 s).
- Stripped Release artifacts were rebuilt and architecture-verified:
  - `nxvm_0_5_0530_x64.exe` SHA-256
    `41D0149F96335E2AD0CAF109AE12913D224EC8CEB8CCD78BB6DC332DE787E596`.
  - `nxvm_0_5_0530_x86.exe` SHA-256
    `C65DB7F24086D1513EC0FE7DE37CC27FF86B0882BB8EEF140F4773391EFCB2E2`.

The task remains open for owner testing; this evidence does not claim T530
closure.
