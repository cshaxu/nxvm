# T583 first-configuration and restart coverage ledger

T583 covers two firmware variants (derived DSTN and TFT), each on x86 and
x64. The DSTN firmware is a derived image, not a verified factory-original
dump. Tests must use isolated media and independently initialized CMOS.

For every variant and host width, record real guest and product evidence for:

1. Empty-CMOS firmware error and transition into Setup.
2. Setup navigation/input and configuration/date-time acceptance.
3. CMOS persistence, process termination and independent restart.
4. Boot to the selected DOS medium after the saved configuration is restored.
5. Stop/start and reset/resume boundaries without destroying retained data.

Each cell is either directly proven, not applicable with a specific reason,
or open with a named receiver and evidence. Time/instruction budgets are
containment, not success. Final qualification requires the applicable cells
and regressions on both widths; later desktop interaction and broader device
coverage remain in their own queued tasks.

| Variant and host | Empty/seeded first Setup | Saved-state new-process DOS boot | Full App entry/session path |
| --- | --- | --- | --- |
| DSTN x64 | Guest ERROR, date confirmation and DOS observed by factory/Machine probe in S2 and actual EXE in S6; deliberate date edit and retention verified in S7. | Guest-produced CMOS saved and loaded by a new actual EXE process; DOS observed in S6 and S7. | Real App entry, native window-message Setup input, normal Session teardown and restart observed on an isolated desktop; interactive RDP remains open. |
| DSTN x86 | Guest ERROR, Setup and DOS observed by the actual EXE in S6; deliberate date/time edit and retention verified in S9. | Guest-produced CMOS saved and loaded by a new actual EXE process; DOS observed in S6 and S9. | Real App entry, native window-message Setup input, normal Session teardown and restart observed on an isolated desktop; interactive RDP remains open. |
| TFT x64 | Independently empty and absent-file seeded CMOS reached Setup/DOS in the actual EXE. Deliberate date/time edits and cross-process retention passed in S7. | Guest-produced empty/seeded CMOS loaded by new actual EXE processes; DOS observed in S6–S7. | Real App entry, native window-message Setup input, normal Session teardown and restart observed on an isolated desktop; interactive RDP remains open. |
| TFT x86 | Independently empty and absent-file seeded CMOS reached Setup/DOS in the actual EXE. Deliberate date/time edit and retention verified in S9. | Guest-produced empty/seeded CMOS saved and loaded by new actual EXE processes; DOS observed in S6–S9. | Real App entry, native window-message Setup input, normal Session teardown and restart observed on an isolated desktop; interactive RDP remains open. |

## S9: x86 edited date/time

Both actual x86 EXEs displayed the firmware date/time screen, accepted native
input changing it to `1995-01-01 12:00` and then proceeded to a new POST frame.
The DSTN and TFT native BMP hashes for those edited screens are respectively
`64158f39f4b6613d7349954ac5d36cdd3a17dee33144923f5c7c46f5e4853738` and
`51f9646c8bf776faa6eb2c3a0e54c767b53ac89b9a541a7b583ebbec5f7486e7`.
Both processes paused and exited normally. Their independently saved 128-byte
CMOS images contain year `95h` and hour `12h`. First-run log SHA-256 values are
DSTN `e7b5bc00ed91bcb64fc736640ddd7dc48e304900e89c477f2883889d01a4be8a`
and TFT `44644cd28b5b6cdc9eede00d45911fadda0074e7740443c1ae82b13c28ec259c`.
Independent new processes reached DOS without Setup input, paused and exited
normally. Their saved CMOS images still contain year `95h` and hour `12h`.
Restart log SHA-256 values are DSTN
`e4e5b55828ef7da828c843923b7f27165bacddb93f55837643a7072b51433fad`
and TFT `0a2192f881ac2cbe7ca0d2938dfa72830c90708f4d93c511f546fa2219c6c4e1`.
The x64 complete repository unit suite passed 712/712; log SHA-256 is
`2d247c725a388a175b86266c5267339b1f308ee6d897cac43fa610472e219a9a`.
The PC110 x64 external-firmware integration suite passed 9/9; its log SHA-256
is `1dbd16820973445493dcba2a327b31658911534340ef837db8407e470197f4de`.
The corresponding x86 suite passed 9/9; its log SHA-256 is
`2e231f8154237438aee04879cb028a5d8f0cf7e91ff0dc1ae97d2f02aabfd3e5`.

