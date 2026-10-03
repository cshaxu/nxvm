# T540 S92 Whole DMA Aggregation Evidence

Baseline: clean accepted S91 37d46f58d. S92 is active, not accepted.
The local project-owned source retains its original notices and root MIT
authorization; no external code, protected input or new asset is imported.

The ledger assigns the complete bus owner: ports/pages, transfer latch,
primary/secondary arbitration, provider bindings, copied request nonces,
Core memory-cycle calls, reset and destruction. App retains clock conversion
and physical PIT-refresh/FDC/HDC wiring. One opaque aggregate replaces embedded
mutable controller/latch layouts. Callbacks borrow scoped transfer values only.
No raw chip getter, second route/registry or scheduler is admitted.

The intake queries recorded by the ledger find 17 source and 23 test files.
These are reference hits, not a fixed edit-count ceiling. Shared DMA matrix
coverage must retain its original assertions; board/Core transaction and real
firmware tests retain their actual NXVM owner. Complete proof and artifact
delivery remain required before any implementation P or acceptance.

## Implementation Progress

The whole dma_bus source and private layout now physically live in
x86/ibmpc-common. One allocated opaque bus owns both controller endpoints,
page state and persistent transfer latch. The copied request nonce moved from
the App controller header into the new DMA public interface; App producers
retain only the aggregate handle. The original port map and arbitration/cycle
helpers are retained. Provider calls use a scoped scratch copy and commit it
back immediately, preserving byte/word effects without exposing the persistent
latch address. The nonce allocator still issues identity only, never selecting
an instance or holding a registry.

The full NXVM production core-machine target compiles with its existing strict
warnings. The independent public-contract regression passes for both single
and dual controllers: page read/write/reset, optional secondary decode,
duplicate channel rejection, issued request assertion/deassertion and copied
signals. This focused result is not whole-matrix or S acceptance proof.

The original DMA matrix now builds and runs independently with a public
neutral Core instance. All 126 first-service rows and the original hardware
cases remain. Same-owner DMA observations stay with DMA, not App. Original
Core-private sticky registration and third-route allocation failure assertions
are retained in test/x86/core/dma_route_rollback_smoke.c, using its real Core
owner and the public DMA constructor. DMA aggregate allocation failure and
atomic conflicting-route rollback are tested through the independent bus.
These three regressions pass. The matrix's first-service fixture now uses the
minimum public Core allocation rather than a synthetic 64-KiB stack RAM;
its tested addresses and expected bytes/cycles remain unchanged.

Seven rebuilt x64 App regressions pass: FDC transfer/media change, XT Xebec
transfer/EOP, Model40 dual topology/reset, DMA/RTC authority/refresh, Core
transaction order and port-construction rollback. App no longer reads DMA
chip/connection fields. Duplicate-binding rejection replaces owner-pointer
assertions; actual device/refresh transfers retain the behavior proof. Each
App transfer fixture declares its original single/dual controller count,
instead of obtaining it from private controller layout. Xebec's direct
terminal callback receives a scoped scratch value, never the owner's latch.

Complete tests/gates, source comparison, fresh products and checkpoint runs
remain required; no implementation commit or artifact acceptance has occurred.
MyNES, owner INIs and external assets remain unchanged.

## Final Source Verification

The final source passes complete root units on x64 and x86, 474/474 each;
both specialized gate targets pass. Independent Shared tests pass 138/138,
and the tools-disabled build passes 132/132. The original DMA matrix still
executes its 126 first-service rows. The Core rollback receiver retains the
original conflicting-route, sticky-error and third-allocation failure checks.

Actual relocated-source comparison confirms the port arrays, page/address
formula, byte/word memory-cycle preflight and primary/secondary arbitration
bodies retain their algorithms. Changed production behavior is confined to
opaque aggregate allocation/publication/destruction and scoped provider-value
delivery. LIB_UPTR_MAX replaces its equal native constant; the App FDC wiring
bound remains four channels, equal to the removed private constant.

