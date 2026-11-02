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
