# M5 T580 S1 — MyPC110 qualification ledger

## S49 integrated-controller receiver

[S49 proof](t580-s49-integrated-fdc.md) wires the existing FDC at standard
ports/IRQ6/DMA2 with the original SMC's fixed compatible status inputs, while
retaining no external dock drive or inserted medium. Both actual TFT BIOS
widths now assert/release DOR reset, return through the IRQ wait and match
all four SENSE reset results. The same integration retains earlier copy/SMI/
RSM/timer predicates. This closes that complete reset/IRQ receiver only;
extended Super-I/O configuration, dock hardware, lower Flash, Setup/boot,
selected storage/guest and live product qualification remain explicit members.

## S48 post-initialization batch reconciliation

[S48 observations](t580-s48-post-initialization-route.md) retain three actual
processor resets and subsequent working ASMI/vendor/RSM service on unchanged
firmware. Both widths remain before INT 19h. The BIOS resets a floppy
controller and waits for its IRQ-completion flag, but the App plan installs
no FDC. This is a missing controller/IRQ receiver, not permission to force a
firmware flag. A separate guest port-98 bit-2 toggle/copy/restore sequence
identifies the lower Flash receiver. FDC reset/IRQ/empty-drive wiring is the
next cohesive implementation batch; Flash/Setup/boot-sector/guest/live
product rows remain unqualified, not removed or counted as completed POST.

## Frozen product universe

The product target remains four build-fixed artifacts, not runtime-selected
models:

| Member | Current input/status | Owner/disposition | Proof required |
| --- | --- | --- | --- |
| TFT x64/x86 | 256 KiB BIOS and 1 MiB font exist under external `ibm-pc110-tft`; hashes match the admitted manifest. | MyPC110 App S2 composition/build receiver. | Construct/destroy and product lifecycle first; firmware progress separately. |
| Original DSTN x64/x86 | No verified original BIOS/font input exists in the external asset root. PC110-EMU's only ROM hashes to the admitted TFT image. | External BYOB/source gate. | Lawful original input, manifest identity, then the same App product route. |
| Product entry/binding | Missing: `src/app-mypc110` contains only board/firmware diagnostics. | MyPC110 App S2. | One normal `x86-nxvm/product` binding and lifecycle test. |
| INI/runtime identity | Missing: no `mypc110.ini` template, selected profile or App artifact declaration exists. | MyPC110 App S2. | App-selected adjacent INI path, without runtime model/ROM choice. |
| Real firmware progress | T579 proves the bounded port-61/LPT/RTC cycle only. | Later finite device receivers. | Named firmware checkpoint, no synthetic BIOS path. |
| Display/panel | F65535 foundation and font route exist; full panel/graphics presentation is unqualified. | Later display receiver. | Source-backed mode/panel capture and actual output proof. |
| Storage/guest corpus | No admitted PC110 floppy/HDD/PCMCIA boot route or lawful guest corpus. | Qualification receiver after hardware source gate. | Explicit medium and finite guest checkpoint. |
| Input/power | Keyboard/TrackPoint substrate exists; power/APM remains deliberately unmodeled. | Existing App route plus later power receiver. | Lifecycle/input proof without host power fabrication. |

## Asset reconciliation

`O:/repos.hobby/nxvm-assets/profiles-nxvm/ibm-pc110-tft` contains the admitted
TFT system BIOS (`232101c8…5232312`) and Kanji font
(`9829fdb8…71784c`).  `PC110-EMU/Roms/pc110_bios.bin` has the same BIOS hash.
S13 also checked every full firmware candidate in PC110-EMU, PC110-QEMU and
Open-Source-PC110. The latter repository's
`Mods/LBA-BIOS-Patch/PC110_BIOS_lba.BIN` is distinct
(`6c859ad2…2bec1d`) but its own README identifies it as an LBA-modified image
which has never been run. It is not original-DSTN input. Its TFT material
documents a forward DSTN-to-TFT patch and explicitly states that a confirmed
unpatched DSTN video-BIOS dump is still required for the reverse patch.
Accordingly, no discovered image supplies the original member. These are
research/asset facts only; no external bytes or third-party source enter this
repository.

