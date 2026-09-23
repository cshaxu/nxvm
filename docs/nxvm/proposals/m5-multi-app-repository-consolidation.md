# M5 Multi-App Repository Consolidation

## Outcome

Keep `nxvm` as the repository name while making it a coherent home for the
NXVM and MyNES applications.  Shared Lib/Common/x86 infrastructure remains at
the repository root; each application has its own source, tests, documents,
tools, task sequence, release artifacts, and external-asset contract.

## Scope

- establish root shared governance and product-local documentation roots;
- correct root `AGENTS.md`, `README.md`, and `CONTRIBUTING.md` for the
  multi-application repository;
- make CMake traverse shared components plus independent NXVM and MyNES
  product build/test entry points;
- keep product CMake modules under `cmake/nxvm` and `cmake/mynes`;
- preserve a single root MIT license and consolidate notices without losing
  provenance;
- separate product tools under `tools/nxvm` and `tools/mynes`, importing the
  owner-authorized MyNES tool corpus;
- version the contents of `assets/binary-nxvm` and `assets/binary-mynes`, while
  continuing to exclude protected firmware, guest media, and ROM assets;
- update governance tooling so it validates the shared root and each selected
  product documentation tree without treating one product's task queue as the
  other's state.

## Non-goals

- No emulator feature, CPU/device behavior, ROM content, or external asset
  change.
- No protected firmware, guest media, game ROM, or machine-local path enters
  source control.
- No forced unification of product-specific task numbering, design documents,
  or tools.

## Design

The root owns only shared policy, shared source/test components, CMake
bootstrap, root licensing/notices, and repository orientation.  `docs/nxvm`
and `docs/mynes` each own their own current packet, queue, TODO, roadmap,
design, evidence, history, and product source policy.  Shared rules name the
product-local current packet as the authority for product work.

The root CMake project remains named `nxvm`; it configures common C11/toolchain
policy and calls product entry points.  Product-specific targets, generated
assets, artifacts, and tests remain inside their product CMake module and
source/test subtree.  A root aggregate may run all selected product tests, but
each product remains independently buildable and testable.

## S-task Plan

1. Create the shared/product documentation governance topology and migrate
   task-reading paths without losing either product's records.
2. Establish root and product CMake entry points, product test aggregates and
   product-owned module directories; build both products.
3. Consolidate tools, licenses/notices, artifact tracking and release paths;
   run documentation and build/test verification, then record closure.

## Completion Standard

The repository has one root MIT license; root governance documents correctly
describe the two-product layout; each product retains an independent document
sequence and task queue; product tools and release binaries are versioned in
their own directories; CMake exposes independent NXVM/MyNES build and test
targets; and neither external ROM/media policy nor a duplicate implementation
path has been introduced.