## S8: in-process stop/start and reset/resume

The actual EXEs, both variants and both host widths, each started from their
own guest-saved CMOS on isolated desktops. After the DOS calibration frame the
observer requested a real pause, received `Machine stopped.` after `stop`,
and issued `start`. It required a new non-DOS frame before accepting a second
DOS frame. It then paused, received `Machine reset and paused.`, sent `resume`,
and again required a changed frame followed by DOS. One captured DSTN x64
POST frame visibly shows `20096 KB OK` and has SHA-256
`94c5272ffead1ebf7b989d7e8fb0bcce5019dbe8ae55be0819dfdc9b4308a515`.
That exact POST frame was independently present after stop/start and after
reset/resume in each of the four cases before their respective DOS frames.

Both x64 cases and TFT x86 then paused and exited normally. First-run logs are
DSTN x64 `cd2c10ed162179e44d81a0c8c5ba9dc8bf1500e77992180fad5aa9a2bf21fcb0`,
TFT x64 `956c06f3769ef495d6d9b62b663046c5d49c055300c9f3098a58cccbcfbde56d`,
and TFT x86 `0f22d5b2af127dae040361ac6489bbbe38ff4810c91c7d4bd74d87f1c3db2e601`.
The DSTN x86 run reached both restart DOS checkpoints, but its observer exited
1 after its final pause; its exit check did not distinguish a 10-second wait
timeout from a nonzero process exit. This is not counted as a normal teardown.
The log SHA-256 is
`dada025b2f7dfbaff07bf90a899b34e9d5caa14108b4a0fa61d7d24e2e177d14`.
An independent repeat with a diagnostic exit result and bounded 60-second wait
again reached DOS after both restart paths and exited normally: wait returned
`WAIT_OBJECT_0`, the exit code was available and was zero. Its log SHA-256 is
`e77805ffe21a83d8d76ec32a860bcaa604fbbd12e724449372f899ddf67a787f`.
The earlier failure's cause remains unknown because its observer did not log
the individual exit-check results. All four private input-media hashes match their initial values;
the saved CMOS configuration bytes from offset 0Eh onward also match their
respective initial guest-produced images. The unsuccessful DSTN x86 teardown
does not prove that its final RTC state was saved.

The x86 full repository unit suite passed 712/712; log SHA-256 is
`ee631116e749eb262bdeabb3ba1a8b389e28bb6815344694d4f78f05f78258bc`.

## S6: isolated native product observation in progress

`CreateDesktopW` returned access denied inside the restricted execution
environment and succeeded on the host without switching desktops. A task-owned
observer uses a separate invisible desktop, the actual released EXE and private
INI/disk copies. It captures real KVM Window client images and posts native
Enter make/break messages only after the relevant frame is inspected. It pauses
and exits through the real monitor; it does not foreground the user's desktop.

