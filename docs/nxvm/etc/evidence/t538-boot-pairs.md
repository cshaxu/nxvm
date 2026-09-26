# T538 Boot-Pair Convergence Ledger

Frozen baseline: a2418ae57, four owner INIs and eight 0535 deployed EXEs.
Expected checkpoint is a real DOS prompt/date input or DOS installer. A stopped
process, unchanged screen, or expired time budget is not success.

| Case | Media / presentation | x64 baseline | x86 baseline | Repair / proof |
| --- | --- | --- | --- | --- |
| XT model 268 | DOS 5 360KB; console | Installer, about 27 seconds | Installer, about 33 seconds | Isolated real-process Console captures; paused debugger remains usable |
| AT model 339 | DOS 5 1.2MB; console | Installer, about 33 seconds; intermittent 303 reproduced | Installer, about 53 seconds; intermittent 303 reproduced | S2 KBC repair proves old-fails/new-passes with 50 ms Return release on both widths; final whole-task repeats remain pending |
| DeskPro Model 40 | DOS 5 1.2MB + blank 40MB; window | Installer at 180-second capture | Full installer at isolated live-window 180-second capture | Earlier hidden-window partial captures do not establish guest failure; isolated image and paused text memory agree |
| Default PC/AT | DOS 5 1.44MB + Win31 50MB; window | DOS A prompt | DOS A prompt | Actual native Window captures |

Permitted dispositions: directly proven checkpoint; reproduced failure with
named owner/repair receiver; blocked input/observation with exact reason.
No pair is non-applicable. T closure requires all eight to be directly proven
on the replacement artifacts, three fresh launches each, and complete applicable
units/integration. Ledger refinements require recorded evidence, never dropping
a failing row. S1 closes only the baseline inventory, not this completion gate.

## Frozen Inputs

All eight executable SHA-256 values match the complete
[T43 S10 receiver inventory](m6-t43-s10-receiving-artifacts.md); no product
executable has been changed. The four INI hashes are:

| Profile | SHA-256 |
| --- | --- |
| Model 40 | `4C363DB4FD3FA8864C89B2557FA2A2BF93994E67E295B11AEE07053EC1FDB0E3` |
| Default | `A858BEE0D2145E539D69861A675FDDB10433B25EAD658A9DFAF0D69632989F13` |
| XT | `48900ED858BE7B6A5AE8CD452B4F4432FC9446365FC366A49470DA331C443B29` |
| AT | `282478D7DFB479070FDB138867DD08C99C5283770C5872DDEA07529D32C8A15C` |

Every INI media path resolves from its own directory, exists, and uses overlay:

| External media basename | Bytes | SHA-256 |
| --- | ---: | --- |
| fdd_360k_msdos_500.img | 368640 | `DE271368874209C07A2FC25C81C17529D4BDD7718B2731B86C49A2DA923A256E` |
| fdd_1200k_msdos_500a_01.img | 1228800 | `0F51D92B482253FC468A2B470FFAB82DB43898D1C8B44E504808B7A3EF3D4BDE` |
| fdd_1440k_msdos_500.img | 1474560 | `FADEB3A27C6A0E1CF582DDE0B9AECB7E5D30678F2F967F2F4562F167CC0CB1D5` |
| hdd_40m_deskpro_386_blank.img | 40256000 | `2BBC68E612A72290A5181E070494A7580CBECE1F528F92F009A269DC05819E73` |
| hdd_50m_win31.img | 51609600 | `61E5CDC0B76151CC65B73EB44094738B9DE86052B1B07F20FC03205984CD77E1` |

Selected firmware/CMOS/font sizes and SHA-256 match the profile selection table
in `cmake/nxvm/NxvmProductProfile.cmake` at a2418ae57. No master was modified or
copied into the repository. The product binaries retain their declared external
asset binding; no INI firmware fallback was used.

## Observation Method And Limits

Owner clarification during S1 identifies IBM 5170 reporting `303 System Error`.
Treat this as an unresolved POST failure even though the initial x64/x86 runs
reached the installer. Repeat unchanged deployed AT cold starts before assigning
a cause; one successful boot does not establish absence of an intermittent error.

Read-only inspection of the local PCjs archive's IBM 5170 BIOS 1985-11-15
`src/TEST2.ASM` keyboard-test branch (F6/F6A/F6B) distinguishes the report:
303 follows keyboard reset failure when the subsequent 8042 ABh interface
test either never fills OBF or returns a nonzero byte. A successful zero
interface-test result instead selects 301. The observed 303 therefore needs
the ABh response and preceding keyboard-reset/IRQ sequence captured; it is not
evidence of a missing disk image. No firmware source or payload is imported.

