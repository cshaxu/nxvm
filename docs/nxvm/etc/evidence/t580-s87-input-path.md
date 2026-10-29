# T580 S87 — Product guest-input boundary

The real released TFT x64 product reached PC DOS driver output under the
actual RDP Window in S86. The previous machine-only diagnostic also reached
Personaware after keyboard input and four inking-pad events; its captured
post-key 640x475 frame asks for the upper-left pad corner.

An initial scratch-instrumented product run found the same recorded `INT 16h`
exception at 1475:0735 after 230,193,750 instructions and sent a Window Enter.
This is the last recorded exception, not proof that the live instruction was
still polling when the event arrived. The final PC sampling remained 0012F63E
while instructions continued. The final debug collection was interrupted by
the probe's monitor-input timing and is not accepted. Probe and runner logs
are SHA-256 `BDE358B0F947643EE570F861F1A0F84E35D20A44E86078FED234D8503169EFB3`
and `27123D172D1433ED1F41518791394D8DF6B726AFDD0241BB1EE8923FE39989D5`.

A second scratch product run enqueued four relative mouse press/release pairs
through `emulator_machine_enqueue_input`; all eight calls returned success.
However the Window frame hash remained
`8571B4E7F63BF5CB7BCBDC03802F53175E14B53AC0AB1DEBECD5890E58BB0B70`.
The experiment did not check whether inking was enabled before injection, and
its colour-only Setup detector incorrectly sent the second Enter while the
first ERROR page was still visible. A subsequent actual date confirmation and
HLT-observed F12 were sent normally. This run proves queue acceptance only;
it cannot prove input consumption or a product defect. It does not exercise
native Window mouse capture. Event log SHA-256 is
`AC64A9812A8ACEDE10CA616455CA6989B205197F33C384FB55EF8CDB39F7B17B`.
The isolated master/slave input image hashes remained unchanged.

The next bounded observation must distinguish the two Setup screens, wake only
a confirmed second HLT, and check the existing inking control register before
each input, then observe the resulting Guest frame. No production or Shared
code is modified to obtain the evidence.

The third observation distinguished the actual ERROR/date pages and sent F12
after the guest's second observed HLT. All eight calibration samples were
enqueued only after port 15E2h reported its enabled value; all enqueue calls
returned success. The output nevertheless remained on the calibration
screen. Review found this test used the wrong relative origin: the actual
PC110 adapter initializes the pen to (128,64), whereas the test's first
relative movement was (+5,+5), not the intended absolute (5,5). Therefore
the first point was (133,69), in the wrong quadrant. This is a test defect,
not evidence of a production calibration failure. The observer exited zero;
the event, runner and observer log SHA-256 values are respectively
`0576805B0B0F33F1EAB66A4B61047C964641D8C1D43ABF765F0B82D769BFBF5B`,
`B19E77B78FBF1ABE8B61FE8171A398A4C2DD23F860C85B880224135B50130C6B`, and
`B4BEC2A5B4A65F74E38C47FDC7FA77A72B71094B94333A6D496947C8D43FDD35`.
The task-owned disk images were unchanged. A new bounded run uses the correct
relative deltas from (128,64), checks enable before injection, and leaves
native Window mouse capture outside its claim.

The fourth scratch-instrumented product observation corrected the initial
relative coordinate using the PC110 adapter's (128,64) origin. The actual
Window showed the successive inking prompts and then the guest's Japanese
message that calibration data had been saved. All eight Emulator input-queue
operations returned success after the inking register reported enabled.
The captured final calibration message has SHA-256
`753A6118931BA080ED2C74E1B6FB2A818008B2B493D20F48EEA21C4273397C54`;
the observer and queue logs have SHA-256
`214B4FD4AC0BB77BEEBBD3CE882D69C9E1137EFAAB50C674BD0456B1EEE4811B` and
`21FEFE1DAAD0A81C7EF34251158AE4AD3A194B886458F4D3C17EB8477D3BB067`.
Both task-owned input image hashes were unchanged. This proves Guest
calibration progress through the Emulator input queue, not native Window
mouse capture or an interactive launcher. The probe deliberately resized its
owned Window to 464x348 to accommodate the changed RDP client size; it does
not qualify natural sizing. It exited after a further 10 million instructions
as the guest entered a 640x480 graphics frame, before the launcher was observed.
A subsequent longer observation is needed for the product screen.

## Product-path launcher and keyboard observation

The fifth isolated x64 product observation used the same production objects
with scratch-only instruction and enqueue instrumentation. After the normal
firmware and PC DOS path, four correctly positioned relative press/release
pairs were accepted through the Emulator Machine input queue with the inking
register enabled. The resulting actual RDP Window showed the Personaware
launcher (screenshot SHA-256
`E834392C80A445ACB985E0DE4800591EE1BF2BF8C688268AC15D634214B5E04F`).
Normal Tab and Enter make/break messages were then posted to this owned native
Window; a subsequent screenshot showed the World Clock application open
(`65F23D3DD2858F4B36F59BAA27180A1DDA9C830B4DBCBBB1548CAE6F5476DC1D`).
This establishes an observable response following those keyboard inputs in
the instrumented product path; it does not independently trace each key's
consumption or test native mouse capture. The Guest continued more than
80 million instructions after calibration without a reported Core error.
The background observer completed with exit 0; this is an observation, not
a registered integration test.

Observer, queue and runner logs have SHA-256 respectively
`8D929908ACD29D444F90189A2AA109E0AA55E056229058A7490BD66C7E64B9C0`,
`78238F0A838A3B57E5662FDEB245ED3DE2588A1087C18E4DB03318ED96339850` and
`C80ABEE113C6D5FD1FDE539804F948D42C894B3A61F8CCA2EB2160B34C80800D`.
The isolated input images retained their original hashes. This is TFT x64
instrumented evidence; the released TFT/DSTN EXEs, both host widths, native
mouse input, physical LCD and full T580 qualification still require their
respective proofs.

## Isolated persistence preparation

The first persistence-preparation observer stopped with exit 1 before F12:
the product had printed `Machine paused.` but the observer did not recognize
the expected monitor prompt within ten seconds. This is an observation
handshake failure, not proof of a Guest or product boot failure. Its log
SHA-256 is `AC7DD191A00E9EAA944EE7F2FE9C4EF3C574485FA91D3F14422A340F9B5089D1`;
both task-owned input disk hashes remained unchanged. A fresh scratch run
uses the existing run result to observe the first HLT and sends F12 through
the normal Window, without requiring that initial pause/debug handshake.
It retains an orderly-exit attempt and reports whether that attempt succeeds.
Only task-owned disk copies are opened for direct writes.

The corrected persistence-preparation run reached the Personaware launcher
using only task-owned direct-write disk copies. It paused and exited normally
with process code 0; the guest configuration file was 128 bytes, SHA-256
`608DF3ABC3CDD9BB3E35015D2D7AA9D8756567E080643B01E593FF99D1616034`.
Both input disk image hashes remained unchanged, so this is not evidence that
calibration was persisted to the disks. The observer log SHA-256 is
`E9E41FC63E782E5B9D079921F56CA5E40BA10D626D489BD6C18CE3E62BA63955`.
The subsequent fresh-process observation uses copies of these files and the
actual deployed TFT x64 EXE, verified SHA-256
`0718B3FC597B607FD4AD48016C84AA4972F60B7C4636E47B0BC2ECE3D38895AD`.
Whether the guest retains or requests calibration is an observed result to be
determined; no guest state is synthesized.
