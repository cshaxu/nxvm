# T580 S53 - Complete power transaction receiver

Shared implementation is pushed as `85a65c3a8`. The subsequent NXVM S53
implementation commit identifies the App source, evidence and eight receiving
artifacts built from this same frozen candidate. Governance acceptance follows
actual-change review; neither implementation commit closes T580.

## Production input receiver continuation

The subsequent driver connection uses the existing copied Machine profile
binding and its transferred context lifetime. One optional `handle_key`
operation runs on the owning input executor. A consumed event does not also
enter the normal guest keyboard mapper; an unconsumed event follows that
unchanged mapper. Other profiles leave the operation null. No App name, key
choice, second dispatcher, queue, event or thread enters Shared Machine.

PC110 alone binds F12 make/break to its virtual resume-button level. It is an
emulator preview binding, not a claim about the physical PC110 key map or a
way to resume a monitor-paused Machine. Physical-operation errors enter the
existing Core fault path. Lib/Emulator and their input admission contracts
remain unchanged. UI documents the reserved key and preview boundary.

The existing Shared construction fixture verifies copied binding ownership,
make/break consumption and ordinary-key fallback selection. The existing App
construction fixture enters through the real described Machine driver and
checks refresh changes after the F12 pair. Both fixtures pass on x64 and x86
(two cases per width). The first build rejected a test-only `BASE_KEY_A`;
printable keys already use ASCII under Lib Base, so the fixture now uses
`'A'`, with no new alias or Lib change.

Source/test X86 Core and X86 NXVM manifest checks, both dependency gates,
documentation governance and `git diff --check` pass. These static results
and fixture passes are not the actual BIOS replay or full qualification.
The same unchanged-firmware power mode now delivers F12 through the actual
driver instead of invoking the App pin operation directly; its two-width
replay subsequently passes on both widths as recorded below. Complete
repository unit coverage is reconciled below, including failed attempts.
Because the family Machine has changed, eight published PC artifacts now
receive this batch and must be rebuilt before its delivery. MyNES and
MyRaidenII do not link that Machine, and PC110 previews remain unpublished.

### Actual driver replay and qualification still outstanding

The actual neutral F12 pair now enters through the described production
Machine driver. Both native bounded runs terminate successfully. Each proves
stable no-event suspension at 39333h with zero additional instructions,
restoration entry at 39336h, the EOSMI call at 3DBC5h, identical retained
512-byte save areas, and actual RSM returning to F000:DD11 with flags 0046h.
Both execute 48,828,526 instructions and four processor resets. The delivered
exception count remains the four known cold-POST probes; no new fault occurs.
Containment remains 60 million instructions and 300 native seconds, never
the success predicate. The combined driver log SHA-256 is
`B7D8B8B0A1F4DFBB4A7B4E6E50CC92D2F2104034846E924D21D22E0E6EB80F8C`.
Its TIMER fields are unsampled in this mode, not timer regression results.
The original FDC/timer/ASMI integration independently passes on x64 (46.73 s)
and x86 (61.51 s); that log SHA-256 is
`A4FF7AAAC1390449943758434C809901D11840294F182EC2B239DA6073C8066A`.

The same existing executable now registers the distinct retained-power case
as `mypc110.integration/real-firmware-power-resume`, with the already admitted
300-second containment. No second harness or executor is created. This is
driver/firmware proof, not physical keyboard, desktop/RDP, full guest boot or
final PC110 publication qualification.

The first complete x64 unit run executes all 712 cases: 710 pass; unchanged
`lib.types-layout-selftest` times out at 180 seconds and unchanged
`x86-ibmpc.corpus-negative` at 30 seconds. A serial selection during the x86
build repeats the Types timeout but passes the IBM-PC negative test in
9.01 seconds. These attempts are not full-suite passes. The x86 complete run
passes 711/712, with only Types timing out. After every build, native replay
and other CTest controller exits, the same Types case passes serially in
104.35 seconds on x64 and 146.75 seconds on x86, within the unchanged
180-second budget. All 712 distinct units per width therefore have a passing
result on the frozen candidate; neither initial parallel invocation is
misrepresented as wholly passing. No Lib source, test, timeout budget or
assertion is changed. The observed load sensitivity requires scheduling this
expensive process-spawning selftest without concurrent build/BIOS work, not
an inferred production repair or another full-suite repeat.

Actual C/H delta against S52 at this review point, using `git diff --numstat`
and excluding CMake, manifests, docs and generated artifacts: production
+205/-28 (net +177), tests +335/-15 (net +320). Of the tests, +118/-3 extends
the one external-BIOS harness for the distinct retained-power contract; the
remaining +217/-12 extends existing owner fixtures. New runtime ownership is
one EC RAM/reply protocol and two distinct Chip facts (physical level and
pending resume event), not a generic state machine. The optional Machine
profile callback is copied once and borrowed only during serialized input.
All four old App construction paths use zero/designated initialization, so
their callback stays null and preserves normal keyboard mapping. The similar
path sweep finds no second input dispatcher or App-specific Shared branch.

### Receiving artifacts

Both stable Ninja graphs select each fixed App in turn and return to the
default profile. The eight Release/stripped products retain version 0.5.0580;
each passes the build's PE-width check and its real adjacent-INI identity
integration. No shared target is copied to a task-specific build tree. The
artifact-build log SHA-256 is
`63A5AAE3546BFF88D6848A11C23228503E99E999565D531D10656BAD610872B5`.