The first fresh-DSTN x64 EXE showed the ERROR page, changed to the date page
after the first Window Enter, restarted POST after the second, and reached
the PC DOS inking calibration screen. Its monitor pause and normal exit both
succeeded, leaving a 128-byte saved DSTN CMOS. A new actual EXE process loaded
that file and reached the same DOS screen without Setup input, then paused and
exited normally. Both private disk hashes remained unchanged. The first and
restart logs are SHA-256
`9d9f8bdd959fbdf9fa9a76415c9e6c32918cd38e41a0be6c6af186913669b5af` and
`5eecadea6c418c3f4f32975ea1f0bcc4ac58cc299ba7402591daba3f2127b37d`.
ERROR/date/DOS native BMP hashes are respectively
`3c1886f620a0cdb598db8bfbe90683a63aea2f75d82c4610da77fbecbe81b717`,
`a0ee33ecb409e64f00b79cc144f4c594d828b55dec557a3b4c49e4ecaa039955`, and
`07987e8e9b2aedabd72e0caf5bd24cf618a55a11f716a4e0ada076bd5d40acf3`.
The restart reached the identical DOS BMP without Window key injection. This
is real native Window messaging on an isolated desktop, not interactive RDP.
The TFT x64 EXE likewise selected TFT, reached ERROR and the `2000-01-01`
date page, accepted two native Enter pairs, reached DOS, and exited 0 through
the monitor. A new actual EXE process selected TFT and reached the identical
DOS frame without Setup input, then exited 0. Both private media hashes stayed
unchanged. TFT first/restart logs SHA-256 are
`dd4de7c7819f8846353fb1168ff6d0921121252e0bf722c3c564c62ac679d727` and
`e6711c0f1c66cfe2db857c1d04dd3e84469f6febf634f60be8157dc2e9674856`.
Both x86 actual EXEs likewise completed first Setup and reached the identical
DOS frame. Each exited normally, saved its own CMOS and reached DOS again in a
new process without Setup input; the new processes also exited normally.
First/restart log SHA-256 values for DSTN are
`6676007e8e4b284b7c4e4c05dec076bbc8360a4f18e5f6f1509de5e2cf16f0cb` and
`86045d1d2cddd2aba6ca681eb2100ad01d2ba603638a3b41e1a6d1da088c73d4`;
for TFT they are
`3c6c41d20382909d18b3919b0e3b9e3991911f2dbc230495ebcae06ebb510661` and
`a597b42d8d35e6afedf8d38380daa2e111b599ece0ae81212b93c6ee22556ce0`.
Earlier x86 attempts reached DOS just before the 600-second observer limit,
but that observer terminated them before normal exit; those attempts were not
counted as passes. Frame-driven input and termination were used for the
successful independent repeats, not a shorter guest execution budget.

A separate fresh DSTN x64 experiment selected the firmware date field and
demonstrated that native digit input changes its displayed value. The experiment
did not verify acceptance or persistence of a valid edited date; these remain
open. Its edited-screen BMP SHA-256 is
`da127a52727fc956c221d13e1d08df8c9ea3bec3b9032273d64dff38f1f77444`,
and its normal-exit observer log SHA-256 is
`03909d0dd25c3d163039681c77c8fdf548abc739dda07d6948c091a2d5e350ce`.
The native EXE TFT runs use the normal absent-file seeded default; the actual
EXE route with a truly zero-filled TFT CMOS remains unverified.
The x64 full repository unit suite passed 712/712; log SHA-256
`7b26d641258aa6a375dacb901a7663773b76dd857f1a48063ec30bc863be9c92`.

## S7: deliberate date edit and empty-TFT product route

A fresh DSTN x64 real executable reached the firmware date screen on its own
isolated desktop. Native digit input changed the displayed date from
`1994-01-01` to `1995-01-01`; the latter BMP SHA-256 is
`19331f113f4ef2959f974e15b3f80d53a390d65bdea8ec5636f543652d6c0d38`.
After Enter, the guest reached the DOS calibration frame and the real product
paused and exited normally. Its 128-byte saved CMOS contains RTC year `95h`;
the first-run log SHA-256 is
`431d65878dcda7460355fc60b6d5cfefa0041d201862b2908ec68d7c3d50327c`.
A new actual EXE process then booted to the same DOS frame without Setup input,
paused and exited normally. The resulting CMOS still contains year `95h`;
the restart log SHA-256 is
`f15c10800d0b97e9dc2a4b6380f06bc3ea1b90798a08f8732d95a32296ba181d`.
This is a native-message test on an isolated desktop, not interactive RDP input;
the deliberate edit/restart here covers DSTN x64 only.

