# T553 S13 Core Test Fixture And INI-Owner Repair

## Scope

This corrective repair implements the two confirmed S12 receivers only:

- `exception_fixture.h` and `memory_alias_fixture.h` are moved from duplicate
  Core/x86 and Core/board-base copies to the existing `test/core/support/`
  owner; and
- the NXVM Architecture authority is corrected to name `core/product` as the
  owner of the shared NXVM.ini grammar and executable-relative runtime-media
  path resolution.

The fixture headers use the public `core/x86` interface, but they are consumed
by both Core/x86 and Core/board-base tests. They are therefore Core test support,
not a child-owner fixture and not a top-level shared-x86 fixture.

## Exclusions

No production source, public API, Lib, Emulator, top-level x86/Product, App,
asset, INI, snapshot or executable changes are admitted by this S.

## Verification Results

1. The old duplicate paths are deleted. A Core-tree search finds all 22 direct
   callers using the single `support/` owner and no former local include form.
2. Core manifest, corpus and test-manifest gates pass on x64 and x86. The
   broader Core static boundary set also passes: x64 11/11 and the available
   x86 equivalent 9/9 (the x86 configuration does not register the
   dependency-DAG-negative receiver).
3. The existing x64/x86 Ninja+ccache trees reconfigured and compiled every
   affected target. The 19 registered direct Core/X86/Board unit receivers pass
   on x64 and x86.
4. NXVM documentation governance and `git diff --check` pass. Final scope
   review finds only Core-test fixture relocation, Core-test manifest, NXVM
   architecture/task/evidence documentation; it finds no production, Shared,
   App, asset or executable input change.

No desktop, external-media or integration runtime test is required or claimed:
this S changes neither a runnable path nor a product artifact input.
