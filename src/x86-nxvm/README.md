# X86 NXVM

Owns NXVM-family peripheral chips, PC boards, the Emulator Machine adapter and
family Product. Matching tests live under the same component paths. Each chip
has one implementation and target under `chips/<chip>`; chip code depends on
Lib Types, with KBC's declared keyboard connection as the only chip dependency.
No chip consumes board, Machine adapter or Product policy.

Machine owns the standard/turbo speed contract. Product's information and
opaque callback bundle consumes that contract and Emulator's composition
descriptor; it does not define another speed enum or pacing implementation.

`board-base` composes peripheral routes, media, video and guest clock wiring;
`board-at` and `board-xt` own their distinct electrical adapters. `machine`
adapts the composed board to Emulator Machine. `product` owns the NXVM-family
INI grammar, factory, version/banner construction and command extensions.
Machine does not depend on Product. Fixed model choices belong to the Apps.

CPU/FPU and neutral guest execution belong to X86 Core, not this package.
No implementation remains in the retired Core root. Component tests consume
inward production contracts, not private CPU or Machine layouts. Repository
manual-ledger qualification and cross-App checks are registered separately
by `cmake/nxvm`; standalone component tests do not require those inputs.
Current task evidence, not this ownership description, records qualification.
