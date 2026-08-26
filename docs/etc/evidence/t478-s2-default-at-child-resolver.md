# T478 S2 Default-AT Child Resolver

`M5:T478:S2:DEFAULT-AT-CHILD-RESOLVER:OK`

`M5:T478:S2:DEFAULT-AT-CHILD-NEGATIVE:OK`

## Result

`default-at` now has one typed VM child declaration of `pc-at-5170`. Its
Core, device, port, memory-window and policy values are child-owned; unchanged
IRQ and DRQ routes remain root-owned. The child accepts only two pre-freeze
options: paired CPU/FPU selection and memory selection. Values are selected
into the child declaration before generic resolution and are rejected when
their option bit is absent/unknown or selected memory is zero.

The generic resolver is unchanged. It receives its existing requested-option
mask and continues to validate copied topology, route conflicts, contract IDs
and policy. Core receives no Default-AT identity and owns all mutable machine
state after the later session materialization.

## Simplicity Review

The profile module now has one shared descriptor-to-resolver-values helper and
one shared copied descriptor snapshot helper. The former root-only snapshot
type/capacity names were generalized because the same immutable storage is now
used by the root and child; no second descriptor arrays or forwarding adapter
were added. The current direct Default-AT session materializer remains only as
the explicit S3 deletion target; it is not hidden behind the new resolver.

Tracked source/test delta for S2, excluding status/evidence: `+251/-53`, net
`+198` lines, from `git diff --numstat` over the four changed source/test paths.
The positive increase is the finite child request/validation and its focused
regression; the shared helpers remove the former duplicated root construction
body.

## Verification

```text
cmake --build --preset current-gcc --target vm-profile-resolver-smoke vm-ibm-5170-root-resolver-smoke
build/mingw-gcc-x64/vm-profile-resolver-smoke.exe
build/mingw-gcc-x64/vm-ibm-5170-root-resolver-smoke.exe
```

Both executables exit zero. The generic resolver emits its retained positive
and negative markers; the root/child smoke proves copied Default-AT identity,
parent, Core values, field provenance, HDC/CGA graph, copied snapshot
independence, invalid parent, unknown option and zero-memory rejection.

## S3 Boundary

S3 consumes this immutable result in `vm_session_create`, converts the existing
Default-AT configuration once into the typed request, and deletes the direct
descriptor selection, direct Core materialization and post-copy memory override.
It must preserve the existing product-visible allowed CPU/FPU/memory behavior
and all session-owned media/lifecycle resources.
