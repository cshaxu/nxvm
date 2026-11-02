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
| TFT x64 | Open: distinguish absent-file seeded default from independently empty CMOS. | Open. | Open. |
| TFT x86 | Open: distinguish absent-file seeded default from independently empty CMOS. | Open. | Open. |

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
