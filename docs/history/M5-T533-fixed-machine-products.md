# M5 T533 Fixed-Machine Products

Owner admitted this task on 2026-09-21 to implement the approved fixed-profile
product architecture while retaining all implemented XT, AT, DeskPro 386 and
default PC/AT machines. Each executable will select exactly one Profile at
build time and resolve its lawful firmware solely from the build-provided
external BYOB asset root. The retained candidate proposal is
[Fixed-machine products and architecture consolidation](M5-T533-fixed-machine-products-proposal.md).

## S1: Baseline And Contract Ledger

S1 inventories every existing machine constructor, reset/provider branch,
asset role, configuration key, build target and owned test before relocation.
It freezes the direct profile/build/INI contract and maps each retained machine
and CPU variant to its next migration receiver. S1 changes no production
behavior; its ledger is the prerequisite for later source moves.

Accepted in `14572230` after independent review: the ledger has a finite
four-machine scope, preserves all existing CPU code and repository-only tests,
assigns every observed construction/configuration/build route to S2--S5, and
does not make a protected-asset claim. The x64 isolated build passed 336/336
repository-only unit tests in 226.85 seconds; documentation governance and
`git diff --check` passed.

## S2: Devices And Profiles Relocation

S2 performs the atomic owner-path move from `core/core` to `core/devices` and
from `core/profile` to `core/profiles`, including their repository-only test
mirrors and all live CMake/static-verifier references. It deliberately leaves
Machine composition, product configuration and runtime behavior unchanged for
S3 and S4.

Accepted in `687db585` after actual-change review. The live old-root sweep is
empty, the shared fixture's six relative references use its new Devices path,
and no asset or runtime route changed. A fresh x64 configure and the complete
repository-only unit suite pass 336/336; documentation governance and
`git diff --check` pass.

## S3: Profile Composition And Machine Adaptation

S3 makes Profiles the only board-selection, board-resolution, firmware-binding
and board-materialization owner.  A frozen opaque machine plan carries copied
BYOB data, Core configuration/topology and its provider/context to Machine.
Machine keeps one generic prepare/bind/run/release route for direct and
file-backed inputs; it no longer branches on a firmware kind or retains
parallel XT/Model-40/default/5170 resolved state.

The Model-40 topology/controller composition moved under its Profile.  The
only Machine descriptor projection is read-only diagnostics, not construction
authority.  Repository-only tests preserve the retained four machine profiles;
white-box failure injection that constructed a second Machine descriptor now
validates at Profile construction, where that authority belongs.

Accepted in `2a589dc7` after actual-change review.  The full repository-only
unit target, focused XT/5170/Model-40/default regression set, documentation
governance and dependency-DAG gate pass.  The selected-plan source sweep found
no former Machine firmware-kind or board-private branch; no protected payload,
runtime configuration or executable artifact changed.

## S4: Fixed Builds And App INI Cutover

S4 replaced runtime YAML/catalog/profile selection with one CMake-selected
Profile binding and one adjacent `NXVM.ini` parser. Each retained machine now
has one fixed product target; manifests validate only the local BYOB asset root,
while the INI supplies runtime media, access modes, memory and presentation.

Accepted in `7874b683`. The four product targets have no runtime alternate
machine, CPU or firmware selector, and every integration scenario uses the same
production INI route. Firmware and media remain external, never embedded or
committed.

## S5: Product Convergence And Closure

S5 removed the duplicate host-input transport, making a composed machine submit
copied input through Common's sole executor FIFO. It also corrected the HDD-only
integration probe to remove the declared floppy through the production media
owner and reset before checking VBR handoff. Final display publication now
precedes pause acknowledgement, preventing a paused observer from seeing a
stale frame.

Accepted in `99de6d11` after actual-change review. The complete repository-only
suite passes **335/335** at four-way parallelism; optimized default integration
passes **20/20** serially and **20/20** at four-way parallelism. All four fixed
products have stripped x64 and x86 Release artifacts, matching adjacent INIs,
only in `assets/binary/<profile>/`. The complete four-product ledger and
artifact evidence are retained in the
[S5 convergence ledger](../etc/evidence/t533-s5-product-convergence-ledger.md).

## Task Closure

T533 closes with one Profile-owned board/firmware construction route, one
build-selected product per retained machine, one App INI route, one Common
input executor route, and one executable deployment location. XT, 5170, Model
40 and default PC/AT remain retained; CPU implementations and repository-only
coverage remain intact. No YAML compatibility loader, runtime profile/CPU/
firmware selector, copied-media test route, shared generated product
configuration or duplicate artifact deployment remains.
