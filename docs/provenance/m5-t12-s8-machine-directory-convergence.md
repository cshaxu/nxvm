# M5 T12 S8: Machine Directory Convergence Provenance

The files flattened from `machine/core/contract` into `machine/core` are
project-owned M3 contract sources. They were moved with `git mv`; no NXVM
baseline source was copied, rewritten, or altered. The obsolete `src/core`
headers were forwarding shims only and were deleted after all callers changed
to the colocated `machine/core/*.h` headers.

`src/nxvm-baseline` and `src/adapters/nxvm_baseline` remain unchanged as
transition/reference areas. Their removal from the formal product is still M5
T13 work.
