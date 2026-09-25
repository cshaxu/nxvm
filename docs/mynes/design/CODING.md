# Source Layout

This is MyNes's source, naming and vocabulary map. Apply
[Coding Rules](../../rules/CODING.md). Dependency and state ownership are defined
only by [System Architecture](ARCHITECTURE.md).

## Current And Target Trees

The source map is the current repository organization, not permission to create placeholders.
The actual delivery state is [Current](../states/CURRENT.md).

```text
src/
  lib/                         shared platform/C services
  common/{machine,session,ui}/  shared coordination
  x86/{debug,xasm32}/           optional x86 capabilities, not MyNES runtime
  app-mynes/
    product/                   main, config, command, debug, keyboard, composition
    core/                      flat hardware and driver implementation
  app-nxvm/                    independently owned NXVM product
test/
  register.cmake               shared test registration input
  lib/                         independently selectable Lib suite
  common/                      independently selectable Common suite
  x86/                         independently selectable x86 suite
  app-mynes/
    unit/{product,core}/
    integration/
  app-nxvm/                    independently owned NXVM tests
tools/{shared,mynes,nxvm}/
assets/{mynes,nxvm}/
```

Directories are introduced with real cohesive source. Tiny App modules can
begin as adjacent files and move into these subdirectories when justified.
Cartridge formats and mappers stay in the machine's cartridge owner;
device-private registers stay with their device. Common UI groups surface bindings; session groups event/prompt
dispatch. Generic native input normalization belongs in kvm-base and its leaves.
Graphics-to-text conversion belongs with Core frame adaptation under core;
its algorithm tests belong under `test/app-mynes/unit/core`. It derives text cells from the
completed PPU image and does not duplicate PPU rendering. kvm-console contains
text-frame presentation/input only; its Lib tests use text fixtures and graphic
rejection cases. No App-specific terminal renderer is introduced.
No generic utils directory, global singleton registry, compatibility shell or
plugin directory is pre-created.

### NXVM Machine Style Reference

The owner requires Core to preserve the historical NXVM machine style as closely as the
NES ownership model permits. The owner's confirmed MyNes layout is flat
`src/app-mynes/core` and `test/app-mynes/unit/core`: reuse NXVM's
file/responsibility idioms, not its machine directory
level. Hardware and driver integration live together under `core`, with
adjacent responsibility-based C/header pairs, rather than one top-level Core
directory per chip. `machine_interface.h` is the opaque machine entry;
`machine.h` holds private assembly state. CPU execution, instruction semantics,
bus, cartridge, clocks and later devices remain cohesive named files. Split a
large owner only when independent responsibilities justify it; no placeholder
files are required by the target tree.

Public `*_interface.h` headers expose durable copied values and narrow operations;
plain owner headers contain private state. Use explicit instances and owner
prefixes, machine-owned wiring/time, named configuration/results, provider/context
callbacks at genuine boundaries and straightforward bounded C control flow.
CPU's bus provider remains the same production boundary used by its tests.
File proximity never permits a device to mutate a peer's private state.

