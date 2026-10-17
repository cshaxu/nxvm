# T1 Integration Freeze

This is the receiving design for the selected original V30 `raiden2` set,
not another current-state authority or an implemented Shared API. The owner
approved V30 profile, NEC instruction-space and timing-origin enum extensions
on 2026-10-17. Lib graphics changes remain explicitly unapproved High TODO.

## CPU And Execution Owner

Use the existing X86 Core CPU inside neutral X86 Core Machine. Its public
memory routes, interrupt acknowledgement, attachment deadlines and one guest
timeline fit the board's integration needs. The App owns board hardware and
composes it; it cannot borrow private CPU/Machine layouts. The existing direct
CPU constructor is not a reason to duplicate Core's runner/time owner.

Public additions are limited to necessary existing-enum values:

1. V30 profile appended after existing Intel values, without renumbering them.
2. NEC instruction space, separate from the Intel 0F map.
3. V30 timing provenance distinguishing source-backed costs from explicit L2
   models; neither may silently inherit an Intel fallback classification.

Keep existing function signatures and public/copied layouts unchanged. Use
architectural PSW/FLAGS MD state for native/8080 mode, not a duplicate App flag.
Any newly necessary field, function or callback needs another review before
implementation. Do not make this approval permission for unrelated refactoring.

Existing Intel profile ordering must not grant V30 protection/32-bit/cache/SMM
capabilities. Audit the entire 306-line candidate inventory from S1 against
its real semantic question. Retain Intel behavior and table style, replacing
only assumptions a NEC branch invalidates. Private capability helpers may
centralize these decisions; no public capability registry/facade is justified.

NEC execution, prefix parsing, instruction length/lexeme preview and timing
must use a single owned NEC form definition. Share verified ALU/addressing
mechanisms rather than another interpreter; preserve real differences in
FLAGS, shift/repeat, interrupts and bus-visible effects. Full V30 includes
documented 8080 mode. That mode is not a shortcut to the separate Z80 sound CPU.

Initial board configuration selects no FPU. That is a board fact, not a PC
default or permission to redefine every V30/coprocessor combination in Shared.
Unqualified external-coprocessor behavior cannot masquerade as a working FPU.

## Bus, Clock And Output Boundary

Board RAM/ROM and mapped devices use public memory routes. Board selects ROM
banks and owns COP/video/input/sound registers. Pending/acknowledge callbacks
deliver vector 0x30 in the fixed reference, not an injected Intel 8259.
Observational reads must not clear a latch or advance a device; admitted byte
and word accesses must preserve effects, alignment, mapping and errors.

Core elapsed time is the one clock axis. Board attachment deadlines and
advance callbacks schedule video, COP and sound work. Reference oscillator
ratios are exact arithmetic conversions but their physical evidence grade
must remain separate: V30 nominal 16 MHz, video 8 MHz, Z80/YM2151 crystal/8,
and OKI crystal/28 with the sound crystal nominally 28636363 Hz. Carry rational
phase across calls; do not round each instruction/frame independently.

V30 instruction/entry timing and retirement qualification must be added at
the CPU/Core owners before treating board time as qualified instruction time.
Measured/manual-backed facts and L2 prefetch/bus/board models remain distinct.
Do not change existing Intel timing grades or mark a nominal pace as verified
silicon timing. One Emulator worker drives bounded Core runs; no sound thread.

Board video produces complete native colour frames. Machine owns portrait
orientation/output conversion. Full-colour delivery cannot be promised through
the current 256-entry KVM graphics palette; the declined RGB proposal is TODO,
not a colour-quantization mandate. No alternative presenter is approved.

Core sound produces deterministic mixed signed-16 PCM for existing Lib PCM.
Sound CPU, FM synthesis, ADPCM, latches and timers remain guest hardware, not
host MIDI/FM/volume services. Respect Lib's accepted prefix on partial failure.

## Assets And Product Boundary