Three additional x64 cold starts reached the installer without 303. A separate
x64 run with the start Return key-up delayed 300 ms also reached the installer;
that one handoff condition does not explain the reported failure. All three
x86 repeats also reached the installer. Owner subsequently confirmed that 303
is intermittent on both widths, not x64-specific. Do not convert these negative
reproductions into a fixed disposition.

Diagnostic-tool checks: x64 full repository-only unit 335/335 passed (216.16 s).
x86 non-desktop unit partition 332/332 passed (262.92 s); its three desktop
cases then passed 3/3 (1.98 s) after real-process observations finished, avoiding
native input/focus interference. Together these cover all 335 x86 unit cases.
Both widths compile the final diagnostic target.
NXVM documentation governance passes. These checks establish neither a repaired
5170 POST nor complete T closure; production binaries and INIs remain unchanged.

The Model 40 x86 live-window rerun paused around 59 seconds, before the observer's
200-second endpoint, and produced no usable Window bitmap. It is not boot-success
evidence or proof of a display defect. The reason for that earlier pause remains
unclassified; preserve the log separately from the earlier hidden-window captures.

`nxvm-deployed-boot-probe EXE DIRECTORY SECONDS LOG BMP [START_RETURN_RELEASE_MS [SCENARIO]]` launches the actual
EXE in a new Console, leaves its adjacent INI intact, sends monitor commands,
reopens CONOUT$ after buffer handoff, and records process exit and Console
content. A Window capture briefly shows that surface without activation and
copies its client pixels. Its return code means observation completed, never
boot acceptance. The coordinator reads the captured semantic checkpoint.
The final observer also records `info` before `start` and reads registers via
the existing paused debugger after the boot observation, not through a new API.
The optional delay changes only the start Return key release (0--1000 ms).
An optional scenario is `reset`, `restart`, `post-key`, or `post-burst`.
The latter sends at most 120 A make/break pairs, one per observer iteration,
starting at the first `00512 KB OK`; it probes input sensitivity, not normal
boot acceptance. None changes firmware, INI or stored guest state.

`post-hold` sends only the make event at that checkpoint. The observer now
supplies real virtual-key/scan-code pairs for monitor command letters too;
earlier zero-key-code letter injection did not fully model native handoff.

## S1 Baseline Disposition And Next Batch

All eight pairs have been inventoried and attempted through the real process.
Seven have directly observed DOS/installer checkpoints; Model 40 x86 remains
an incomplete-display/observation case. No process crash was reproduced.
Two unresolved classes remain in this T: AT intermittent POST 303 on both host
widths, and Model 40 x86 guest/presentation progress. The next diagnostic/repair
batch must cover cold boot, reset and stop/start, retain input identity, and
compare response/IRQ order for the former and guest text versus displayed text
for the latter. Owners are NXVM device execution and its machine adaptation;
a demonstrated Shared defect requires owner review before editing Shared.
No production repair or T-level acceptance is inferred from S1.

## S2 Diagnostic Updates

The isolated Model 40 x86 run now shows the complete DOS 5 installer in its
native Window. Its paused debugger reads the full DOS title at B800:00A0;
CPU is in the BIOS keyboard-read path (F000:DF4D). This resolves the earlier
partial-screen observation as insufficient evidence of a guest hang, not as a
production repair. Preserve `model40-x86-live-isolated` log/bitmap for this S.

5170 investigation now includes real monitor reset/resume and stop/start
sequences through the diagnostic observer. These run at the midpoint of a
bounded observation, record the lifecycle request and subsequent Console output,
and cannot pass merely because the first boot reached its installer. There is
still no confirmed cause of the owner's intermittent 303.

The first x64 reset/resume run reached the installer before the request and
again afterward (second complete screen around 97.5 seconds, request at 60
seconds). No 303 appeared. The separate x64 stop/start run also reached the
installer again, around 98.3 seconds. An x64 run injecting one A make/break
when the screen first showed `00512 KB OK` reached the installer too
(`at-x64-post-key`, `POST_KEY_SENT=1`). These negative reproductions exclude
neither other input timings nor the owner's intermittent failure.