The independent all-zero TFT CMOS x64 real-product run also traversed ERROR,
Setup and DOS, then paused and exited normally. Its first-run log SHA-256 is
`b7ee839cfc679abfbc79bbbe8bda5556ba0e75a278f04ba33ca15dfbd5d1294c`;
its resulting guest CMOS SHA-256 is
`344bc634dee98f58818f696d1fa7743fd7d0c917e0f9b33d9a84d0a037681628`.
The independent x64 restart also reached DOS without Setup input and exited
normally; its log SHA-256 is
`69560c9afb7c2e7f148dce221b345b57ace9819be9b801f64b5c1a761cecc2ae`.
The x86 first run likewise reached DOS and exited normally; its log SHA-256 is
`267c23f6d8f1b53f0ca9fbca3be39801d3c019edb9608cdca18557e311aa13b9`.
Its independent restart reached DOS without Setup input and exited normally;
the restart log SHA-256 is
`2d2fd7a9769d9aa2bea5130dd45458a15dd474c990762478c8b512807621de6a`.
The x64 full repository unit suite
passed 712/712; its log SHA-256 is
`6f59905ebe11954f88cc5c9d65db64fa5ee7e9d2908b8b71cde6c5b6409f853e`.

A separate TFT x64 seeded case changed the date to `1995-01-01` and the time
from `00:00:15` to `12:00:15` in the actual firmware Setup screen. The native
BMP SHA-256 is
`51f9646c8bf776faa6eb2c3a0e54c767b53ac89b9a541a7b583ebbec5f7486e7`.
After Enter the guest reached DOS and the product exited normally. Its saved
CMOS contains year `95h` and hour `12h`; the first-run log SHA-256 is
`4021ec0ac039b76c3c007127a06207a1864e3a203e4d3c4cf1575de9018708bf`.
A new product process reached the same DOS frame without Setup input and
exited normally; its saved CMOS retained year `95h` and hour `12h`. Its log
SHA-256 is
`07354fea77585762fc11b6d79253e035832925a075d069a0f17dcbaffbb50177`.

An initial S7 observer used the repository root as its own working directory,
although the guest EXE and media were isolated correctly. Its automatic
frame detector therefore did not see those screenshots. The observer was
normally stopped, its task-owned screenshots moved into the task directory,
and a new fresh case started with the observer working directory corrected.
The interrupted observation is not counted as Setup, boot or persistence proof.

## S5: real executable lifecycle and CMOS persistence

On both host widths, a task-owned extension of the existing hidden-console
process fixture copied the actual released MyPC110 EXE and independently
guest-produced 128-byte CMOS images into isolated directories. For DSTN and
TFT separately it selected the variant, issued `start`, `pause`, `debug`,
`stop` and `exit`, and verified every monitor marker and zero process exit.
Through the paused Debug port path it changed the RTC seconds register to
`42h`. The selected variant's resulting file had that value, while every
configuration byte from offset 0Eh onward matched the guest-produced input.
The other variant's distinct, guest-produced `2000-01-01` CMOS remained
byte-for-byte identical. The four x64/x86-by-variant cases passed; verification
transcript SHA-256 is
`049fed5bf010ac2367daa23793f1d623ee2a0652b4bc90af5945b2d77b0df698`.

The x64 executable was SHA-256
`11f05d0087e0f0ce4f90c7a9217294801298355b285d13e718e497a93b306d60`;
the x86 executable was
`63876fe16fa10b1ddd0349dbf490b031c79efa7adbdc39f12b64db6153a42875`.
The temporary Debug write is a controlled persistence witness, not a claim
that the guest Setup UI changed the date. These quick lifecycle cases do not
boot DOS or test Window input. The complete x64 unit suite passed 712/712;
log SHA-256
`a25b1a7b5c1f5e1c6ef5cb845a02c51a97c28a2b3b8bede838f48fc1736e287b`.

## S4: TFT independently empty CMOS in progress