The reviewed 13-image content inventory is fixed; two background socket names
differ from the reference while their role/hash identity agrees. Product's
runtime manifest is code-owned when implemented. The evidence JSON is input
to the offline audit tool, not a runtime dependency or a second loader.

Runtime loading initially consumes individual external files through Lib
Storage. ZIP is an input packaging detail: a separate, explicit preparation
operation may unpack owner input after validation. S2's verifier does not
extract. Preparation must preserve the archive and validate destinations;
there is no requested Lib ZIP decoder. Game data never enters the EXE/repo.

Product supplies the lowercase INI, product identity, controls and Machine
composition. Emulator keeps canonical commands, framing, lifecycle and one
Session provider. Initial input policy uses Escape pause, guest movement,
fire/bomb, coin/start and service from the selected-board controls. No PC CAD,
BIOS, disk profile or X86 NXVM/IBMPC Product path is part of this board.

## Bounded CPU Implementation Batches

The CPU receiver admits its exact S scopes dynamically, using these complete
mechanism boundaries rather than one instruction per S:

1. Family/form admission audit and preservation tests for all existing Intel
   profiles, with V30 reset/PSW and decode contracts fixed.
2. Native V30 execution: shared compatible instructions, NEC extensions,
   prefixes, flag differences and matched execution/preview form definitions.
3. Mode/interrupt/control completeness: 8080 mode, interrupt entry/return,
   repeat restart, WAIT/HLT and declared bus behavior.
4. Source-graded timing and neutral Machine admission, maximum/partial run
   bounds and preview/retirement consistency, followed by full regressions.

Every delivered CPU batch keeps one production path and honest eligibility;
it must not register an incomplete profile as a playable product. Source/test
manifests, both host architectures, every affected consumer and current eight
PC artifacts are reviewed after Shared source changes. MyNES does not consume
X86 Core; its binary need not change merely because a CPU batch is delivered.

## Fixed Semantic Acceptance Checkpoints

| ID | Required observation | Owner / proof receiver |
| --- | --- | --- |
| G01 | All 13 required roles validate; wrong/missing/replaced data fails without publishing a machine | Product/Machine asset tests and owner-input integration |
| G02 | Correct reset fetch and initialization through real V30/bus operations | CPU and Board units; product headless integration |
| G03 | Actual COP/graphics initialization completes without bypass or fabricated results | Board/COP units and real-set initialization checkpoint |
| G04 | Attract/title progresses with correct layers, orientation and sound commands | Video/Sound units and real-set scene comparisons |
| G05 | Coin/start for both players produces guest state changes | Product input-to-board integration |
| G06 | Movement, fire and both bomb selections work, including input release/focus/pause | Board/control and host integration |
| G07 | Boss damage/defeat and stage transition occur through unmodified game execution | Real-set gameplay qualification |
| G08 | Every selected-game stage/boss transition and loop/end boundary is reconciled | Gameplay receiver freezes the stage-ID/input/signature matrix before its first coverage run |
| G09 | Death, game-over, continue and restart preserve credits/control/game state | Real-set gameplay qualification |
| G10 | Pause/resume, stop/start, reset/pause, Window close and teardown obey shared lifecycle | Machine/Product units and native integration |
| G11 | Palette/scroll/priority/raster and mixed PCM remain correct across named scenes | Video/Sound and full-colour release gate |
| G12 | x64/x86 full owned/receiving units, required integrations and latest pair identities pass | Final release receiver and owner gameplay acceptance |

These freeze what must be observed and who proves it, not the values of a
working emulator. Neither inspected operator manual nor flyer enumerates all
stage IDs or their RAM signatures. The gameplay receiver must freeze that
matrix against the unmodified selected game before its coverage-bearing run;
it cannot use elapsed seconds, one later screen or a smaller subset as success.
No proof of G02--G12 exists yet, and this design is not a gameplay qualification.
