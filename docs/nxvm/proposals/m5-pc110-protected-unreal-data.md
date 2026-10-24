# MyPC110: protected unreal-data reload

## Admission

T576 S2 follows the committed C-shadow receiver.  The normal external TFT
firmware diagnostic reaches `F000:605F` on both widths after the T575 cache
receiver, where `POP DS` loads selector `0008` while PE remains set and the
selected descriptor is all zero.  No visible register has selector `0008`, so
the existing narrow visible-cache policy correctly declines it.

The exact `F000:605F` bytes are firmware evidence.  Open-Source-PC110 and the
two external PC110 emulators independently describe this firmware family as
using real-address-style data segment reloads while PE is set.  Their behavior
is L2 research only; no third-party source, address-specific rule, hard-coded
selector map, BIOS service, or media byte is imported.

## Objective

Add one explicit, strict-by-default CPU L2 policy for protected **data** segment
reloads whose descriptor is exactly zero:

1. reuse an already visible matching data cache when one exists;
2. otherwise load normal real-address data attributes for the requested
   selector (`selector << 4`, 64 KiB, 16-bit writable data).

MyPC110 selects the policy.  Other boards retain strict descriptor loading.

## Boundaries

- The owner is X86 Core CPU because descriptor interpretation is a CPU fact.
  The policy name remains generic and does not know PC110, firmware addresses,
  ROM banks or board ports.
- This S affects DS/ES/FS/GS data reload only.  It does not relax CS, SS,
  descriptor-present/type/privilege checks for nonzero descriptors, gates,
  far transfer, interrupt return, task state, or V86.
- Existing `VISIBLE_CACHE` behavior stays available and stable.  The new
  policy is opt-in and composes the existing visible-cache step before the
  real-address fallback.  Strict remains default.
- No Lib, Emulator, X86-NXVM, application firmware mapping, protected media,
  BIOS interception, second executor or PC/AT fallback changes.

## Verification

Core tests prove strict rejection, visible-cache reuse, fallback attributes,
and rejection of any nonzero invalid descriptor.  MyPC110 tests prove its
explicit policy selection.  The normal external TFT diagnostic advances beyond
`F000:605F` on x64 and x86 and records the next finite boundary, if any.
Affected dual-width Core/MyPC110 tests, dependency/manifest gates and a
minimal diff review are required.  Advancement is not a POST, Setup, DOS boot
or product-release claim.

## Stop conditions

Stop and transfer if progress requires code/stack/far-control relaxation,
hardware-specific selector bases, a source-unproved controller behavior, or a
change outside this contract.  Do not turn an L2 data-reload policy into a
general loose-protected-mode emulator.
