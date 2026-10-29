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

## Published TFT x64 cross-process guest

The uninstrumented published TFT x64 EXE, byte-identical to its asset, was
started in an isolated directory using the CMOS and disk copies generated in
the preceding normal Guest run. After ordinary F12, it bypassed the earlier
ERROR/date pages, booted PC DOS and displayed the Personaware launcher without
repeating inking calibration. Normal Tab/Enter messages posted to its owned
Window were followed by the World Clock screen. The launcher and World Clock
screenshots have SHA-256
`BEFBE1F71614A12969C213EDDC30330D1C26909F3909E2FDB044B7877C5D301D` and
`1C29BDF7A55BDA61F22CFDB33BD62BBBFB4669B1D54B3905BB27695966B19F61`.
This establishes observed cross-process behavior and a visible keyboard
response for this published TFT x64 build, not native mouse, DSTN or x86
qualification. The observation was deliberately stopped after its successful
checkpoint rather than completing the probe's full capture budget; it is
not represented as a passing registered integration test.

The task-owned master disk copy changed during this direct-write run, from
`3CD463CC5D05D17F4709DA98F9E18F457BF7CCE279A78BDACC12ABC2DA49598A` to
`F27518A03119C26D0CA2721CBC14935E764E2F7D7F2DE91A2EC91CA12E04793B`; the
slave remained `5149DB391D13CFEAB330016FCF0EDBE6B0D379CBB66A3AED91DBF7684142D52E`.
The CMOS hash remained `608DF3ABC3CDD9BB3E35015D2D7AA9D8756567E080643B01E593FF99D1616034`.
No owner media or configuration was opened for writing. The interrupted
observer log hash is
`5345DA7055E1B6BB0AE04DC696A2F1194EED60030A37628BC9F6D4EB6EAC317F`.

## Published TFT x86 and derived-DSTN x64 guest observations

The byte-identical published TFT x86 EXE used fresh task-owned copies of the
TFT Guest-generated CMOS and media. It reached Personaware; ordinary Window
Tab/Enter was followed by World Clock. The corresponding screenshots have
SHA-256 `BEFBE1F71614A12969C213EDDC30330D1C26909F3909E2FDB044B7877C5D301D`
and `1FB12EA32D5D86A152820A1AD1127E9CFA6F0E864237D89F2D6E74F05C71AC0C`.
Its interrupted observer log hash is
`B3C6E116D25D0034687715DC4046F17F04229EA6A1ADBEA81548A09D3091F66F`.
The first attempted observation used an outdated probe and was stopped before
its checkpoint; it is not treated as a product result.

The byte-identical published derived-DSTN x64 EXE independently reached
Personaware, then World Clock after ordinary Window Tab/Enter. Its screenshots
have SHA-256 `BEFBE1F71614A12969C213EDDC30330D1C26909F3909E2FDB044B7877C5D301D`
and `15B4BB905FBA96B517A6AD6750DCA31C0CA339EB1E035EC36E6750E964B22AAF`;
the interrupted observer log is
`E67D6CD51E402790741A0652A268FD49039F965EE51A4742DAC46A8AB26FE50F`.
This DSTN experiment copied the TFT Guest-generated CMOS to the DSTN-specific
filename and therefore does not prove a pristine DSTN cold setup. The derived
firmware is not an independently recovered factory-original BIOS. Both runs
were deliberately stopped after the successful checkpoint; neither is a
complete-budget registered integration result. Their isolated direct-write
master disk copies changed to
`F27518A03119C26D0CA2721CBC14935E764E2F7D7F2DE91A2EC91CA12E04793B`;
slave copies remained unchanged. Owner assets and user media were not written.

The remaining byte-identical published derived-DSTN x86 EXE used fresh
task-owned copies of the TFT Guest-generated CMOS and media. It reached
Personaware; ordinary Window Tab/Enter was followed by World Clock. The
launcher and World Clock screenshots have SHA-256
`53E87F766E4C0404AA23EE0C1948940B202FC4DB4772299CB04F6124C997E76E` and
`9CE8D639ED854BACBD7BB1757DFE82FF00D71A301AC4B1456BABCFEE4F1408C8`.
The observer was deliberately stopped after the checkpoint, not counted as
a full-budget registered integration pass. The task-owned direct-write master
image changed to `F27518A03119C26D0CA2721CBC14935E764E2F7D7F2DE91A2EC91CA12E04793B`;
the slave retained `5149DB391D13CFEAB330016FCF0EDBE6B0D379CBB66A3AED91DBF7684142D52E`.
The interrupted observer log has SHA-256
`E6953B4F9E38130C8E024C972DEAE72E7B4330683BFEAB7ACE1518AC0AC90D6A`.
The copied CMOS originated in TFT, so this is not a pristine DSTN cold-boot
or independent physical-panel qualification. No owner medium was written.

The existing Win32 capture contract mock and PC110 machine-construction and
inking unit cases passed 3/3 on each architecture. Their output SHA-256 values
are `C36E2231E9B9813C8A189A7D5E563F8C5B50041A420882937DE867DC104E26C2`
(x64) and `EA9488A041B1A1328AECA9730D8BD7F341AAB2A0CDA06473215C95C8A9FD5DCE`
(x86). These separately cover the Lib capture fixture and PC110 driver/device
contract; they do not establish real native mouse capture end to end.