| Asset path | PE machine | SHA-256 |
| --- | --- | --- |
| `assets/my5160/my5160_0_5_0580_x64.exe` | 8664 | `6B98E4AC4DB60C50042E4800504D2C41E7DE8FDABE87724B4AD66FB3D3CDB54F` |
| `assets/my5160/my5160_0_5_0580_x86.exe` | 014C | `124961A784A55600B83FC39C3D6C8B3A76AF895147A9FBC6A85B34A77037355A` |
| `assets/my5170/my5170_0_5_0580_x64.exe` | 8664 | `5BF396D1A164182318715C606C9DB52620B90D271D15F0CB56CFCCAF3D8EEBB0` |
| `assets/my5170/my5170_0_5_0580_x86.exe` | 014C | `12C87C646F59CAD7A4D17FB36A19880690C88535C5AABCCDFB6AD2112ADF894A` |
| `assets/mydeskpro386/mydeskpro386_0_5_0580_x64.exe` | 8664 | `10ED8F2A61B99205804E703FD04868AC9E0438A73329DBF25678A71A85D3BA88` |
| `assets/mydeskpro386/mydeskpro386_0_5_0580_x86.exe` | 014C | `B6F1ACBD0E2B8AA5D2F34ECA60297CF9AB5B65C55B7B1D82FF873CADF853A5F0` |
| `assets/nxvm/nxvm_0_5_0580_x64.exe` | 8664 | `C3EE98E7A59F924509E572966FDAFED797AB66D2CB3BDFC7141E41CE8000D322` |
| `assets/nxvm/nxvm_0_5_0580_x86.exe` | 014C | `073748C674DC9544107023EBC6BC3BD9BDF32F2115C3743DAA456FA03234B0F0` |

The four explicitly rebuilt, unpublished previews are:

| Width/panel | SHA-256 |
| --- | --- |
| x64/TFT | `4F0173676D7BA5521A62F579D327646DAC87C6CE81CA001815C3185CA9590B3E` |
| x64/DSTN | `630C1F6CA1A3CC4AAAE775DE19D6DFFB7D4359B548523A4547A278EC2C3BE465` |
| x86/TFT | `80744601B2E72B5A53F1F4D27D0C6D8BB39DDF1B77D4DEFCEF7A6972F1DFB7FA` |
| x86/DSTN | `5792B638F864814866CB907994F7CE8CBD0155E23A1C72A608734E86C4FCE7A5` |

MyNES/MyRaidenII source, tests and deployed artifacts remain unchanged.
All four PC INIs retain their S52 hashes: My5160
`C960F4085633999360E45C0EDBD47EAA787294745DEE24CE5EB7110125224924`,
My5170 `CEF6300135AA38E410E50E1EFD536940323A63ECB7441B40B879D9635B7A460A`,
DeskPro `C812A0C99D258C19CA8BB4BED584617DF2F4D25F17372583EB58BBD71D378599`,
NXVM `FEEFDF82F40A8C7C1CD96C130BBEC4B5C41E099E194FA955AD42B99F35E8FC4F`.
Protected ROMs/media are not modified or copied into tracked source.

### Whole receiver disposition and actual-change review

| Member | Candidate proof / remaining boundary |
| --- | --- |
| Cold status and mode 22h | Retain the S47 width/unlock and S52 source reconciliation. Cold/held/released button states do not invent a wake; 0083h does not enable automatic wake. |
| EC replies and retention | One App mailbox owns supported replies/RAM. Actual BIOS consumes them on both widths; unknown GPIO/charging effects are not fabricated. |
| Suspend and no event | Real FF=82h stops the existing CPU clock input; a second bounded run retires no instruction and does not wake spontaneously. |
| Explicit wake | PC110's F12 pair uses the actual copied Machine profile/driver, one Chip button edge and existing processor RESET, not monitor resume or a firmware PC branch. |
| Retention/restoration | Both actual BIOS runs restore the same 512-byte CPU save image and execute RSM without an added exception. Full machine reset reuses construction's cold image. |
| Original product universe | Setup/boot, selected guest/storage, front panel/live keyboard/RDP, lifecycle and publication retain their original ledger gates; none is accepted by this bounded transaction. |

Executor actual-diff review reads the changed Chip, App EC/config/reset/plan,
Machine binding/dispatch, owner fixtures, existing external harness, CMake,
manifests, docs and deployed artifact identities. The retained path is the
sole Core executor and bus dispatcher, existing Machine input delivery and
profile lifetime, and firmware-owned SMM/RSM. The removed partial cold-reset
loop is replaced by the existing common initializer, not another reset path.
CPU reset already clears CPU SMI/clock latch state; the Chip alone retains
the physical level and resume event until its own documented acknowledgement.
No App name, BIOS address/hash, fixed read count, host delay or artificial
completion selects production execution. Firmware addresses in the external
test are observations only. Existing diagnostic access is not expanded into
a new public observation API.

Lib, Emulator, X86 Debug and X86 IBM-PC source/test diffs are empty. Existing
normal keyboard, ASMI, lane-width and reset owners remain; no hidden warning
suppression, compatibility path, queue, thread or separate clock is added.
Three small mailbox test helpers express repeated read/reply/page transactions
inside the existing fixture; they do not introduce production or test framework
owners. The positive delta implements missing receiver behavior and its real
consumer regression, not formatting compression or duplicated generic shells.
Coordinator acceptance and complete-P delivery remain pending at this point.

### Final verification and scratch disposition