Source inspection found a diagnostic distinction still to prove: the existing
KBC read path calls `advance(0)` after consuming ACK, and that call can already
publish pending BAT. Its comment describing BAT as waiting for the next time
callback does not describe that path accurately. Existing CPU/PIC tests expect
the immediate BAT and IRQ edge; this discrepancy alone is not evidence that it
causes 303, and no production timing change has been made on that basis.

The x86 delayed-Return run (1000 ms) reached the installer. The x64 continuous
POST-input run instead displayed `1E 301-Keyboard Error` at about 9.6 seconds.
That is consistent with the BIOS stuck-key check after successful reset: it
prints the scan byte, then 301. It is not the owner's 303 interface failure,
and does not justify dropping ordinary input or bypassing POST.

### S2 Reproduced 303 And Candidate Repair

The unchanged deployed 0535 AT x64 EXE, unchanged adjacent INI and real key-code
observer reproduce `303-Keyboard Or System Unit Error` followed by the F1 prompt
with a 50 ms start-Return release delay (`at-x64-physical-return-50`). A 300 ms
real-key-code run and the isolated post-hold run reached the installer. Timing
therefore matters at monitor/raw input handoff; this is no longer only an owner
report or a synthetic controller failure.

Two owner-local regression checks fail before repair:

- An unmatched Set-2 Return break starts typematic. The make branch did not
  exclude a pending break prefix when its key differed from the last repeat key.
- Typematic writes directly to output instead of the keyboard serial backlog.
  Four repeat ticks produce five output bytes including the original make;
  five ticks while inhibited still produce five bytes. Replaying POST's
  flush/E0/FF/AB order reads `1E/1E/1E`, contaminating AB's zero interface result
  with a scan code and satisfying the ROM's 303 branch.

The candidate excludes all break bytes from typematic activation and routes
automatic repeats through the same owner-local serial queue as native input.
The boundary check then observes one output byte, zero while inhibited, and
E0/FF/AB results `00/FA/00` (consuming reset BAT before the separate AB test).
The controller regression passes on x64 and x86. The first full-ROM 50 ms replay
against the 0538 x64 candidate reaches the complete DOS installer without 303
(`at-x64-fixed-return-50`); the copied INI has exactly the frozen AT hash and
retains the same relative media resolution. This is an old-fails/new-passes
observation, not the required repeated dual-width acceptance yet.
An independent confirmation batch reproduces 303 on both original widths
(`at-return50-confirm-old-x64` and `at-return50-confirm-old-x86`). Both candidate
widths reach the installer with the same 50 ms setting
(`at-return50-confirm-fixed-x64` and `at-return50-confirm-fixed-x86`). Thus the
host-width difference is excluded: an input-handoff timing window exposes two
guest KBC defects, not an x64-only ROM/configuration difference. The regression
also checks an unrelated break while another key is held, preserving its repeat
owner, and covers both single-byte and batch input APIs.

Similar-issue sweep: searched `publish_native_byte`, `submit_native_byte(s)`,
`typematic` and `break_pending` across `src/app-nxvm`. The single and batch
machine entry points both reach this KBC owner for AT/default/Model 40; XT
routes to its separate PPI keyboard and has no matching typematic producer.
Set-1 already excludes high-bit break codes from the make branch. After repair,
the serial drain is the only caller publishing native scan bytes; generated
repeats and both external submission APIs use the same queue. Controller command
responses remain distinct protocol responses, not scan-input bypasses. No new
profile-specific path, Shared change or public interface was needed.

The build deploy hook produced untracked 0538 AT candidates under the product
asset directory; the original 0535 pairs and INIs remain intact.
Full dual-width unit/integration, remaining product rebuilds and final artifact
replacement are not yet complete.

Candidate full x64 units pass 335/335 (206.00 seconds), including all three
desktop cases. The first x86 full run was stopped after concurrent-build
timeouts (MOVS, default apply, runner error propagation, SETcc and T359 timing);
several printed success before process timeout, which is not a passing result.
The final regression is rebuilt; an isolated complete rerun is required without
changing test expectations or timeout limits. That isolated full run completed
334/335 in 336.50 seconds: all 332 non-desktop cases passed, while the modal
Window case failed its still-in-modal assertion alongside concurrent desktop
cases. All three desktop cases then passed three repetitions each with `-j 1`
(4.52 seconds). Together the runs cover all 335 without changing source or
expectations; the parallel desktop failure is retained, not called a full-pass
run. All eight product candidates compile as optimized Release with PE
architecture checks. Four owner INI hashes remain unchanged. Full external
integration subsequently passed 20/20 on x64 (12.09 seconds). Replacement-pair
boot acceptance remains pending: the first final batch reached DOS A prompt on
default x64 and the complete installer on Model 40 x64. AT and XT on both
widths instead exited with code 1 immediately after start, before POST. This
new Console-mode observation is not classified as the earlier 303 and requires
old/new replay and exit-surface diagnosis. Default x86 became paused around
45.7 seconds before the observer's scheduled pause; no terminal image was
captured, so it is not accepted from this run. Observer exit zero means only
observation completed, not product success. The old 0535 artifacts stay in place.