## Current code reconciliation

`mypc110-board` currently constructs a 486SX, Core/X86-NXVM board plan,
firmware map, F65535 route, font route, RTC, 8042 input substrate and the
bounded LPT model.  It is not connected to `x86-nxvm/product` or an App
executable.  The four existing PC Apps demonstrate the intended direction:
each App supplies a fixed product name, adjacent INI name, machine binding and
build-selected firmware; `x86-nxvm/product` owns the common parser/lifecycle.

## S4 C000 checkpoint disposition

The TFT BIOS's selected C&T video image is now an immutable instruction source
for the App-local C000 route while ordinary data reads remain the writable
shadow required by copied BIOS tables.  The real image crosses the former
`C000:6000` width-sensitive port boundary on x64 and x86.  F65535's finite
input-status reads and the two traced Core port-width defects are accepted
receivers with regressions.  This is a progress checkpoint only: it does not
qualify POST, panel output, Easy-Setup, storage, guest boot or the unavailable
DSTN member.  S5 owns classification of the next bounded post-C000 trace.

## S6 post-C000 port-61 disposition

The subsequent real-TFT sample observes alternating port-61 bit 4 values after
the C000 return on both host widths. The existing elapsed-tick source therefore
remains the owner and no new PIT, F65535 or Core receiver is justified. The
firmware is still in its own delay body; the exact sample and its limits are in
[the S6 evidence](t580-s6-post-c000-port61-sample.md). This does not advance
any display, panel, Setup, storage or guest qualification row.

## S7 post-C000 delay-return disposition

The same post-C000 real-TFT path enters `F000:EA86` with `ECX=040Bh`, executes
the delay routine's `F000:EA92` return and resumes at its caller's
`F000:EA84` epilogue on both widths. The port-61 polling body is therefore a
completed BIOS delay, not a missing hardware receiver. The exact bounded trace
is in [the S7 evidence](t580-s7-post-c000-delay-return.md). This opens no
display, panel, Setup, storage or guest qualification row.

## S8 display-receiver disposition

Research now identifies the first display receiver without treating a
host-rendered front panel as machine behavior. The F65535's standard VGA
mode-12 (`640x480x16`) copied scanout is absent from the current component,
while the existing X86-NXVM snapshot/frame path can already carry the required
indexed pixels. S9 owns that bounded L2 receiver; front LCD/power, physical
panel timing, original-DSTN input, storage and guest rows remain separate. The
source grades and boundaries are in [the S8 evidence](t580-s8-pc110-display-receiver.md).

## S2 boundary

S2 may add the missing **TFT-only** App entry, binding, profile declaration,
INI template and focused composition/lifecycle tests.  It must use the one
existing X86-NXVM Product path; it may not publish a PC110 EXE, claim real BIOS
boot, add a model selector or manufacture the missing original-DSTN asset.

## S44 receiver reconciliation

The earlier inventory above records its admission baseline. At S44 the same
frozen members have these dispositions; no product requirement is removed.

| Member | Current proof and remaining receiver |
| --- | --- |
| TFT x64/x86 | Fixed App, embedded inputs, composition units and previews exist. Both widths pass the unchanged BIOS timer calibration; later POST and release qualification remain open. |
| Original DSTN x64/x86 | A separately labelled, panel-table-derived preview pair exists. It is not a recovered factory dump or independent DSTN guest/panel qualification. |
| Product entry/binding | Both fixed variants use the normal X86-NXVM Product/Emulator route. Full live lifecycle/UX and release acceptance remain required. |
| INI/runtime identity | One adjacent App-selected mypc110.ini exists and is preserved; it does not select ROM or panel at runtime. |
| Real firmware progress | S43 locates the PIT/refresh calibration failure; S44 repairs its App clock relationship and proves delta 1668 on both widths, within the BIOS range. Remaining POST/Setup/boot milestones remain with T580. |
| Display/panel | F65535/font/graphics mechanisms exist. Actual panel output and independent TFT/DSTN display qualification remain open. |
| Storage/guest corpus | App ATA wiring exists; boot media, real guest progression and required storage/PCMCIA coverage remain open. |
| Input/power | Existing keyboard/pointer construction remains. Live input, Fn/inking and power/APM coverage retain their receiver/source prerequisites. |