The registered retained-power integration also passes once per width:
x64 186.98 seconds; x86 248.64 seconds, both within the original 300-second
budget. The exact unchanged BIOS/driver command is now executed by CTest,
not inferred from compilation or a manual diagnostic. Its log SHA-256 is
`A7536DCE097666475DBAD8D9E58F91452E1613F7EA5EE457082EBA808258B604`.
Four source/test manifests, both dependency gates, documentation governance
and whitespace checks pass. All affected code builds with its existing strict
warnings; no new suppression is introduced or whole-tree warning-free claim made.

| Qualification | x64 | x86 |
| --- | --- | --- |
| Repository units | 712 distinct passing cases, including documented serial corrections | 712 distinct passing cases, including documented serial correction |
| Original FDC/timer/ASMI/RSM integration | Pass | Pass |
| Registered retained-power integration | Pass | Pass |
| Four deployed PC INI identities and PE widths | 4/4 | 4/4 |
| Two unpublished PC110 preview builds | Built | Built |
| Desktop/RDP/live physical input, full PC110 guest and other Apps' whole boot suites | Not executed/claimed by this batch | Not executed/claimed by this batch |

Additional final log hashes preserve the failed and successful attempts:

| Log | SHA-256 |
| --- | --- |
| `final-unit-qualification.log` | `4207303FB1EE9420254BFD0936BB8B439648189546BCABF0A102D418FBFC3428` |
| `final-unit-x86.log` | `F510A5E0B54C379FB74C4CEEE32BC1ACD1387A530F7F660C9F1C49EE2D5B60CD` |
| `x64-static-timeout-check.log` | `E839087CBDF6D47219E48B077DA8459F741310BB80F786CF72E0B31D209BE5EA` |
| `types-isolated-qualification.log` | `D6C6BD11E09A47BE834BA003BE086D44B5F9A34D01071D246275E498BCC22808` |
| `input-profile-checks.log` | `F6385DF7399FED71D71142CCFF5F369F4E93D2B67066BD9E11093C36AA2BD77B` |
| `input-profile-checks-corrected.log` | `45EF40219F252B6B81A0CEE1A115049559C0D74A34EE0ADC1DC46C3C5AE67BF8` |

All owned controllers and native CMake/CTest/Ninja/BIOS processes are terminal.
The official cleanup preview identifies exactly 28 S53-owned files totaling
3,011,103 bytes, and reports Deleted=False. They are logs/extracted research
text only; no Git checkout, owner configuration, ROM or disk is present. The
facts/hashes above and in preceding phase records precede Apply. Cleanup
retains both stable Ninja graphs and the ccache; it neither moves a graph nor
touches global caches. Coordinator acceptance remains a separate review after
the complete target-separated implementation delivery.
Apply subsequently removes exactly those 28 files/3,011,103 bytes; the final
literal path check is False. No task scratch remains.

## Admission and unchanged boundary

S53 consumes the existing power/status/wake/retention batch after clean,
pushed `d206cddfa`. It is not a new boot-success definition. S52's RTC repair
and complete regression qualification remain accepted; PC110 publication,
Setup/boot, guest, panel, input and lifecycle qualifications remain open.

Production is not changed by the first observation phase. In particular no
FB value, resume cause, interrupt, CPU reset or firmware byte is injected.
The existing actual-firmware integration harness follows the normal
`mypc110_machine_construction_create` / Machine / Core path. Temporary
retirement-observer fields track the indexed Block-2 and CMOS addresses
separately. Only copied instruction/I/O observations are read.

## Whole cold-to-suspend observation

The x64 run terminates normally inside its 180-second, 40-million-retirement
containment budgets. It executes 32,019,249 instructions, three processor
resets and the four previously recorded delivered GP probes. Timer delta
is 166Ch; initialization, ASMI handling and RSM markers remain true.
The final result is waiting-for-interrupt at 39334h, after the actual guest
`FF=82h; HLT`. This is a diagnostic endpoint, not a successful guest boot.

The bank-qualified transaction inventory confirms:

- Initial BIOS programming writes Block-2 B8=80h and FF=00h.
- Cold initialization writes B8=00h and FB=00h through its own helper.
- Later three ordinary processor resets do not produce a power-wake cause.
- At sequence 27,207,454 the actual FB read returns 00h.
- Before suspend, the BIOS writes CMOS 0F=0Eh, clears FB/B8, unlocks the
  existing CPU configuration protocol, selects mode 0083h and writes FF=82h.
- No software FB write setting bit 3 is observed in this complete prefix.
  Treating an ordinary processor reset as a wake, making FB permanently high
  or extending a static live dump into a power-on truth table is unjustified.

## Newly established EC response gap

The same run records 462,892 status reads and 11 data reads from the EC
mailbox. Every observed status byte is zero. Seven command transactions
contain at least one full 65,536-read response wait; the actual helper starts
CX at zero and uses LOOP while response-ready bit 0 is clear. This is not a
host wait or a CPU deadlock. It is a finite failed guest response protocol.

| Command | Data written | Status reads before next command |
| --- | --- | ---: |
| 0A | 9B | 4 |
| 05 | none | 65,538 |
| 23 | FF | 4 |
| D0 | D2 | 65,540 |
| 22 | F0 | 4 |
| 21 | none | 69,634 |
| D0 | D2 | 65,540 |
| D0 | D2 | 65,540 |
| D0 | D2 | 65,540 |
| FC | none | 65,538 |
| D4 | 88, 00 | 6 |
| 10 | none | 1 |

The App's existing mailbox implements only write-side latching and an
always-ready-to-write status. Reading its data port is explicitly unsupported;
its existing unit tests verify that limited behavior, not a command reply.
The earlier ULTRACHG source establishes write arbitration for command 31 and
the Zn text sequence; it does not establish reply bytes for the transactions
above. The new observation therefore expands the complete power receiver's
required EC protocol reconciliation. It does not prove that synthesizing any
particular reply will fix suspend, and no guessed reply is implemented.