Follow-up full runs (`recheck-*`) reach the complete installer on both AT and
XT widths, and a captured A prompt on default x86. AT retains the 50 ms Return
release trigger. Model 40 x86's `model40-x86-print-compare` ordinary Window
capture shows the complete installer and the paused BIOS keyboard-wait location
matches x64; the immediately following PrintWindow capture shows only `Micr`.
Consequently a partial native capture alone cannot establish guest no-progress.
Both capture methods are retained as diagnostics, not automatic success checks.
Each of the eight replacement pairs now has a direct terminal checkpoint;
the task's three-fresh-launch predicate is not yet met.

The immediate Console exit remains unclassified. Six alternating old/new short
launches and a two-observer comparison all remain alive and enter POST; these
negative reproductions do not erase the four recorded exit-code-1 failures.
The diagnostic observer now also attempts to read its retained Console handle
after early process exit, before cleanup. No Shared source was changed and no
INI was rewritten to work around this observation.

A temporary NXVM-owned linker-wrapper diagnostic uses the same optimized AT
objects and frozen INI. It observes existing Common UI return values and forwards
the original event sink; it does not alter Shared sources, public APIs or the
deployed EXEs. Twelve five-second launches covering Return release at 0, 1, 10
and 50 ms enter POST without exit. Console create/bind and monitor return all
report OK. A separate post-desktop-test four-launch batch also remains alive.
These short runs narrow the investigation but do not count as DOS acceptance
or prove that the earlier exit class is resolved. Wrapper source and ignored
diagnostic outputs are retained only for this active investigation and must be
removed when they cease to serve it.

The 180-second diagnostic-linked AT run `failure-wrap-reset` reaches the
complete installer both before and after reset/resume. All observed create,
bind and monitor-return actions report OK. This corroborates the reset path;
the final deployed x64/x86 reset and stop/start batch is still required and is
run separately without linker wrappers. No matching NXVM/Console crash record
was found in the last two hours of the Windows Application log; that absence
does not classify the recorded exit-code-1 observation.

The actual deployed AT x64 `accepted-at-x64-reset` and
`accepted-at-x64-restart` runs each reach the DOS 5 installer before and after
their requested lifecycle transition. They retain the 50 ms start-Return
release trigger and run without linker wrappers. Their final observed process
status is STILL_ACTIVE (259), not an exit or an automatic success verdict;
the installer text is the checkpoint evidence. Neither log contains 301/303.
The corresponding x86 reset and restart runs also reach the complete installer
both before and after their transitions, with final STILL_ACTIVE and no 301/303.
These observations
do not resolve the earlier early-exit class or replace three fresh launches
per final pair.

A new bounded Console-size experiment reproduces a separate pre-POST exit.
Only the diagnostic child's native font is changed; the product objects, frozen
INI, Shared sources and deployed binaries remain unchanged. At height 16 the
reported largest visible Console is 80x25 and the process enters POST. At height
24 it is 58x17, and at height 128 it is 10x3; both receive
COMMON_UI_EVENT_KVM_DELIVERY_FAILED / LIB_STATUS_IO_ERROR and exit with code 1
after successful Console create/bind. The initial backing buffer remains 75x9001
in all three runs. Thus backing capacity is not the limiting resource.
The early failed batch's captured initial stride was 55 columns, compared with
75 in successful replays, but its font/maximum viewport was not recorded: the
new reproduction proves this failure mechanism, not every historical exit's
identity. Shared console_broker_ensure_text_surface requires a visible 80-column
surface and returns failure when the native host cannot provide it; Common ends
the session on the delivery failure. Font fitting or a Window fallback changes
Shared presentation policy and requires owner review before implementation.