See [the S44 clock proof](t580-s44-pit-refresh-clock.md). A timer calibration
pass advances only that firmware substage, not the final product predicate.

## S47 receiver reconciliation

The cold-initialization member is still open. The approved write-width
prerequisite and App configuration protocol now have actual CPU route proof;
both host widths retain timer calibration success. Complete CPU-only SMRAM
decode, Board-to-App SMIACT wiring, initializer predicates and trigger/acknowledge
remain one receiver batch, not accepted subcomponents of an assumed boot.
The [S47 record](t580-s47-cold-initialization-contract.md) identifies the missing
contracts, source grades, exact test results and receiving artifact hashes.
All other frozen qualification rows retain their S44 dispositions above.

The later S47 decoder receiver replaces prefilled C/F and the unproven F9
hard gate with register-qualified shadow access. Both widths now execute the
actual initializer, handler copy and return; the existing timer integration
requires those markers and passes. This disposes the previously skipped
initializer predicate as repaired, not the complete cold-memory/SMRAM member:
CPU-only isolation, Board SMIACT forwarding, trigger/acknowledge and RSM are
still open. The boot-only lower Flash bank view also remains unqualified.
No other frozen product row changes disposition and no PC110 EXE is published.

The final S47 active-view continuation implements the remaining cold-receiver
members: one CPU-only SMRAM overlay, Board-to-App SMIACT, guest preparation,
independent configuration locks, source-qualified ASMI, W0C acknowledgement,
rearm and real RSM. Both widths execute the registered vendor service and
return before passing timer calibration with delta 166C. Core/chip/App units
and all 712 repository unit cases per width pass; receiving artifacts rebuild.
Shared implementation is `794380f69`; the App, evidence and PC receiving
artifacts are delivered by NXVM `782f09ebb`, with MyRaidenII `5ed407b4b`.
Coordinator actual-change review accepts this cold-receiver batch. The boot-only lower Flash
view and all later POST/Setup/guest/panel/product members remain unqualified;
none is hidden by accepting this one cold-initialization receiver.

## S49--S50 receiver reconciliation

The existing FDC/PIC/DMA receiver now completes the unchanged BIOS's DOR reset,
IRQ wait and four reset SENSE results on both widths; see
[S49](t580-s49-integrated-fdc.md). The independent lower-Flash selection now
has App-owned port-98/E/F decoding and direct existing-fixture proof of guest
copying, restoration, shadow independence and reset behavior; see
[S50](t580-s50-lower-flash-selection.md). Its actual BIOS Setup bulk-copy and
loader consumer is still unqualified. The bounded normal-route sample ends
inside an SMM checksum loop, with no INT 19h; that is neither boot success
nor proof of a deadlock. All remaining original source, Setup/boot, selected
guest, panel/input/power, live lifecycle and publication members retain their
T580 receiver. The coverage universe and completion predicate are unchanged.

## S51 cold-power handoff reconciliation

The [S51 diagnosis](t580-s51-cold-power-handoff.md) proves the actual cold FB
clear and later zero-valued suspend guard on both widths; x64 completes the
checksum and reaches firmware-requested suspend/HLT. This identifies the next
complete power/status/wake/retention contract, not a generic CPU HLT defect
or permission to force FB high. The hardware semantics of FB/B8/FF and mode
22h require source reconciliation before repair. Bounded L2 remains allowed;
the source/Setup/boot/guest/live-product and publication rows remain open.

## S52 RTC space and power-boundary reconciliation