The smallest valid receiver must keep one App mailbox owner, consume commands
and data there, and derive OBF/IBF from an actual supported transaction. It
must not return fabricated success for every query or add read-count-based
settling. Power status and its physical wake source still belong to the
chip/board receiver, not this mailbox's generic ready bit.

## Source reconciliation

Read-only pinned Open-Source-PC110 material additionally identifies the U67
keyboard MCU's two data-bus-buffer channels. Its Pluto notes distinguish that
MCU from U6's power-sense link and explicitly leave serial/telemetry attribution
uncertain. The supplied `kbc_disasm.txt` is generated with a minimal standard
6502 decoder; its `.byte` handling does not decode M740 bit branches or LDM.
It is not sufficient to import a claimed M740 command-handler meaning.

The already admitted external keyboard-MCU image is 16,255 bytes, SHA-256
`B29761A2FD39ABD9C9419CA73B03BEB6C41BC52102E2A7429D1F023F82A2A2B8`.
The external file named `13-M38813.PDF` actually identifies the 3802 group,
not an M38813 device-local DBB specification. Its SHA-256 is
`970F66D290992B66B1F58ADB1A1C92E5AC1600257BCC36D7A90DEB74F10BF092`.
Its 740-family instruction/vector facts are useful; the selected controller
image, not that mislabeled filename, supplies the host and RAM contract.
ULTRACHG analysis SHA-256 is
`3A4E3AC965776B38E41B01715C4F1B7877450E9037E6AB47A9B1EEAC256D83BB`.
These are research inputs, not copied production code or new ROM acquisitions.
MAME's BSD-3-Clause M740 opcode description was read only to distinguish
M740 instructions from ordinary 6502; no implementation/table is imported.

## Verification and current continuation

The first temporary harness compile rejected the misspelled `lib_printf`;
it was corrected to the existing Types `lib_c_printf`, without weakening
warnings. The successful native x64 diagnostic is not a new unit/full-suite,
desktop or RDP qualification. The x86 contrasting run terminated at its
180-second native containment deadline after recording the same query
transactions through sequence 27,208,774. Its partial output corroborates
the earlier response waits but does not prove the complete suspend endpoint
on this invocation. It is a timeout, not a passing test; no automatic retry
or increased timeout is used to turn it into one. S52's separate complete
both-width endpoint proof remains historical evidence, not a new S53 pass.
No Shared source, product artifact, owner INI or external firmware is changed.

S53 remains active. Its next receiver work reconciles the complete observed
EC command/reply set and the independent physical power wake/restoration path;
the finite waits above are not a reason to weaken either proof obligation.

Initial raw x64 transaction log: 82,038,761 bytes, SHA-256
`775637347911611431F827D871F890F86CBCA9565931AD975F127AB8539287EA`.
Its command summary SHA-256 is
`18C35A470521443650E0845D1FD2B08F6F265BB0F93B85CB64C2605EE09EA104`;
successful trace build log SHA-256 is
`2A49766CAECB71E4B8D932D98B403C91F3D41E58EE507AC694851204B5A7B7C6`.
The partial x86 log SHA-256 is
`E037906753C202E073B8EA112443E6D370935AE4140486E32284A38F7FA4A36B`.
Both owned native processes are terminal. Temporary observer changes are
removed from the tracked harness and both standard harness targets rebuild
successfully in the original cached trees; no new diagnostic mode remains.
Raw observations are task-local, not committed firmware data. Repeated status
reads are summarized for the x86 run to avoid repeating the 82 MB text output.

## Observation-phase cleanup

After the two native processes and the restored-target builds are terminal,
the official cleaner preview identifies exactly ten S53-owned generated files,
100,092,006 bytes, below `build/t580-s53-power`. They contain only this phase's
logs, extracted manual text and the project-authored summary script; no owner
configuration, media, Git data or still-used process output is included.
The facts and hashes above are retained before the previewed files are removed.
The x64/x86 standard harnesses are restored, Ninja/ccache remain reusable,
and this phase does not refresh any product EXE.

| Other phase output | Bytes | SHA-256 |
| --- | ---: | --- |
| 386sl.txt | 1,035,069 | D478845DE728BB574F7C70476383776D630CF4EBCC93FAADDEF2C9D301C9626D |
| m38813.txt | 253,344 | 8B4A0AABECAAD2A17274204EF570038671A170B2079FFD5002ABAA82A515D1AB |
| restored-builds.log | 466 | 844612D02D325ECE1AF2010685AE606517705B426DDA6BB546FD7C44B3E8EF32 |
| sl-architecture.txt | 1,079,061 | F6B420837568C7AD3DBE45078E12FD4B34F83E81EEC50A62E07E6625537BD355 |
| summarize-power.ps1 | 1,718 | FC0E10FBABA08273873DEE21A9B803B936A0695063CBAA9F31F9B36046FDE615 |

Documentation governance and `git diff --check` pass for this phase. These
are structural/whitespace checks, not implementation or product acceptance.
No implementation P is submitted from this incomplete receiver phase.

## Controller command contract reconciliation

The next source inspection resolves the apparent missing command grammar.
The reference `kbc_disasm.txt` assumes image base C081h. That places absolute
subroutine targets and data tables two bytes before their actual contents.
Using C07Fh for the supplied payload instead reconciles four independent
internal anchors: the E49Eh initialization prologue; the E496h eight-bit mask
table; the E472h indirect-word lookup helper; and the D874h command dispatcher
using the D898h group table. This is an address reconstruction for static
research, not a repaired or executed MCU image. No image byte is changed.
The reference's ordinary-6502 decoder also misses M740 LDM and bit operations.