On both host widths, the TFT probe began from a task-owned 128-byte all-zero
CMOS image, not the embedded configured seed. It matched the same ERROR,
date and DOS frame checkpoints as the DSTN runs, sending two Enter pairs only
after the matching screens. Both first-run processes exited 0 and saved
guest-produced TFT CMOS via the existing App callback, with SHA-256
`344bc634dee98f58818f696d1fa7743fd7d0c917e0f9b33d9a84d0a037681628`.
Both sets of copied disk hashes remained unchanged. First-run logs: x64
`2da08f453e0794f2116d662c265987a26318799daec61bdb6ad4126e7e07366e`,
x86 `c9620ca687a2c5eff5aec80d783a0a196ebb5d53a5b31a073b327ac5993518d0`.
Independent second processes on both widths loaded their saved TFT CMOS and
reached the same DOS frame without Setup input, then exited 0. Media copies
remained unchanged. Restart logs: x64
`8b2d4d75a7e98ee89a861e23b3ff4b18bd1ef928684d3b8190af801db71bb09e`,
x86 `f4b3bd7241e61066e09bfe498a07d94ad9c85f4f5e26a2fa68a6f72546ed18b0`.
The probe's initial RTC print occurs before the switch from default DSTN to
TFT, so it cannot prove the TFT RTC was restored. A separate short rerun on
each width loaded a saved TFT image with seconds `39`, switched to TFT, then
shut down and captured seconds `39` through the existing safe teardown path.
An earlier scratch attempt to read RTC while the executor was still bound
exited 1, as `x86_nxvm_machine_capture_rtc()` explicitly rejects that state;
it is not a product failure or a successful observation. Deliberate date edits
remain open. No native Window or full App/session claim is made.
The x64 complete unit suite passed
712/712; log SHA-256
`1cb8958742fb53cc35812c0d41cf1d04822d940f73929a4bb2338c715c7bcf70`.

The first separate absent-file seeded-default observations on both widths
displayed the same ERROR 00173/00163 screen. Their driver intentionally
classified that frame as requiring Setup and requested a normal shutdown;
driver exit 2 reflects that classification, while both probes exited 0.
These observations do not establish a defect. Two fresh seeded-default runs
are continuing through actual Setup rather than assuming a direct DOS boot.
Both seeded runs reached the date page, but its values were
`2000-01-01 00:00:15`, unlike the all-zero image's 1994 page. The identical
x64/x86 frame SHA-256 was
`070bfb1bfa058b3b10f1208ba9623fe3c2d25a4e000c0243bd96eb93e2041ca8`.
After inspecting that frame, one Enter pair was sent to each actual Machine;
the original hash-specific driver did not mistake it for the 1994 page.
The x64 first-run probe subsequently showed `Starting PC DOS...`; the x86
probe was still in POST when its 96-sample observation budget ended. Both
exited 0 and saved their guest-produced TFT CMOS. The hash-specific drivers
later exited 1 because their original 1994 date checkpoint did not match;
this is a diagnostic-harness limitation, not a guest failure. The two saved
CMOS images were copied unchanged into isolated new-process restart runs.
Initial seeded logs: x64
`d036cc4d01c38f18cf61edb5826864abbba1f61b3220290546aa086fa6a6b4ec`,
x86 `4aa31e93378783bb03b13d6a4e8d533bc7f5859110a2a0e7a003aca47dcff995`.
Full first-run logs: x64
`a62fe1162366a35123f471177633cf2915c3babf0c11729d1ad65d3150472370`,
x86 `f8a847566e936454ba78afd3c932ae1db1ba9f961865d571eedbcba9b1e769de`.
Both separate no-input restart processes subsequently reached the DOS
calibration frame and exited 0. Their saved CMOS files retained the date
`2000-01-01`, and RTC seconds advanced from `35` to `54` on x64 and `32`
to `51` on x86. Private disk hashes remained unchanged. Restart logs: x64
`43dcc7fda611d8c827456beb22482974cb48debb79420ab597978fb8ee4c924b`,
x86 `b1b19769ffae1646d70956ecbd49ed8c7fb0ac82c3cc2064a04cf2b12810d48d`.
The first x86 seeded process did not reach DOS within its sampling budget;
its first-run DOS checkpoint stays open for the real-product receiver.

## Initial source evidence

