# T553 S1 — Four PC App Ownership Ledger

## Scope and method

This documentation-only inventory covers the four fixed PC application roots:
`app-my5160`, `app-my5170`, `app-mydeskpro386`, and `app-nxvm`. It classifies
apparent overlap by owner rather than treating similar spelling as a defect.
No firmware, media, INI semantics, assets, production sources, tests, or public
interfaces change here.

The inventory used current file roots, direct C/C header includes, App CMake
source/link definitions, registration files, artifact directories, and the
configured App-boundary checks on both existing default x64 and x86 Ninja
graphs.

## Owned roots

| App | Source | Test | Deployed root | Current owner boundary |
| --- | ---: | ---: | ---: | --- |
| My5160 | 9 files | 3 files | `assets/my5160` (3 files) | Fixed XT-5160/268 composition, ROM mapping and 360K boot route. |
| My5170 | 8 files | 8 files | `assets/my5170` (3 files) | Fixed IBM 5170/339 PC/AT construction, two firmware images and 1200K route. |
| MyDeskPro386 | 19 files | 36 files | `assets/mydeskpro386` (3 files) | Model 40/D4 memory, platform, observation and two-firmware composition. |
| NXVM | 29 files | 63 files | `assets/nxvm` (4 files) | Default 386 PC/AT composition plus self-authored firmware build input. |

Each root has a fixed `main.c`, binding, profile construction path, App-owned
unit/setup/integration registration and adjacent executable/INI deployment
root. PC-family version and generic startup machinery are correctly received
from Core rather than copied into these App roots.

## Dependency and test-boundary result

All direct production edges point inward from the App to Core and the shared
corpus. No App production or test source includes a sibling `app-*` root. The
only `app-*` includes found are self-owned profile, binding or fixture headers.
The configured x64 and x86 graphs both report all four independent App
boundaries, all four App-test-support boundaries, and the selected default
composition graph as passing.

The selected-profile CMake orchestration in `cmake/nxvm/` is a family build
owner: it registers all four boundary gates and rejects a non-selected App
source from the selected composition graph. It is not a production sibling
dependency.

## Candidate disposition

| Candidate | Disposition | Reason / receiving work |
| --- | --- | --- |
| `mydeskpro386-profile` links `core-machine` twice. | App-local defect. | Remove the duplicate CMake link item in T553 S2; no target contract changes. |
| Four `main.c`/binding pairs have near-identical startup shape. | Valid App distinction. | Each owns a different product name, fixed machine descriptor and construction callback; generic startup is already Core-owned. |
| My5170 and NXVM have similar PC/AT plan/profile code. | Valid distinction. | Both already share Core PC/AT preparation; remaining BIOS kind, CPU/memory policy and media facts are machine-specific. |
| Profile CMake files repeat fixed asset/profile selection mechanics. | Retain. | They encode distinct BYOB firmware paths, hashes, artifact roots and target identities. No neutral helper is proven simpler. |
| MyDeskPro386 has larger D4/Model-40 source and test inventory. | Valid distinction. | It owns D4 platform/memory, dual-disk and observation contracts absent from XT and PC/AT Apps. |
| App-local integration registrations look similar. | Retain. | They bind distinct fixed INIs and external boot terminals; they are not interchangeable unit fixtures. |
| Firmware construction under `app-nxvm/firmware`. | Valid App distinction. | It is self-authored NXVM firmware input, not shared hardware abstraction or protected imported firmware. |
| App documentation/tool roots are not per-App directories. | Valid project topology. | NXVM-family docs/tools/version remain unified while source, tests and artifacts are App-parallel. |

No Core or Shared transfer is admitted by this inventory. No retired/dead App
path was found. The sole demonstrated S2 repair is the DeskPro duplicate link
item; lifecycle, INI, firmware, media or artifact behavior needs later S3
evidence and a reproduced owner-local predicate.

## Static verification

On 2026-10-10, reconfiguring existing `build/t548-s33-nxvm-{x64,x86}` Ninja
graphs passed all four App boundaries, all App-test-support boundaries, and the
selected composition graph. This is static ownership evidence only, not runtime,
integration or desktop qualification.