The readiness gate initially rejected the old path of the unchanged identity
token issuer. Its inventory now names the actual Shared owner; both final
specialized runs pass. Three DMA boundary negative fixtures reject private
includes, embedded old controller fields and a second App implementation.
The product build script's native stderr handling was corrected without
changing production code. The independent negative replay initially retained
its own injected duplicate file; cleanup now makes the fixture repeatable,
and baseline plus all three negatives pass again. No repository duplicate
DMA source was found. These results do not authorize T540 closure.

## Current Product Identity

All eight rebuilt deployed EXEs have the 0.5.0540 banner, the expected PE
architecture, no compiler-debug sections, and modification times later than
the final DMA/Core inputs. Runtime debugger functionality remains linked.
The generated product and boot-probe link lists contain one normal x86-core
archive and one ibmpc-common DMA owner; no observable Core is co-linked.
Boot checkpoint acceptance is recorded separately, not inferred from hashes.

## Size And Ownership Review

Across tracked source/test C and header paths, Git reports 34 changed paths,
1,710 added and 1,598 removed lines, net +112. This includes the relocated
matrix represented as delete/add; it is not 1,247 newly invented test lines.
The increase supplies the public opaque contract, failure-atomic allocation
and independent public/owner-local regression receivers. It does not add a
forwarding controller or framework. Three embedded App state objects become
one borrowed opaque bus handle; one DMA implementation remains. Providers
retain their existing device state and may only borrow transfer scratch.
Physical clock conversion and refresh/FDC/HDC signal wiring remain App-owned
until their complete board receiver, not duplicated inside DMA.

## Final Checkpoints And Delivery Review

All eight final Release products and their rebuilt INI boot probes pass one
checkpoint each, without changing owner INIs: default x64/x86 reach dos-prompt;
XT, IBM 5170 and Model40 x64/x86 reach installer-running. Each product/probe
link list has one normal Core and one common-board archive. All eight neutral
Core link proofs pass. Model40's final x86 probe completed normally; no retry
or success substitution was used. This proves the finite S92 checkpoint set,
not indefinite absence of intermittent failures or complete T540 acceptance.

Both complete units, independent/tools-off tests, specialized gates, six full
manifest inventories/hashes, Shared corpus, DMA boundary negative replay,
documentation governance and Git whitespace checks pass. App source has no
raw DMA controller/latch layout or private DMA include. MyNES tracked paths,
owner INIs and external assets are unchanged. Shared P1 owns the physical DMA
source/test migration and independent build; NXVM P2 owns callers, fixtures,
gates, task evidence and the eight current products. Coordinator review of
the actual pushed commits remains required before S92 acceptance.

| Product | Bytes | SHA-256 |
| --- | ---: | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1336490 | `AF2C5D69B752B5914C81069EF8909657F45EBCDF155C868C2AD1F8B4241C2C57` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1506306 | `B9E5EF4616DBD37000CD51C9D5D4FDEE215C16FFD17400D2E63AFEC6B5A388F5` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1352807 | `3516D2DC65E62F7FF6C38871D6E244CC75389915286DF1FF7A0F858D71ACD76D` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1522621 | `F72A5FA5B9F842128882934EA7A884AEACE066DB0BE1E99F8E228DBA46ED389C` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1352775 | `284220D172B49B856B194D88B084123BC3CAC648D1D8B9E04E101076B9A7F9C8` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1522588 | `85D45801468ACCB82CFED2F5A963E2B2E59E3BC9546A74FB5E2BD59DB786E51A` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1352841 | `9DB51D62CFD8E103B9BC6945B4E7F5D60ED43D9C94BE8F184CC23F3CA49D5505` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1522656 | `BAE1230ECFF64F1896F871E135DF0D6834EE7EAAF2B3535E222A3CB280D4E528` |
