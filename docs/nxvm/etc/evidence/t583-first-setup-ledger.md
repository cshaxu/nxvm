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
| DSTN x64 | Guest ERROR, date confirmation and DOS observed by factory/Machine probe in S2; deliberate date edits remain open. | Guest-produced CMOS saved by App callback and loaded by new probe process; DOS observed in S2. | Open: exercise the real executable/session and lifecycle in a later batch. |
| DSTN x86 | Guest ERROR, date confirmation and DOS observed by factory/Machine probe in S3; deliberate date edits remain open. | Guest-produced CMOS saved by App callback and loaded by new probe process; DOS observed in S3. | Open: real executable/session and lifecycle. |
| TFT x64 | Independently empty CMOS reached Setup/DOS; absent-file seeded default reached Setup and began DOS in S4. Deliberate date edits remain open. | Guest-produced empty/seeded CMOS loaded by new probe processes; DOS observed in S4. | Open. |
| TFT x86 | Independently empty CMOS reached Setup/DOS; seeded default reached Setup but not DOS within the first observation budget. Real-product first-run DOS remains open. | Guest-produced empty/seeded CMOS loaded by new probe processes; DOS observed in S4. | Open. |

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