A separate false source barrier is also resolved. The pinned power-MCU
research README's section 2.2 says the reset-vector tail is absent, but its
later section 7.4 explicitly corrects its own old C000h base to C080h and
withdraws that conclusion. The actual 16,254-byte image maps C080h--FFFDh;
its last two bytes contain reset vector E6B1h. That target begins the expected
initialization prologue and jumps into the original initializer. The keyboard
candidate likewise contains its E49Eh reset vector at FFFCh/FFFDh under the
reconstructed C07Fh base. Missing reset vectors are not a valid reason to block
functional receiver work. This does not certify every physical MCU variant,
the omitted reserved/vector locations or full MCU execution. Historical
research retains its original text; this reconciled evidence supersedes the
old truncated-reset-vector claim for the current receiver.

Following the actual command-group tables and the initialization records yields
the following neutral functional contract, graded L2. These facts do not import
the firmware instruction sequences or a reference emulator implementation.

| Host command | Defined operation | Controller handler anchor |
| --- | --- | --- |
| 05 | Return identifier 21h through the second host buffer. | D9D2 |
| 21 | Return current input status OR F0h. | E061 |
| 22 | Consume one byte; replace the low four mask bits with its complement. | E06C |
| 23 | Consume one byte; replace mask bits 1--7 with the complemented byte shifted left. | E07A |
| 0A | Consume and retain the controller-control byte. GPIO effects are not inferred from that storage. | DA3C |
| D0--D2 | Consume an address byte and read page 0, 1 or 2. | E152/E157/E15C |
| D4/D5/D6 | Consume address and value; selected-image D4/D5 use page 1, D6 uses page 2. | E16D/E172/E177 |
| FC | Return controller location 40h. | E18A |

The 40h--23Fh RAM window's cold initialization includes 40h=30h, D1h=10h,
FFh=FFh, with the observed input-status and D2h locations zero. The complete
selected RAM initialization is represented once in the App receiver. The
second-page buffers include 100h--112h=FFh, 13Fh--1F2h=EAh and the later
144h=14h override. The model does not pretend that unimplemented SFR locations
below 40h, or addresses above 23Fh, are RAM.
Unimplemented command forms do not synthesize reply data. The previous
write-only 31h traffic remains a latch, not a claim of completed charging.
The 10h input/GPIO/timer effects and unsolicited controller events remain
unqualified; this batch does not turn them into a power-wake source.

The resulting receiver uses one command/parameter parser, one retained RAM
store and one reply latch. Reply availability owns status bit 0; reading the
data consumes that availability, whereas polling status does not. There is
no arbitrary busy duration, read-count settling, second queue or MCU worker.
Other status bits remain outside this bounded host-reply contract rather
than being guessed from a post-POST capture. This is not full MCU emulation.

Initial focused x64/x86 builds pass strict existing warnings and the expanded
existing mailbox unit test passes once per width. The test exercises the
actual installed Core port route, all three direct queries, cold/retained
RAM, read/write transactions, masks/control, interrupted writes, CPU-reset
retention, unknown commands and the unimplemented SFR boundary. A real-BIOS
consumer replay is required before accepting this as a repaired firmware
transaction. S53's complete power receiver remains open regardless of that
replay's next endpoint.

### Pre-acceptance correction: D4 page alias

The first draft incorrectly grouped D4 with zero-page writes. Byte inspection
finds D4's entry selects page 0 but contains BRK and its padding byte where an
ordinary branch would have bypassed D5. The supplied image's BRK vector points
to the single RTI at E62Ch, returning into D5's page-1 selection. The functional
compatibility model follows that selected image's outcome. It does not patch
the byte into the apparently intended BRA, claim a documented silicon D4 alias
or execute MCU firmware in the host. One 512-byte 40h--23Fh RAM store replaces
the erroneous first-page-only draft. Tests prove page isolation, D4/D5, D6,
initialization and both address bounds. This is explicitly L2.

The first x64 consumer replay, before this correction, reaches the old suspend
endpoint after 27,893,750 instructions with timer/ASMI/RSM markers retained.
Its x86 counterpart exceeds the 180-second native budget and has no complete
endpoint output. Neither result is acceptance of the corrected RAM model.
The subsequent diagnostic removes unnecessary per-instruction snapshots and
records only actual reply creation/consumption at the sole App mailbox.
Both widths compile and pass the corrected existing mailbox fixture. The
first light replay returns before the usual endpoint print and destruction;
its process exits but it is rejected as complete endpoint/cleanup evidence.
The corrected light replay restores that path and checks the named genuine
suspend endpoint, not an arbitrary wait. The finite budget is revised to
300 seconds for host-throughput headroom; earlier timeouts remain failures,
and no original product, timer or integration assertion is weakened.

## Corrected receiver observation and regression

### Next receiver contract: suspend refresh and explicit button edge

The original SL architecture reference (SHA-256 recorded by S52), printed
page 138, Listing 10.1, independently specifies the observed sequence:
select indexed FFh, open the internal bus unit, write 82h to 25h and
immediately halt. This is suspend-refresh enable, not an arbitrary software
reset command. Printed page 216 identifies CPUPWRMODE bit 1 as unit enable
and bits 2--3 as unit selection. Bit 7 enables the PI-bus flash interface
(printed page 188); it is not evidence for an automatic-resume option.
Consequently 0083h does not justify an invented automatic wake event.