Owner subsequently approved a different policy: retain complete backing storage
and allow a smaller scrollable viewport; do not scale fonts or switch to Window.
The Console broker now grows storage only, verifies the resulting capacity and
writes all active cells, clearing old lower rows within its 50-row capacity.
It does not resize or rebase the viewport. True capacity/query/cell/cursor I/O
errors remain failures. Existing unit tests now distinguish small visibility
from failed or silently ignored backing growth, retain clipped-write rejection,
and verify offscreen last cells and scrolled viewport preservation. Native
narrow-buffer/switch tests pass. The changed production file is +10/-35 lines;
its two tests total +24/-33, excluding manifests and documentation.

The same diagnostic child at font heights 24 and 128 now enters POST and stays
alive, including reported maximum visible dimensions 43x35 and 8x6 respectively.
No delivery failure is recorded; before repair both conditions exited with code
1. Host dimensions changed between experiments, so the proof is for the same
undersized-viewport condition, not identical desktop resolution. These five-
second probes establish failure-path repair, not full DOS acceptance. No product
font API or user INI was changed. Final NXVM units pass all 335 cases on each
width (332 non-desktop plus three serial desktop cases); all six manifest checks
pass. Final MyNES receiver suites pass 132/132 on each width (127 non-desktop
and five serial desktop cases), and the NXVM external integration suite passes
20/20. All eight NXVM 0538 and both MyNES 0043 receiver builds completed;
optimized Release and x64/x86 architecture checks passed. Four INI hashes
remain identical to S1. Final deployed acceptance remains in progress. The
temporary linker-wrapper source has been removed after recording its diagnostic
results; it is not part of the production or test corpus.

The final `final-buffer-at-x64` and `final-buffer-at-x86` deployed runs each
reach the complete DOS 5 installer with the 50 ms Return-release trigger,
remain alive through the 180-second observation, and contain no 301/303.
These are one fresh run per final AT hash, not the three-run whole-matrix gate.

The following are the post-Console-fix candidate hashes, not a pushed baseline.
Owner subsequently transferred this combined candidate to S3. They are not
the isolated S2 KBC delivery hashes recorded below.

| Candidate | SHA-256 |
| --- | --- |
| NXVM AT 0538 x64 | D22EDC51BA8EF2A265C8E8F9C5339843224542C1F862B829EB7429510A910541 |
| NXVM AT 0538 x86 | 754B25F4C12186D6CFB4AA8EAE928F626DB3C5BD624DFD816B29AEAE9669FEDB |
| NXVM XT 0538 x64 | 9C918F05620DACE896CCEF640C026AFBCCF61008B52E48C66C93A8A9254E1D71 |
| NXVM XT 0538 x86 | 6495E00EE05E71E936A722A4FD2CC029F7D113F1C642EF04E4BCAC122A4D39C3 |
| NXVM default 0538 x64 | 5532108714B019EA8E5F202673C19C230493B5CF86733F837620A1E3DD57DAC2 |
| NXVM default 0538 x86 | 90A650BD8E6F48275982A7E07A36F21ABB79D2FA9FD3FCE6B7D941E97B7268AF |
| NXVM Model 40 0538 x64 | 6EA04B34BF793F2D2615E5B9FD6164498540C1284470EA387C384C7B4E272BF2 |
| NXVM Model 40 0538 x86 | F75C8F5F01DEA8918A8C94C622E183218131167DCCDB23282DE5B937F7EA4C3A |
| MyNES 0043 x64 | B18FE1042EFE5E2CDF626E7DA4E24B6354A7FA96B09AA7B0FF89DDE2A483B3B9 |
| MyNES 0043 x86 | D414F7B232D6C059E1F57AFA2C9E3871DBED1162481AE6E6B1EB11F4D65D0815 |

An exploratory modification of the existing native lifecycle fixture also
observed a separate issue: a 120x60 cooked history buffer with a 40x13 viewport
was restored as 120x13 after injected raw-reader startup failure, before any raw
frame write. The original wide-history lifecycle case remains unchanged; the
small-viewport frame cases cover the approved repair. That partial-initialization
rollback observation is retained in TODO for separate Shared review, not claimed
fixed by the backing-growth change.

Run it with an external process-tree watchdog of SECONDS + 15 seconds because
a native Console operation itself can block. Launch the helper hidden and use
only its verified child tree for timeout cleanup. Raw observations stay under
ignored `build/t538-s1/` for the immediate next diagnostic batch.

