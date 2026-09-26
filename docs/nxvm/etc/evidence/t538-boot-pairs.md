# T538 Boot-Pair Convergence Ledger

Frozen baseline: a2418ae57, four owner INIs and eight 0535 deployed EXEs.
Expected checkpoint is a real DOS prompt/date input or DOS installer. A stopped
process, unchanged screen, or expired time budget is not success.

| Case | Media / presentation | x64 baseline | x86 baseline | Repair / proof |
| --- | --- | --- | --- | --- |
| XT model 268 | DOS 5 360KB; console | Installer, about 27 seconds | Installer, about 33 seconds | Isolated real-process Console captures; paused debugger remains usable |
| AT model 339 | DOS 5 1.2MB; console | Installer, about 33 seconds | Installer, about 53 seconds | Three additional cold starts per width reached installer; owner reports intermittent 303 on both widths, unresolved |
| DeskPro Model 40 | DOS 5 1.2MB + blank 40MB; window | Installer at 180-second capture | Partial title at 180/300 seconds; native-paint check pending | Paused debug works at F000:F61B; no process exit. Do not label partial title boot success |
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

`nxvm-deployed-boot-probe EXE DIRECTORY SECONDS LOG BMP [START_RETURN_RELEASE_MS]` launches the actual
EXE in a new Console, leaves its adjacent INI intact, sends monitor commands,
reopens CONOUT$ after buffer handoff, and records process exit and Console
content. A Window capture briefly shows that surface without activation and
copies its client pixels. Its return code means observation completed, never
boot acceptance. The coordinator reads the captured semantic checkpoint.
The final observer also records `info` before `start` and reads registers via
the existing paused debugger after the boot observation, not through a new API.
The optional final argument delays only the start Return key release (0--1000
ms), to test native input handoff without changing firmware, INI or guest state.

## Baseline Disposition And Next Batch

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
