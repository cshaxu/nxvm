# M5 T540 S65 Neutral Construction Input

## Scope and owner cut

NXVM only; Shared and MyNES source, tests and artifacts remain unchanged.
The existing public `core_machine_config` is still the board composition's
frozen input. Its callers and the plan's sole stored copy are unchanged.
Board-owned public values move in S66, not by a simultaneous 152-consumer ABI
rewrite in this S.

The existing substantive neutral allocator now accepts only private
`core_machine_executor_config`. Board composition derives that temporary
copied value once, validates both owners, allocates Core, attaches the board,
then publishes the one machine. The stack value is not retained. No second
plan, runtime configuration mirror, allocation path or rollback is added.
The public create and both allocation-failure seams move together to
`machine_board.c`; their original Core-to-board creation order is preserved.

The thirteen neutral fields are RAM size; CPU and FPU models; 386 MOV-CR
behavior; A20 wrapping; base instruction ticks and instruction timing; bus
transaction contract; provider clock ratio; guest time axis; bounded L1
policy; retirement contract; and retirement qualification. CPU identity and
source timing remain unchanged.

The board retains PIC/PIT/DMA topology and personalities, six device clock
ratios, IRQ timing, auxiliary PIT placement, KBC protocol/electrical inputs
and XT PPI wiring. It still supplies the same values to existing constructors.
No host time creates guest ticks; no timing grade changes.

## Regression and similar-issue sweep

The existing RAM allocation/failure test now also checks each of seven
invalid clock ratios and both public null-input cases. All seven clock
failures must null the output without attempting RAM allocation. Existing
RAM/port failure rollback, plan, instruction timing and four-profile tests
remain registered.

The controller-authority gate rejects a board configuration/device clock
plan or composition constructor in neutral `machine.c`, and board-specific
fields in the neutral construction value. The historical constructor gate
explicitly classifies the RAM fixture's added public calls; its count rises
from 118 to 119 rather than weakening the check.

Production caller search finds exactly one caller of the neutral allocator,
in board composition. All thirteen neutral fields have explicit designated
assignments. Core still has board reset/NMI/finalization calls and public
board types; this S does not claim independent compilation or complete
physical extraction. S66-S69 retain those measured receiving boundaries.

## Verification

- Complete repository-only units: x64 **469/469**, x86 **469/469**.
- The local construction/clock/plan selection passes **6/6** per width.
- Both-width specialized target sets pass. The first run correctly rejected
  the newly unclassified public RAM-fixture calls; classification was added
  before both final target sets passed. Documentation governance and
  `git diff --check` pass.
- Four optimized PE x64 and four optimized PE x86 0540 products rebuilt;
  no `.debug` sections. Owner INIs and external assets have no Git change.
- Production: three paths, **100 additions / 56 removals**, net +44, for
  the typed construction boundary and one-time value binding. Test: one
  path, **34 additions / 0 removals**. Static gates: two paths,
  **28 additions / 6 removals**. There is no new executor/device framework.

Artifact SHA-256 identities, x64 then x86:

- Model 40: `D3947CD7FE45C551BEB3006311B25B15B0E6C95224575094541502CBCDD22F4C`, `D3DC5DE3A157A736307E44EB5EEAA41495D594D6EC449DEDA5347C6C5CB01CB2`.
- Default: `155CB53F6E621AB0B9369B181B085407F08ABE0871A42207155B57624FA0565A`, `6512CAC645178FF54D8A57923C41C978D976ADCB7DBB1558DB08CB9F92A28BDA`.
- XT: `F8389869C0020CC0447120C0F7BE14EBDC1594065C18135B8FB8EE9E0C270629`, `07AE2DD4DCD73DD64667370ABD82959CC76C43B46C0C60AB84A1D710B9EFDAE3`.
- IBM 5170: `A26E50DA49E31E2A9535914D3CC71C5FA4706EF9192FFF1B796FF0BEBC6D9EF9`, `A169337812CF6746D5B4DE4EBEC46CAD399144AE04331DEB8AE8068F5282A821`.

Exactly one external boot per profile/width passes, **8/8**: Default reaches
`dos-prompt`; XT, IBM 5170 and Model 40 reach `installer-running`.
All probes were relinked after the construction change; no older executable
substitutes for the current build. This proves the sampled checkpoints, not
indefinite freedom from intermittent faults or T-level integration completion.

Actual-P coordinator review follows the complete delivery commit. T540 stays
open for the public board interface and remaining extraction receivers.