The first hidden Window captures were black because no visible paint had
occurred; they are not guest-state evidence. A visible-console batch received
unrelated keyboard input (`Illegal command ',start'` / `',bstart'`) and its XT
rows were discarded. The corrected isolated batch is the authoritative run.
An initial eight-case monitor/exit-only check passed, but that old check never
sent start and cannot establish boot reliability.

The Model 40 current-source guest observer also reached installer-running
within its 180-second standard-speed bound. It is corroboration only, not a
substitute for deployed-process proof. XT and AT both widths reach complete
installer screens in their extended isolated runs; Ctrl+Alt+P and existing
Debug register reads work. Model 40 x86's 300-second observation can close its
Window, report Machine paused, and read registers, despite the incomplete
captured display. The observer now exposes an existing Window without activation
as soon as it is created to distinguish native hidden-paint behavior from the
product failure; that result must precede a production root-cause claim.
No matching Windows application-error
record was found in the preceding two days. Neither fact disproves the owner's
intermittent crash report. No production repair is claimed by this S1 baseline.

## Owner-Directed S2 / S3 Split

On 2026-09-25 the owner accepted the 5170 repair for S2 closure and requested
a separate S for the Console issue. S2 therefore delivers NXVM KBC and its
regressions/observer only. The six pending Shared files were preserved in an
ignored patch and returned to the committed baseline during S2 builds and
tests. MyNES receiver changes remain pending S3; none enter the S2 commit.
Whole-task three-fresh-launch coverage remains required after the Console repair,
not retrospectively claimed from successful diagnostic observations.

The isolated S2 tree again passes all 335 NXVM unit cases on each architecture
(332 non-desktop and three serial desktop), plus 20/20 external integration
cases. The similar-issue query over src/app-nxvm and test/app-nxvm for
core_machine_kbc_publish_native_byte, submit_native_byte(s), and
set_typematic_timing finds the sole production publish caller in the serial
drain; external single/batch submissions and automatic repeats enter that
backlog. Machine adaptation has exactly one KBC path and a separate XT PPI
hardware path, not a duplicate AT implementation. Existing translation, BAT,
IRQ, serial timing and XT tests remain passing. No guest timing grade changes.

Actual-diff review counts four tracked source/test/build files with git diff
--numstat: +188/-55, net +133. Production KBC alone is +30/-28, net +2;
regression and real-process diagnosis account for the increase. The latter
is observation-only, not an additional product execution or success path.
The comment correction does not alter BAT scheduling. No firmware source,
protected bytes or absolute local asset paths are added to the source corpus.

### Isolated S2 Delivery

All eight optimized stripped 0538 products are rebuilt against the unchanged
Shared baseline. The final isolated AT x64/x86 runs both reach DOS 5 Setup
with the 50 ms Return release and remain alive for the full 180-second probe,
with no 301/303. Earlier KBC-only reset/resume and stop/start observations remain
applicable. The four INIs retain their frozen hashes. Documentation, six baseline
manifests and diff whitespace checks pass. Old 0535 EXEs are retired only after
these replacement builds, with recovery through Git history.

Source identity is the S2 implementation commit containing this table: KBC,
observer/regression and version recipe only, no Console source change.

| S2 artifact | SHA-256 |
| --- | --- |
| nxvm_model40_0_5_0538_x64.exe | 97DD1963EFE67769CBA9ADD28B139A6D854490880825C675DD10B09AC227F3A9 |
| nxvm_model40_0_5_0538_x86.exe | 365857C8EB6D7BE44B9274D57FBD77FC021E8CF71D528C852ED8230D9EE94942 |
| nxvm_default_0_5_0538_x64.exe | CFBF95B188A6B3B4D5F7519861F3D789B0D51AE11792517EFC38C953E0F4896B |
| nxvm_default_0_5_0538_x86.exe | 1370301E44059F2111A61C4A0E5F30D49A98E3B0146A9AA0CEE03E20821C0971 |
| nxvm_xt_0_5_0538_x64.exe | 04D60C08B389D7037894B7426EDA11D2E46F168313A7B19C3C944711FE941208 |
| nxvm_xt_0_5_0538_x86.exe | B77672292A742A16C663872D5D58680256E8EB647061E8E7B9916AF205259D06 |
| nxvm_at_0_5_0538_x64.exe | F9DFE3112CDB5E392C49A7F8BAF0E2FF7B273AEEE12C3D0F59DBCFECD057976A |
| nxvm_at_0_5_0538_x86.exe | 902039D17CFAD2A6B334005545674F574175DA7F55E47A8B805E21B2671B12B9 |
