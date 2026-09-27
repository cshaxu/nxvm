# T538 S6 Final Deployed Qualification

## Boundary And Baseline

Owner admitted final verification on 2026-09-27. Baseline is c134b0982;
the eight deployed 0538 executable and four INI hashes match
[S5 delivery](t538-s5-input-reset-import.md). No executable input is changed,
so this verification does not manufacture replacement binary files. MyNES and
Shared are read-only. The five queue candidates are not admitted.

The complete batch is four products, two host widths and three fresh processes
per pair. The actual EXE loads its unchanged adjacent INI. Observations are
sequential to avoid native focus interference. No F1 or guest boot workaround
is injected. The observer sends `info`, then `start` with a 50ms Return release
delay, retaining the original intermittent AT reproducer condition.

The existing observer source is compiled into ignored `build/t538-s6/` with
`gcc -std=c11 -O2 -Isrc ... -lgdi32 -luser32`. An old temporary observer rejected
the new arguments before launching any product; those attempts are excluded.
Each valid observation has a 180-second guest budget and a 195-second external
watchdog. Observer exit zero or product STILL_ACTIVE is not boot success:
console text, a native capture and paused debugger evidence establish the
semantic terminal. Temporary logs and captures remain only for this verification
and immediate failure diagnosis; no protected asset is copied into the repository.

## Frozen Final Batch

| Product | Width | Fresh 1 | Fresh 2 | Fresh 3 |
| --- | --- | --- | --- | --- |
| IBM 5170 Model 339 | x64 | Installer | Installer | Installer |
| IBM 5170 Model 339 | x86 | Installer | Installer | Installer |
| DeskPro 386 Model 40 | x64 | Installer | Installer | Installer |
| DeskPro 386 Model 40 | x86 | Installer | Installer | Installer |
| Default PC/AT | x64 | DOS A prompt | DOS A prompt | DOS A prompt |
| Default PC/AT | x86 | Unclassified early pause | DOS A prompt | DOS A prompt |
| IBM 5160 Model 268 | x64 | 9C 301 / F1 error | Installer; pause unproved | 9C 301 / F1 error |
| IBM 5160 Model 268 | x86 | 9C 301 / F1 error | 9C 301 / F1 error | 9C 301 / F1 error |

Raw runs are named by executable basename plus `-r1` through `-r3`.
AT x64 first launch reached the DOS 5 Welcome to Setup screen at 34.359s;
the post-observation pause accepted existing Debug commands and read registers
and B800 text memory. No 30x error occurred in any of the six AT launches.

Default x86 run 1 logs `Machine paused.` at 147.250s, before the observer's
180s pause request. No matching native Window remains at capture time. The
process stays alive and Debug reads CS:IP=0B43:A627, but B800:00A0 is blank;
neither fact proves boot completion. Runs 2 and 3 visibly reach `A:\>`.
External interaction has been queried, not established. The first run remains
unclassified and is not replaced by its successful neighbors.

XT x64 run 1 remains at `9C 301` / `ERROR. (RESUME = "F1" KEY)` through 180s;
paused Debug reads F000:E842. No F1 is sent. This is an actual failed terminal,
not an image-preview ambiguity. The same terminal occurs in x64 run 3 and all
three x86 runs. XT x64 run 2 reaches the complete installer, but the observer's one-second pause wait
does not establish a monitor handoff: its following diagnostic text reaches the
guest installer instead. This run proves boot only, not pause/Debug usability.
The observer does not wait for a semantic pause acknowledgement before typing
Debug commands; this limitation must not be mistaken for successful diagnostics
or a proven product root cause. Media remain overlay-only.

The initial image previews for Model40 x64 run 2 and x86 runs 1/2 appeared to
show only `Micr`, so they were initially withheld. Byte-level verification then
found that all five completed Model40 BMPs and their PrintWindow counterparts
have identical SHA-256 (subsequently also matched by x86 run 3)
`7FBC8F8DBCD82B6416602987107EAAEFD8D76AF7F7CBBFE24043B614EC29E838`.
Each checked image has 6,083 light text pixels in the central body rectangle
x=50..589, y=50..249, and original-resolution reinspection shows the full
Welcome to Setup body and bottom key legend. The partial preview therefore
does not establish a partial image file or product failure. The provisional
failure inference is withdrawn on this evidence, not on a neighboring success.
Paused Debug also reads the complete DOS title from B800:00A0.

