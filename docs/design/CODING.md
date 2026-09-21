# Source Layout

This is the current macro source-layout authority. Apply the local
[Coding Rules](../rules/CODING.md) when changing it. Detailed M5 layout and
contract evidence is supporting material indexed in
[etc/README.md](../etc/README.md); it cannot override this file.

## Current And Target Trees

NXVM is a single bootable-machine product. `src/lib/`, `src/common/`, and
`src/x86/` are shared components; `src/app/` and `src/core/` are NXVM-owned.
Empty future-product roots are prohibited.

```text
src/
  lib/{types,console,base,console-broker,storage,kvm-base,kvm-console,kvm-window}/
  common/
    {contracts.h,session,machine,ui,xasm32,debug}/
  app/
    main.c
    request_interface.h
    {catalog,command,composition,config,keyboard,recorder,version}/
  core/
    core/
    machine/
      media/
    profile/
```

The diagram is a target source map, not permission to create every directory
today. A temporary adapter or baseline reference root requires an explicit
admission and does not become a permanent source root.

## Files And Names

Headers stay beside their implementations. A public cross-module contract is
named `*_interface.h`; an injected implementation is named `*_provider`.
Public symbols use their ownership path, for example `core_machine_*`,
`common_session_*`, `x86_debug_*`, and `vm_app_*`. Existing stable symbol
prefixes need not mirror a directory rename. `src/lib/types` is the sole shared C
type, status, atomic, and basic C-runtime vocabulary foundation.

Files remain flat within a module until a real multi-file subsystem justifies a
subdirectory. `main.c` and `composition/` belong at the appropriate component
root.

## Source Organization

The retained test layout uses one repository-root `test/` directory.
Repository-only test modules mirror their source owner: `test/app/`,
`test/core/core/`, `test/core/machine/`, `test/core/profile/`, `test/common/`,
and `test/x86/` follow their corresponding source components. A directory is
introduced only for a real source subsystem or cross-owner composition boundary. `test/support/`
contains setup-only helpers, never a second product path. External-asset
product scenarios live only in `test/integration/`; they are not unit tests.