Implement the following bounded L2 functional receiver, not an Intel-to-VLSI
per-index specification or physical timing claim:

- The existing Chip's FFh bits 7 and 1 together request suspend refresh.
  This is derived from its one register store, not a second power-state enum.
- One physical resume-button input owns its level. Only a new asserted edge
  while suspend refresh is requested produces a resume-reset request. A
  held input, release, cold press or absent event does not produce a reset.
- That qualified edge clears FFh bit 7 and latches B8h/FBh bit 3. These two
  cause bits are the selected firmware's demonstrated reset/restoration
  consumers; their mapping is explicitly L2. Other bits remain untouched;
  guest writes still acknowledge the cause. Neither read forces a status bit.
- The App forwards suspend refresh to the existing CPU stop-clock input;
  a qualified physical edge releases that input and requests processor-only
  RESET on the same executor. RAM, SMRAM, CMOS and board configuration are
  not cold-reset. The CPU remains the sole reset/architectural-state owner.
- The chip adds only `suspend_refresh_enabled` and `set_resume_button`
  operations. App's firmware binding adds the serialized button operation.
  There is no Lib/Emulator change, new queue, clock, worker or automatic
  button injection in production. Product input wiring and original BIOS
  restoration must still be proved before this whole batch is accepted.

This is a falsifiable candidate. The existing chip/config fixtures must
reject spontaneous reset, repeated held-button reset and loss of retained
data. Actual firmware must distinguish stopped clock from its old HLT
endpoint; a changed endpoint alone is not success. If original restoration
contradicts the candidate, revise or remove it rather than intercept a PC.

#### First actual wake observation: missing REFNORM input

The x64 candidate reaches an unchanged no-event PC 39333h with zero further
retirements. One explicit button assertion produces the fourth processor
reset. At the 40-million-instruction ceiling the diagnostic correctly fails
its restoration predicate, with no terminal Core fault and PC F339Fh.
This is not a passing power/boot qualification or permission to raise the
budget blindly. Static continuation inspection changes the next receiver:
CMOS shutdown value 0Eh dispatches through the real ROM table at F46A9h to
F332Ah. This routine reads B8h bit 3, clears FFh and then polls 704h bit 0 at
F33AEh, repeating through F339Fh while zero. Thus normal resume dispatch has
already happened; the stalled consumer is the missing refresh-status input,
not evidence that ordinary POST restarted or that the CPU needs a PC redirect.

The original reference's printed page 164 names REFNORM as MCRF/704h bit 0
and requires checking it before accessing retained DRAM. Add that read-side
functional status at the existing App CPU-configuration receiver, derived
from the one Chip suspend-refresh output: 0 while enabled, 1 after release.
No countdown, extra cached flag, controller clock, register-write completion
or rate-field model is claimed. Unit assertions cover all three phases.
The existing BIOS must then perform its own SMRAM checksum and restoration.

The corrected x64 observation passes the 704h wait and the firmware's SMRAM
checksum, then reaches its explicit F3419h self-loop awaiting SMI. Its
unchanged restoration predicate still fails at the same 40-million ceiling;
the second log SHA-256 is
`627B71BB5F3A42E741D9319EA01DF166A64E26881E6A1CD83597628674DF9EEB`.
The preceding failed log SHA-256 is
`35774219E8AE990977EC89B17CD8BC5BE1E0DA0810164BA8473432127EB29C1D`.
These are distinct missing receivers, not passing retries or a need for more
time. ROM code at F3414h issues FA=1 after setting B6=81h. The retained
SMM dispatcher's byte-3 bit-7 path invokes its registered mask-80 restoration
handler independently of B7's ordinary source list. This requires one actual
resume-SMI pending latch, set only by the qualified physical event, serviced
through the existing SMI enable/rearm owner and acknowledged by FB bit 3
clear. A cold static FB=18h cannot stand in for that event. No invented B7
resume bit or immediate forced SMI is used. Full reset clears that transient
latch; processor-only resume preserves it until firmware acknowledgement.

The next candidate demonstrably enters the original 39336h restoration
routine. Its 40-million observation still does not see RSM: it ends in the
shared firmware delay at 3E535h, without a new terminal fault. Log SHA-256:
`C35C4F10814CBD9E4AC6E15C383D0BE9C18DE91A0FAA348D9B5C6A629C7BC297`.
That routine counts refresh-bit transitions in 16-bit CX; an initial zero
requires 65,536 transitions. With the existing 498-tick source the maximum
single countdown alone represents 32,636,928 guest ticks. A partial observation
inside it is not proof of deadlock or of completed restoration.

The corrective observer therefore uses existing execution watchpoints at
restoration entry and RSM, then executes the original RSM with the existing
single-step operation. It removes per-instruction snapshots from the delay.
Its native deadline stays 300 seconds; total-retirement containment is
60 million, including the already observed cold prefix. This changes only
diagnostic coverage and overhead, not production timing or the success
predicate. Original firmware, all register/data checks and the requirement
for actual RSM remain unchanged.

The first watchpoint observer is rejected: CPU reset clears its debugger
controls, so an initial-only breakpoint cannot prove absence of restoration.
Its log SHA-256 is
`2927874155126B023EB0EB6159B49FFEB1FE4B22A59FB5561D19622EAB471190`.
After 60 million retirements it records a delivered-exception storm, not
qualification. A subsequent diagnostic was stopped explicitly after source
review also found that an execution watchpoint reports after its instruction,
not before it. Neither defective observer is accepted as a hardware result.
No production change is made to satisfy those observers.