An independent Windows.Graphics.Capture inspection was attempted through the
Computer Use skill, but window access was not approved; no input or workaround
was performed. Original-resolution file inspection and actual BMP bytes are
used for the remaining image verification, not the inconsistent preview.

## Read-Only Failure Triage

The selected XT mapper emits scan-set-1 `1C | 80 = 9C` for a Return release
(`profiles/default_profile/keyboard_mapper.c`). Common's ordinary key path
forwards releases even when `forget_pressed` finds no preceding delivered press
(`src/common/session/control.c`). NXVM then routes the bytes through the XT
keyboard FIFO/PPI, rather than the AT KBC repaired in S2. These code facts make
a cooked-to-raw start-key tail a plausible explanation for `9C 301`; they do
not establish the exact observed event sequence. A repair needs explicit
handoff/source ownership and regression proof, not a BIOS-specific discard.

Default's early pause remains a separate unclassified observation. The trace
does not identify the producer of that pause request. Do not assert an x86
CPU fault, a crash, or external user interaction without further evidence.

These findings remain inside T538's outstanding deployed qualification, not
transferred to unrelated queued work. No repair is implemented by S6. Any
Shared change still requires owner review; a follow-up diagnostic/repair brief
must cover both widths and preserve the successful AT/Model40/default cases.

## Verification And Closure Review

- NXVM x64 incremental build: no executable work; 353/353 non-desktop cases
  pass (332 repository-only units and 21 static checks), 232.34s.
- NXVM x86 incremental build: no executable work; 353/353 non-desktop cases
  pass (332 repository-only units and 21 static checks), 277.41s.
- Established external integration: x64 20/20 pass, 10.99s; x86 20/20 pass,
  14.03s. No test removed or input substituted. Desktop units pass 3/3 per width,
  serially after the boot batch, in 1.37s / 1.35s. Full unit totals are 335/335
  per width, plus the 21 static checks.
- All six manifests and NXVM documentation structure pass.
- All eight deployed PE files have their expected host architecture and no
  compiler debug sections. EXE/INI and five configured media master hashes
  match the previously frozen identities before and after execution.
- All 11 selected external ROM/CMOS/font inputs match the pinned SHA-256 values
  in `cmake/nxvm/NxvmProductProfile.cmake`; neither firmware nor runtime-media
  substitutions explain the observations.
- Actual T production-diff review rechecked the single keyboard serial queue,
  unmatched-break exclusion, backing capacity versus viewport, retained clearing
  coverage, and source-local input reset/delivered-key ownership. No new source
  path is introduced by S6. Existing cooked-history rollback debt stays in TODO;
  this is not a claim of complete hardware timing or guest compatibility.

## Delivery And T-Gate Decision

All 24 observations completed their 180-second budgets; no watchdog termination
or spontaneous process exit occurred. There are 18 proven boot terminals, five
XT keyboard-error terminals and one unclassified default early pause. Only 17
boot-success rows also have the observer's confirmed paused Debug register
response; XT x64 run 2 does not. Prior lifecycle integration remains passing,
but does not erase this deployed handoff gap. No full-matrix input/resume claim
is made.

Executor self-review reconciles every row, log outcome, original-resolution
Window capture, suite count and immutable input against the packet. Source/test
changes are +0/-0; only NXVM task documentation changes. No new EXE is necessary
for a verification-only S. All owned observer/product processes have ended.
Ignored logs/captures are retained for immediate diagnosis of these unresolved
rows, not as shipped assets. Queue and MyNES/Shared trees are unchanged.

The verification is complete and its T-gate result is **failed**. This is not
T538 closure or acceptance of any failed row. The next action requires a bounded
diagnostic/repair admission covering the XT input-handoff batch and the separate
default early pause. Coordinator actual-change review accepts delivery
9d5e0ed0a and closes S6 as a completed negative verification, separately from
the rejected whole-task qualification. The archived packet and review are in
[task history](../../history/M5-T538-deployed-boot-pairs.md). T538 stays open;
no next S or repair is admitted.
