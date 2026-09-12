# Source Layout

This is the current macro source-layout authority. Apply the local
[Coding Rules](../rules/CODING.md) when changing it. Detailed M5 layout and
contract evidence is supporting material indexed in
[etc/README.md](../etc/README.md); it cannot override this file.

## Current And Target Trees

The current M5 tree contains `src/lib/`, `src/core/`, `src/vm/`, and a
non-runnable `src/vdm/` skeleton. The M5 target adds `src/common/` only for
shared session, machine, UI, xasm32 and Debug components. `src/mantle/` and
`src/dos/` appear only when their admitting milestones begin; empty placeholder
roots are prohibited.

```text
src/
  lib/{types,console,host,storage,ui-base,ui-console,ui-window}/
  common/
    {contracts.h,session,machine,ui,xasm32,debug}/
  core/{machine,product}/
  vm/
    main.c
    {app,machine,platform,product,profile,session}/
  mantle/
    composition/
    {machine,platform,product}/
  dos/{machine,platform,product,profile}/
  vdm/
    main.c
    composition/
    {machine,platform,product,profile}/
```

The diagram is a target source map, not permission to create every directory
today. A temporary adapter or baseline reference root requires an explicit
admission and does not become a permanent source root.

## Files And Names

Headers stay beside their implementations. A public cross-module contract is
named `*_interface.h`; an injected implementation is named `*_provider`.
Public symbols use their ownership path, for example `core_machine_*`,
`common_session_*`, `common_debug_*`, `vm_product_*`, `mantle_platform_*`,
`dos_machine_*`, and `vdm_product_*`. `src/lib/types` is the sole shared C
type, status, atomic, and basic C-runtime vocabulary foundation.

Files remain flat within a module until a real multi-file subsystem justifies a
subdirectory. `main.c` and `composition/` belong at the appropriate component
root.

## Source Organization

The retained test layout uses one repository-root `test/` directory.
Repository-only test modules mirror their source owner: `test/core/`,
`test/common/` and `test/vm/` follow their corresponding source roots; a
directory is introduced only for a real source subsystem or cross-owner
composition boundary. `test/support/`
contains setup-only helpers, never a second product path. External-asset
product scenarios live only in `test/integration/`; they are not unit tests.