Follow NXVM's organization and ownership idioms while retaining the adopted
`lib_*` type/C vocabulary and MyNes formatting rules. Do not introduce NXVM's
`type.h`, `type_unsigned_*`, `C_*` or `STD_*` as a competing facade, copy x86
devices/firmware paths, or reproduce PC-specific timing/qualification machinery.
The inspected references and exact revisions are recorded in the
[reference register](../etc/hardware-references.md#structural-and-style-references).
All Core component tests live directly under `test/app-mynes/unit/core`, including driver,
media and debug tests. No tests belong beneath src.

### Machine File Responsibilities

Machine is an ownership concept and file prefix here, not a subdirectory.

| Files | Sole responsibility / introduction |
| --- | --- |
| driver_interface.h, driver.h, driver.c | Opaque core_driver, Common driver construction, callbacks and signal latches; M2. |
| media.c | Existing removable-media callback, bounded Lib file read, candidate construction and commit; M2. |
| debug_interface.h, debug.c | Pointer-free protocol values and executor-side NES request dispatch; M2. |
| machine_interface.h, machine.h, machine.c | Opaque deterministic hardware, private assembly, reset and bounded execution; M2. |
| cpu_interface.h, cpu.h, cpu.c | CPU pins/bus contract, private register/microcycle state and tick progression; M2. |
| cpu_instructions.h, cpu_instructions.c | Private instruction metadata and semantics used by the one CPU engine; M2. |
| bus.h, bus.c | CPU address decoding, RAM and data latch; M2. |
| cartridge.h, cartridge.c | Pure ROM parsing, PRG/CHR ownership and mapper logic; M2. |
| clock.h, clock.c | Guest clock/device scheduling as real device consumers arrive; M3/M4. |
| ppu.h, ppu.c, controller.h, controller.c | Guest graphics and controller devices; M3. |
| input.c, display.c | Driver key-to-button mapping and frame output/ASCII conversion; M3. |
| apu.h, apu.c | Guest audio device; M4. |
| audio.c, pacing.c | NES sample conversion/stream submission and guest-to-host speed policy using Lib; M4. |

The table records responsibilities and their introduction milestones, not pending
implementation. Display and input now have their own files. driver.h is private to the integration files;
hardware never includes it or Common. Private device headers remain device-owned.
There is no core/host, separate Core debug directory or extra core_interface.h
facade. Composition includes driver_interface.h; App debug includes only the
value-only debug_interface.h. Common owns workers/queues/publication and Lib owns
native resources; none of these machine files creates another host runtime.

## Files And Names

### Names And Headers

Use lower_snake_case for files, functions, variables, typedefs and struct tags.
Public symbols identify the owner: `core_cpu_*`, `core_machine_*`,
`common_machine_*`, `product_command_*`, `lib_storage_*`. Enumerators and constants
use upper-case owner prefixes, for example `CORE_MACHINE_STOP_BUDGET`.
Private helpers are static and use concise role names within their owner.

Public cross-module contracts use `*_interface.h`; adjacent implementation files
use responsibility names. An opaque handle is declared with an incomplete struct
typedef, never a typedef hiding pointer indirection. Public value records describe
copied facts, options, spans and results, not a writable internal state layout.
Use `*_provider` and the shared machine driver for real injected behavior.
Core machine driver implements the machine driver; Common never includes Core headers.

Private headers have plain responsibility names such as `decode.h` or `registers.h`.
Avoid `_private`, `_internal` or `_native` filename suffixes: include visibility
establishes privacy. Headers have path-derived include guards such as
`CORE_CPU_INTERFACE_H`; product include paths are rooted at `src/app-mynes`,
for example `"core/machine_interface.h"`; shared includes are rooted at `src`. Headers compile with only their declared
prerequisites; consumers do not rely on accidental transitive includes.

### Shared Type And C Vocabulary

`lib/types/types_interface.h` is the one header-only foundation, using NXVM's
spelling; consume the repository's reviewed canonical Shared baseline:

| Vocabulary | Meaning |
| --- | --- |
| lib_u8, lib_u16, lib_u32, lib_u64 | Exact-width unsigned values. |
| lib_i32, lib_i64 | Exact-width signed values where required. |
| lib_size | Native object/buffer length, equivalent in range to size_t. |
| lib_bool, LIB_TRUE, LIB_FALSE | Project boolean vocabulary. |
| lib_status, LIB_STATUS_* | Common operation status and named categories. |
| LIB_NULL | Null-pointer spelling. |
| lib_allocate, lib_reallocate, lib_release | Common allocation vocabulary, with normal explicit ownership. |
| lib_memory_copy, lib_memory_move, lib_memory_set | Common byte-memory vocabulary with documented preconditions. |

Use LIB_STATUS_OK, INVALID_ARGUMENT, INVALID_STATE, UNSUPPORTED, NO_MEMORY,
IO_ERROR and LIMIT_EXCEEDED categories as applicable; numeric ABI assignments
are not a serialized format. Guest stop/fault reasons are typed owner-specific
values in result records, not additional layer-specific success/error systems.
`char` represents text/code units; `void` and typed pointers retain normal C
meaning. `int` is allowed where C requires it, such as main's return type or a
comparison result. Guest addresses and counters use deliberate fixed widths,
never native pointer or platform integer widths.

Common Types owns the used ISO C declarations and one-to-one vocabulary.
Core/Common/App do not directly call malloc, printf, fopen, time or SDK functions.
OS declaration groups, when needed, live beneath `lib/types/win32/` and may be
included only by matching Lib platform implementation files. They preserve native
signatures, not product policy. Allocation/memory wrappers are this deliberately
small vocabulary boundary, not permission for arbitrary forwarding abstractions.
Preserve imported compatible vocabulary and its source layout; add vocabulary
only with a real consumer. Do not make a competing MyNes type facade.

### Entry And Platform Names

`src/app-mynes/product/main.c` owns process entry and delegates composition and command dispatch.
App config owns defaults/effective settings; command owns monitor grammar,
debug owns NES debug policy and keyboard owns mappings/hotkey policy. They use
one Common session route and public machine/UI operations. They do not implement
native Console I/O or import SoftPC x86 assembly/debug commands.
Only App composition includes core/driver_interface.h. App command/debug may
include core/debug_interface.h for copied protocol values only; requests
still travel through Common, never a direct Core runtime call. Core machine driver contains NES
driver/input/frame/debug and loading adaptation; hardware remains in the
machine-centered files above. No separate vm, compat or mvdm directory is needed.
Lib service implementations place platform code under their own
`win32/` directory with neutral public contracts. Unsupported hosts fail build
selection or return an explicitly documented unsupported result; they do not
silently use successful empty stubs. Additional hosts are not promised by this map.

## Source Organization

Headers live next to their implementations. Module-local helpers remain local;
shared behavior moves to its actual neutral owner only when callers need the
same contract. Split or merge files by responsibility, independent change and
readability, never a line-count quota. Do not expose a helper just to test it.

Tests live under the matching repository-root `test/` owner. MyNES repository-only
tests use `test/app-mynes/unit/{product,core}`; admitted external-ROM scenarios
use `test/app-mynes/integration`. Shared suites use `test/{lib,common,x86}`. Core tests are flat under
`test/app-mynes/unit/core`, without an extra machine directory. There is no test/support:
fixture helpers stay beside the tests of their owning component, or within
integration when specific to integrated scenarios. Existing shared tests remain
product-neutral. The build gives each component only its declared
include/link dependencies; one executable or aggregate target cannot bypass them.
The [App/Core design](../etc/app-core-contract.md#build-and-test-ownership)
defines the component build/test boundaries. Actual product targets and artifact
identities are created only by source admission, never by design scaffolding.

### Shared Source Adoption

SoftPC supplied the initial four-root neutral subset; these paths are now shared
repository roots, not a private MyNES copy:

| Transfer unit | SoftPC source | MyNes target |
| --- | --- | --- |
| Lib source | src/lib | src/lib |
| Common source | src/common | src/common |
| Lib tests | test/lib | test/lib |
| Common tests | test/common | test/common |

Keep internal component paths/names and source/test manifests stable. Placement
of the four roots may differ by product; root paths are explicit build inputs,
not edits to shared source or product-name conditionals. Transfer `test/register.cmake` alongside the four roots; each suite remains
independently buildable and selectable. The optional x86 source/test pair uses
the same helper. There is no parallel `src/test/` tree or staging code copy.
Product driver tests belong under `test/app-mynes/unit/core` or
`test/app-mynes/integration`.

The adopted neutral corpus excludes x86 debug/assembler implementations.
Receiving x86 products compose their own optional x86-debug/x86-xasm32 modules
outside these four roots. MyNes creates no such modules and performs no extraction.

This repository is the canonical Shared publisher. Owner-approved Shared work
may update its neutral components and manifests, with all receiving Apps audited.
Transfers pin a committed revision and preserve those bytes; sibling repositories
remain read-only without separate authorization. App work cannot create a
MyNES-specific branch or local copy of Lib/Common.

Existing imported formatting remains intact. New product code follows C11;
the adopted shared baseline has passed standalone strict-C11 builds. Source
provenance distinguishes unchanged transfer units from externally owned adapters.