T580 S90 observed derived DSTN with independently empty CMOS reaching the
`00173 / 00163` ERROR screen, without completing Setup or reaching DOS.
Configured CMOS reached the launcher on x64. T580 S87 exercised TFT Setup
and calibration with instrumented input, but does not by itself establish
the entire empty-CMOS process/restart sequence for both variants and widths.
All ledger cells remain open pending current-source reconciliation.

## S1 reconciliation: 5715cd392

The current `mypc110.integration/{tft,dstn}-cmos-product` tests use an
isolated copied executable and temporary INI. Both tests passed on x64 and
x86 (four total). They cover product lifecycle commands, debug-register
CMOS/RTC writes, file re-opening and per-variant isolation. They do not enter
the firmware Setup UI, save configuration through Setup, or boot DOS, so they
cannot close any complete first-configuration row above. Test transcript
SHA-256: `1997152ED5BAF028BDB00E883E53B8292CE2AD382C814F87DACAF3AF8BC58B2B`.

The registered `real-firmware-post-integration` links the TFT firmware only;
its registered tests check FDC reset and cold/resume firmware handoffs, not
the complete Setup/restart/DOS sequence. Its additional command-line Setup
diagnostics use instruction checkpoints and a reference frame; their direct
Machine path is not an independent product or DSTN result. Reuse them as
diagnostics, not as a replacement for product-path proof.

Product initialization supplies the embedded TFT CMOS seed when its CMOS
file is absent. DSTN has no corresponding embedded seed. Therefore “no saved
file” and “independently empty CMOS” are different initial conditions for TFT;
both must be tracked explicitly, not treated as interchangeable or silently
manufactured. Current evidence does not show whether a truly empty TFT CMOS
can complete Setup, nor whether a fresh independently configured DSTN CMOS
survives a new process and boots DOS. All five completion rows remain open.

S1 additionally runs one x64 no-input DSTN observation through the current
Product factory, Emulator Machine and published frames. It uses a dedicated
`build/t583-s1/fresh-dstn` directory, private copies of both disks, no saved
CMOS file, and a 32-sample/approximately 320-second observation budget. It
does not exercise a native Window or claim Setup interaction. The isolated
master/slave input SHA-256 values are respectively
`3cd463cc5d05d17f4709da98f9e18f457bf7cce279a78bdacc12abc2da49598a` and
`5149db391d13cfeab330016fcf0edbe6b0d379cbb66a3aed91dbf7684142d52e`.
At sample 12 the observation switched to a 640x480 graphical frame; sample
14 shows ERROR 00173/00163 with OK and Cancel. Its frame SHA-256 is
`b9ea224dfeb96227bc89cf2b1fd9c5147324f85767c21b9fe8259dbad9159b96`,
matching the earlier T580 observation. This is the published Emulator frame,
not native Window/RDP evidence; reaching a prompt without providing Setup
input does not prove a failure or complete the guest configuration.
The bounded observer completed with exit code 0 and no input after sample 31;
both copied disk hashes remained unchanged. Its log SHA-256 is
`b8a61e66bb394d4a1d050c235ef9d8bfe1b4a3842e97c9aa72540b79a25d252f`.
The observed 640x480 ERROR screen remained present in the later samples.

The x64 full repository unit run passed 712/712; its output SHA-256 is
`a688c48ca13b6165c2492614f3fbb42fb5c8dbebdc953c8aa19402b5daE971a1`.

## Next batch

Build on the existing product-factory and published-frame probe with
explicit, observed input delivery and independent task-owned CMOS/media.
First characterize the derived-DSTN empty-CMOS ERROR-to-Setup interaction,
save, restart and selected DOS-media path on x64. Record the actual first
failed or unverified boundary without treating a time budget or accepted
input queue event as success. Repeat the same semantic checkpoints on x86
and the TFT seeded/empty cases after the route is understood. Desktop/native
input remains with the next queued interaction receiver; this probe does not
claim that receiver passes.

## S2: independently initialized DSTN interaction in progress