The [S52 receiver](t580-s52-power-contract.md) repairs a separately reproduced
CMOS 3B/7B address alias. Existing RTC/Board construction selects the old
64-register or compatible 128-register space, with PC110 selecting 128.
Both widths prove actual guest IN/OUT isolation and pass the complete 712-unit
corpus, original timer/SMI/FDC integration and all four deployed INI identities.
The eight PC receiving EXEs are refreshed; four PC110 previews remain
unpublished. No other source, media, panel or product gate is advanced.

The two repaired real-BIOS observations are identical at the genuine suspend
endpoint after 32,019,249 instructions. RTC AIE is not enabled, so an already
armed automatic alarm cannot be assumed as the missing first wake. FB/B8/FF
status/write rules, the named physical event, retention/reset and real BIOS
restoration remain one power receiver inside T580, not a generic HLT change
or a demand for owner hardware. No fixed FB bit or synthetic wake is admitted.
The complete original qualification universe and completion predicate remain.

## S53 active power/EC reconciliation

The [active receiver evidence](t580-s53-power-receiver.md) distinguishes a
second, concrete defect from missing power wake: the existing EC mailbox had
no response implementation for the BIOS's defined queries. Seven transactions
exhausted their finite guest response waits. Original controller command tables
and initialization now define an App-only reply/RAM receiver, with focused
both-width units passing and a normal x64 BIOS consumer observation. It still
reaches genuine suspend; the physical wake/status/retention/restoration batch
and all original product predicates remain open. This is not S53 acceptance.
The reference's stale missing-reset-vector claim is explicitly superseded by
its later corrected map and the supplied images' actual reset vectors; it
does not form an owner-asset prerequisite for this receiver.

The implemented physical receiver now passes actual F12 Machine-driver and
unchanged-BIOS retained-reset/restoration/RSM replay on both widths, including
all 512 retained CPU bytes and no extra exception. This proves the complete
bounded power transaction, not a live keyboard, guest or product release.
S53 is accepted after receiving artifacts and coordinator actual-change review.
All 712 units per width now have passing results, including serial rechecks
of the unchanged static cases that timed out under concurrent load. Original
failed attempts remain recorded; no Lib change or budget relaxation is made.
The original T-level universe and product predicates remain unchanged.

Shared `85a65c3a8` and NXVM `a8561555e` are pushed. The registered retained-
power test passes on both widths, all eight PC artifacts and INI identities
are verified, and all 28 scratch files are removed. This accepts the bounded
EC/power receiver, not the original live-input/guest/product gates.

## S54 ordinary POST and real bootstrap disposition

The complete post-restoration route batch is reconciled in
[S54 evidence](t580-s54-post-restoration-route.md). A generic CPU defect,
unconditional SX #NM for EM/TS-clear ESC, prevented the BIOS's no-FPU
probe. The corrected explicit L2 absence model and existing L2 timing owner
are independently tested across opcode, operand, control and protection
contexts. No FPU, ROM patch, intercepted BIOS PC, second executor or guest
clock is supplied.

Both host widths now execute retained wake, the same 512 CPU-save bytes,
actual RSM, ordinary POST and the original CD19 at F000:52BD, entering
F000:E6F2 after 56,679,591 instructions and four processor resets, with
the same four delivered exceptions. The expanded full route takes 302.96 s
x86 / 224.16 s x64; its justified 360-second budget and earlier honest
timeouts are documented separately from functional success. Existing FDC,
timer and ASMI regressions also pass both widths. All 712 distinct units
per width have passing evidence, including the corrected legacy executor
assertion after its recorded initial failure.

This accepts an ordinary firmware bootstrap handoff, not DOS boot or product
publication. The untouched original universe still requires actual selected
storage/guest checkpoints, real Setup selection/loading/UI, panel/live input,
model identity, lifecycle/desktop and final four-artifact qualification.
Those members remain explicitly unaccepted at the existing App/product
owners; the finite S54 disposition does not transfer or erase them.
