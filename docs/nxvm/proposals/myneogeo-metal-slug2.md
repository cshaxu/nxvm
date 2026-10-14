# MyNeoGeo: Metal Slug 2

## Goal And Boundary

Implement an independent MyNeoGeo App, with running the real Metal Slug 2 ROM
set as its first required compatibility goal. The owner placed this candidate
after MyArcade at the NXVM queue tail. It has no implementation T number yet.
This is Neo Geo MVS, not x86 or an IBM PC; Metal Slug X and Neo Geo CD are not
substitutes for the requested game. Admission fixes the exact MVS revision,
region, BIOS and cartridge set against lawful owner inputs.

## Architecture

- Proposed parallel roots are `src/app-myneogeo`, `test/app-myneogeo` and
  `assets/myneogeo`. Before coding, admit documentation/tools/version/commit
  ownership for the new App; this proposal remains in NXVM's planning sequence.
- Reuse Lib and Emulator Product/Session/UI/Machine contracts for host services,
  monitor, lifecycle, frame/audio delivery and input. App Product supplies
  identity, configuration, controls and cartridge policy; App Machine adapts
  the Neo Geo board to the existing copied contracts and sole control flow.
- Board implementation owns the 68000 main CPU, Z80 sound CPU, video, sound,
  cartridge/BIOS mappings, input, interrupts and one coherent guest timeline.
  Audit existing capabilities first; neither NXVM's Intel CPU nor PC board
  code supplies these semantics. Do not depend on shared x86 Product/Debug.
- MyArcade and MyNeoGeo may reuse a proven Z80 mechanism if their actual
  contracts support it, with a separately reviewed owner/dependency decision.
  Do not duplicate host loops or prebuild a universal arcade device framework.
- Fixed monitor syntax and completion text stay in Emulator. Machine-specific
  Debug or snapshot support is implemented honestly or reports the existing
  unsupported-feature result; do not introduce a second monitor parser.

## Sources And ROM Handling

Initial research references checked on 2026-10-13 are the
[MAME Neo Geo driver](https://github.com/mamedev/mame/blob/master/src/mame/snk/neogeo.cpp)
and [device declarations](https://github.com/mamedev/mame/blob/master/src/mame/snk/neogeo.h).
They are reference leads, not imported code or hardware certification. Freeze
reference revisions and original manuals/observations at admission. Separate
reference-based L2 models from measured/manual-backed claims, and review any
third-party CPU/device implementation's license before copying or derivation.

Neo Geo BIOS, game program/graphics/sound and other required payloads stay in
lawful owner-managed external assets. Inventory roles, lengths and hashes;
reject incomplete or incompatible sets explicitly. No download, ROM patch,
protected payload commit or embedding into published EXEs is authorized. The
four-PC firmware exception does not extend to this game or BIOS.

## Planned Batches And Coverage

These batches do not allocate S identifiers:

1. Freeze board/BIOS/game inventory, source/license choices, component gaps,
   App governance and one finite convergence ledger.
2. Build the native Product/Machine composition and validated asset loading,
   with ROM-free independent tests and unchanged existing Apps.
3. Implement/qualify 68000 and Z80 execution, mapped memory/banking, interrupts
   and timing; then video, sound and controls in bounded component batches.
4. Prove BIOS boot, attract, coin/start and Metal Slug 2 gameplay across stage,
   boss, completion, game-over and restart checkpoints.
5. Qualify lifecycle, failure/cleanup, dual host architectures and release
   artifacts, then obtain owner gameplay acceptance.

The ledger must include each selected game's controls, display geometry/
orientation/sprite behavior, sound, CPU/device timing, BIOS/cartridge behavior,
named gameplay checkpoints and host lifecycle. Define exact inputs, observations,
budgets and test owners before implementation; collect the full affected batch
rather than declaring completion after reaching one later screen.

ROM-free units and explicit owner-ROM integrations run on x64/x86 in reusable
Ninja/ccache graphs. Preserve authentic game slowdown; measure host efficiency
separately. Validate audio without relying on physical volume. Run affected
consumer regressions and types/dependency/manifest checks, publish the current
dual EXEs and review actual diffs and provenance. No all-games or cycle-exact
claim follows from this single-game acceptance.

Missing legal assets, CPU/device evidence, required Shared changes or weakened
gameplay scope requires review. A build, title screen, silent sound stub or
scripted scene cannot close the task. Implementation is not admitted here.