On x64, an isolated fresh-DSTN Product-factory/Machine probe reached the same
ERROR frame (`b9ea224d…ad9159b96`). After a single Enter make/break was
accepted through the Emulator input queue, the published frame changed to
the firmware date/time page (1994-01-01 00:00:00); its PPM SHA-256 was
`0644016d2ee985038efc43e069a260768de91ac98231a7891a5463ebd45f8edd`.
A second Enter was accepted, after which the firmware returned to text POST
and displayed `20096 KB OK`. These observations demonstrate guest response,
not native Window input or successful persistence. The first probe does not
invoke the Product's configuration-save callback at teardown; a separate
isolated round-trip probe must do so after shutdown and unbinding, then start
a new process using its own saved CMOS. No configured seed was fabricated.
By sample 87 its published frame showed `Starting PC DOS...`, loaded DOS
drivers, and the inking-pad calibration prompt. The frame SHA-256 is
`75a43c909f79609524cbd7a1c1b938ff39023ea28c116c275b52738e4b8f2b29`.
This proves the first fresh-DSTN x64 guest reached the selected DOS media via
the Product-factory/Machine path; the independent saved-CMOS restart and
native product interaction are still open.
The probe exited normally and both private input disk hashes stayed unchanged;
its log SHA-256 is
`d26fdec531bd1472f990b539ebbf9390754679e500ac9bb71bc58418ea2f434c`.

The S2 x64 complete repository unit suite passed 712/712; log SHA-256
`0c3bf326ce8792df344be74854d79cf32424821e9caa63e418a0bac3a1dbf46e`.

An independent fresh-DSTN x64 repeat matched the ERROR, date and DOS frames
before sending the next input. It shut down and unbound the Emulator Machine,
then invoked the same App `save_configuration` callback used by Product
teardown. The process exited 0 and wrote a 128-byte DSTN CMOS image, SHA-256
`344bc634dee98f58818f696d1fa7743fd7d0c917e0f9b33d9a84d0a037681628`.
Both copied media retained their original hashes. This proves the App callback
can save the guest-produced state in the probe, not yet that a real product
process automatically performs that teardown. First-run log SHA-256:
`ce7cd7b4e602ecc701ad8938ba36058221bc8af0f8a5577bbd83140c14415321`.

A separate new x64 process loaded that saved DSTN file and received no Setup
key events. It reached the identical PC DOS calibration frame, then shut down
normally through the same probe teardown. Its initial RTC seconds were `20`,
matching the first process's final `20`; its final seconds were `40`. The
copied media hashes were unchanged. This directly supports the x64 DSTN
Product-factory/Machine save-and-restart route; it does not establish the
native application entry/session teardown, native Window input, other host
width or TFT cases. Restart log SHA-256:
`b9bb20105211027316867cc7b9f5238612d331a5e0a1658bde6e4ec420f3da87`.

## S3: derived-DSTN x86 machine-path observation

The x86 Product-factory/Machine probe is compiled against the current x86
libraries and uses independently copied disks and an absent saved CMOS. It
checked the same known ERROR, date and DOS frame hashes before delivering
the next input or requesting shutdown. A saved-CMOS new-process run followed
only after guest Setup and the existing configuration-save callback succeeded.
The probe does not exercise native App entry or Window events.
The fresh x86 run matched the x64 ERROR, date and DOS frame hashes, accepted
two Enter make/break pairs, exited 0, and saved a 128-byte guest-produced
DSTN CMOS with SHA-256
`344bc634dee98f58818f696d1fa7743fd7d0c917e0f9b33d9a84d0a037681628`.
Both copied media hashes remained unchanged. First-run log SHA-256:
`47073221a637fc4e07ca5bdf5c68f7a888c186272a9473fd043cfaf87958477e`.
A second x86 process loaded that CMOS and reached the identical DOS frame
without Setup input. Initial RTC seconds matched the first run's final `20`;
the second process exited 0 at seconds `39`. Both disk hashes stayed
unchanged. Its log SHA-256 is
`1629f7828beccabeffb7e923fac9972189499377d1cf06ea21ea3a4d5d95589b`.
The complete x86 unit
suite passed 712/712; log SHA-256
`e3fab39ae57fe6b492182269e7b28a7d9ea1af2ca858a377b120055b843b16b0`.