The corrected observer rearms after the actual processor-reset result,
identifies the retired instruction through the existing copied watch-hit
observation, and follows at most 128 actual single steps through the EOSMI
helper to the RSM boundary. It compares the original and restored 512-byte
CPU save area before executing RSM and rejects new delivered exceptions.
Those are stronger real restoration requirements, not a budget-as-success
test. The native deadline and the single Core execution path are unchanged.

One further boundary detail is verified rather than worked around in
production: a watch hit can precede the physical retirement tail. Clearing
the watchpoint correctly returns INVALID_STATE until that tail completes.
The observer uses one ordinary step and proves that CS base/EIP do not change
while completing the already executed CALL, then clears the watchpoint. The
earlier observer that stopped at this guard is rejected, SHA-256
`CC53FB861DC5CBB33B261AAB570E4F9DF486B0A3A2FE3428644C76BB18116570`.
It is not a second hardware failure or a reason to weaken the Core guard.

### Actual retained-reset/restoration proof

The corrected observer completes normally on both widths. Each execution:

1. Reaches the real guest FF=82h request; a second run with no event retires
   zero instructions and stays at 39333h.
2. Applies one explicit physical resume-button edge through the App/Chip
   receiver, not a direct register write or a test CPU reset.
3. Observes the fourth processor reset and the original ROM resume path.
4. Reaches original restoration 39336h, completes its helper/delay path and
   reaches the original EOSMI/RSM exit.
5. Proves all 512 saved CPU bytes are identical to the pre-reset image, then
   executes the actual RSM and returns to F000:DD11 with flags 00000046h.
6. Retains the original four delivered GP probes with no new exception or
   terminal Core fault.

Both totals are 48,828,526 retirements. This is a completed hardware receiver
observation, not an assertion that the post-RSM guest boot, normal Product
input, panels, desktop/RDP or whole PC110 product are qualified. Timer observer
fields are deliberately unsampled in this mode; their zeros are not failures
or new timer passes. The original prefix regression is separate.

| Observation | SHA-256 |
| --- | --- |
| x64 retained CPU image/RSM | 145911A157ABE5588CE552C6BBD39F67083501B5245633EC200AFE14F2D18E6C |
| x86 retained CPU image/RSM | 187EAB87EACD8AD3FE0EB492F44262EADB4DA1C7A2508B8D717CD3BD4B1939E2 |

Receiving-target inspection shows `x86-core-vl82c420` is an independent Types-
only library. Only MyPC110 and its chip test link it; it is not linked into
`x86-core-machine` or the published PC/MyNES/MyRaidenII products. Thus this
batch so far needs four unpublished preview rebuilds, not timestamp-only
refreshes of ten unrelated EXEs. A later change to the family Machine adapter
would have its own actual receiving-artifact requirements.

### Normal Product input receiver admission

The remaining production gap is real: the firmware operation is otherwise
reachable only from the diagnostic. Add one optional `handle_key` operation
to Machine's existing copied profile binding. Machine invokes it on the same
executor before its ordinary key conversion; true means the profile consumed
that event, false retains the existing keyboard route. The callback borrows
the event only for its duration, uses the same profile context/lifetime, and
reports a physical-operation failure through the existing Core fault owner.
It adds no provider, queue, thread, global state or Lib/Emulator contract.

The PC110 preview's host binding is F12 make/break to the resume-button
level. This is an emulator control choice, not a claim that the original
PC110's F12 key is its power switch. It wakes guest firmware suspend, not
monitor pause; it is reserved from guest keyboard delivery on PC110 only.
Other profiles' null callback and unconsumed keys preserve the original path.
No common PC command/hotkey/help implementation is replaced. The binding is
documented in the PC110 UX and remains prepublication until its full gates.

Estimated source delta +25--45/-0--5; extend existing Machine construction
and App construction fixtures by +35--60/-0--5. The real-firmware power mode
will use the actual driver delivery operation, not call the pin directly.
This explicit Machine public-profile receiver changes the eight published
PC receiving artifacts; rebuild them once on the frozen qualified source,
not at each intermediate diagnostic. MyNES/MyRaidenII remain unaffected.

The full-cold-reset sweep also removes the old partial EC/ED-only reset loop:
firmware reset now reuses the App's one cold-image initializer. This prevents
FF suspend refresh surviving a full reset; processor-only resume never calls
it, so SMRAM and the wake cause survive that operation. There is no second
reset table. The physical input level remains a level, not a fabricated press.

Both widths pass the final scoped thirteen cases (eleven App units, the
Chip fixture and existing Core system-power fixture): x64 6.51 s and x86
0.95 s in `rearm-regression.log`. This is not a complete repository/unit,
guest, desktop or publication qualification. Four previews are rebuilt in
the stable caches and remain unpublished. Manifests for Shared X86 Core
source/test and its dependency gate pass; Lib/Emulator remain unchanged.

Current source-only delta against S52, excluding manifests and the temporary
observer: +179/-27 (net +152); existing unit-fixture delta +169/-9 (net +160).
The power addition alone, excluding the earlier EC receiver, is production
+75/-15 and tests +84/-0, within the refined pre-change estimate. Two Chip
Booleans represent distinct real facts: physical button level and an event
awaiting SMI rearm. No power enum, new queue, worker, retry or clock is added.
The new shared surface is exactly two Chip operations. The App operation
coordinates that pin with the existing Core stop-clock/reset inputs; it is
not a second reset executor. The complete S remains open for verified
restoration, production physical-input delivery and the original full gates.

