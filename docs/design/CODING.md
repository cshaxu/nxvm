# Source Layout

This is the current macro source-layout authority. Apply the local
[Coding Rules](../rules/CODING.md) when changing it. Detailed M5 layout and
contract evidence is supporting material indexed in
[etc/README.md](../etc/README.md); it cannot override this file.

## Current And Target Trees

The current M5 tree contains `src/type.*`, `src/core/`, `src/vm/`, and a
non-runnable `src/vdm/` skeleton. `src/mantle/` and `src/dos/` appear only when
their admitting milestones begin; empty placeholder roots are prohibited.

```text
src/
  type.*
  core/{utils,machine,platform,product}/
  vm/
    main.c
    composition/
    {machine,platform,product,profile}/
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
`vm_product_*`, `mantle_platform_*`, `dos_machine_*`, and `vdm_product_*`.
`src/type.*` is the sole system-wide type and status foundation.

Files remain flat within a module until a real multi-file subsystem justifies a
subdirectory. `main.c` and `composition/` belong at the appropriate component
root.

## Source Organization

The retained test layout uses one repository-root `test/` directory.
Repository-only test modules mirror their source owner: `test/core/` follows
`src/core/` and `test/vm/` follows `src/vm/`; a directory is introduced only
for a real source subsystem or cross-owner composition boundary. `test/support/`
contains setup-only helpers, never a second product path. External-asset
product scenarios live only in `test/integration/`; they are not unit tests.
