# M5 T533 Fixed-Machine Products

Owner admitted this task on 2026-09-21 to implement the approved fixed-profile
product architecture while retaining all implemented XT, AT, DeskPro 386 and
default PC/AT machines. Each executable will select exactly one Profile at
build time and resolve its lawful firmware solely from the build-provided
external BYOB asset root. The retained candidate proposal is
[Fixed-machine products and architecture consolidation](../proposals/m5-fixed-machine-products.md).

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