The corrected, lightweight actual-firmware replay completes normally on both
widths. Each log is byte-identical, SHA-256
`DFD4CC6AAC83BF8A2F3B79BB1C8963DC523C5F75F546EE5BDF864DDEDF63A599`.
The CPU consumes identifier 21h, input status F0h, the actual D2h RAM values
and controller configuration 30h through its real IN instructions. Both
execute 27,893,750 instructions, deliver the same four known GP probes and
three processor resets, and stop at the original suspend endpoint 39334h
without a terminal Core fault. This removes 4,125,499 instructions of failed
response protocol relative to the S52 prefix; it does not measure host frame
rate or establish hardware timing. The observer-specific TIMER flags are not
sampled in this light mode, so their zeros are not a new timer pass or failure.
The original timer/SMI/FDC prefix is separately revalidated below.

All temporary App reply prints and the temporary harness mode are removed.
The restored code builds its existing eleven App units and all four preview
EXEs. Results, once per width on that restored code:

| Scope | x64 | x86 |
| --- | --- | --- |
| All eleven MyPC110 units | 11/11, 4.87 s | 11/11, 1.62 s |
| Original real-firmware FDC/reset/timer/ASMI/RSM integration | Pass, 47.94 s | Pass, 66.29 s |
| Corrected EC consumer diagnostic | Defined replies and genuine old suspend endpoint | Identical replies and endpoint |

The complete repository unit corpus, other products' guest integrations,
desktop/RDP and full PC110 power/boot qualifications are not rerun or claimed
by this phase. Lib, Emulator and every Shared source/test package remain
unchanged. App trees have no manifests to regenerate. Existing deployed Apps
do not link MyPC110's private board library and retain their current artifacts.

### Actual change review in progress

Against the S52 HEAD, `git diff --numstat` for the three changed tracked C/H
paths gives production +104/-12 (net +92), test +85/-9 (net +76), total
+189/-21 (net +168). Documentation and generated artifacts are excluded.
The increase implements one real 512-byte controller RAM store, one reply
latch/availability bit and its bounded command/parameter protocol. The only
new production helper owns reply publication; no thread, event queue,
executor, registry, compatibility path or Shared API is added. Tests extend
the existing fixture rather than adding an executable or framework. The
original write-input latch is distinct from the outbound reply register.

The retained path is App port callbacks through Core's original route; RAM
and protocol state belong only to that App instance. No firmware PC, image
hash, post-POST capture bit, reset-vector redirection or synthetic wake selects
production behavior. Accepting a bus write does not certify its unimplemented
GPIO, serial or physical-power effects; no reply or completion is fabricated
for those effects. S53 is still active;
these changes are not a complete implementation P or task delivery.

### Unpublished rebuilt previews

| Width / panel | PE machine | SHA-256 |
| --- | --- | --- |
| x64 TFT | 8664 | EE0FF5EE520DE2042CA307D227B88055D23FCACFC75061CE642D715636D74720 |
| x64 DSTN | 8664 | 2F47AF6B238818F07358F223C11070A760FC820C2A58F0674397331A2DB44DC5 |
| x86 TFT | 014C | AF9A302DE558FCE3F6511E34A060DF74A99CB06FEB4131B0DF17EA04EA99F10E |
| x86 DSTN | 014C | C919EAA45274AF69190FBFDF72164D72365C38862A137AE550594B85C9B2C994 |

These remain ordinary generated targets in the two stable Ninja trees, not
published PC110 products. Their DSTN input still has the documented derived
panel provenance; no new factory-ROM claim is made.

### Receiver-phase cleanup

Both bounded-consumer and restored-regression controllers are terminal and
no owned native integration process remains. Ten obsolete receiver-phase
logs total 18,904 bytes. The essential results, discarded-attempt limits and
hashes are recorded before official preview-first cleanup; the two reusable
caches and four generated previews are retained in their proper build trees.

| Receiver-phase output | Bytes | SHA-256 |
| --- | ---: | --- |
| app-regression.log | 7412 | A6B2619B4FB36767A9F9AFFE185D54462962D5820D0C322B0D05D1BFD104D4B5 |
| ec-consumer-builds.log | 466 | 844612D02D325ECE1AF2010685AE606517705B426DDA6BB546FD7C44B3E8EF32 |
| ec-consumer-x64.log | 1393 | DFD4CC6AAC83BF8A2F3B79BB1C8963DC523C5F75F546EE5BDF864DDEDF63A599 |
| ec-consumer-x86.log | 1393 | DFD4CC6AAC83BF8A2F3B79BB1C8963DC523C5F75F546EE5BDF864DDEDF63A599 |
| ec-final-builds.log | 2230 | DF0BCC1D2B94CDC686E80EB9D9F8BEEFEAFA3EE4424FA09A94F4A1A822751BF9 |
| ec-final-x64.log | 698 | 0DBFD038D181537535AD94CEB72C6D4991D3759E559A93C4FF224FD3F2109D05 |
| ec-final-x86.log | 698 | 0DBFD038D181537535AD94CEB72C6D4991D3759E559A93C4FF224FD3F2109D05 |
| ec-replies-builds.log | 466 | 844612D02D325ECE1AF2010685AE606517705B426DDA6BB546FD7C44B3E8EF32 |
| ec-replies-x64.log | 4146 | 9D5E06C699B77B82A1FEDE36C7DD8B8FAD7E50451062EE0F190558FB519C29A0 |
| ec-replies-x86.log | 2 | 7EB70257593DA06F682A3DDDA54A9D260D4FC514F645237F5CA74B08F8DA61A6 |
