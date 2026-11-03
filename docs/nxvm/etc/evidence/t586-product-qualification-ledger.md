# M5 T586 — PC110 current-product qualification ledger

This ledger consumes the owner request to finish the first four PC110 tasks
and the [final qualification proposal](../../history/M5-T586-pc110-final-qualification-proposal.md).
It does not equate an L1/L2 device model with physical PC110 fidelity.
The current product is one selectable DSTN/TFT build in each host width;
the DSTN firmware is derived, not a verified factory-original image.

| Requirement | Earlier evidence | Current disposition and next receiver |
| --- | --- | --- |
| Build and artifact identity | T585 S21 built all 14 EXEs and verified PE width; PC110 deployed files matched both builds. | S1 and S3 verified the current paired PC110 artifacts and unchanged production after S22. |
| Firmware cold and resume handoff | T585 S21 final integrations reached real firmware cold and resume handoff on both widths. | S22 extended the existing TFT cold test with actual BIOS APM; both widths pass. Both derived DSTN probes also exercised that APM route. |
| First Setup, CMOS, DOS, stop/start and reset/resume | T583 exercised both variants and widths with private media and guest-created CMOS. | S2 repeated both restart paths on all four current-product cells, with a fresh POST and DOS screen after each. First-Setup and CMOS creation remain the earlier T583 observations. |
| Personaware and guest input | T584 reached World Clock and exercised input and pause/resume on both variants and widths. Four-corner calibration used the Machine path. | S1 repeated current-product DOS-to-Personaware and pause/resume on all four cells with isolated desktops and disposable copies. Posted native messages are not physical input. |
| PC Card, audio, serial and power | [T585 device ledger](t585-s23-device-coverage.md) names real receivers and tests. | Keep its explicit limits: no external I/O card/host serial, dynamic battery model, full ES488/OPL2 synthesis or panel fidelity claim. |
| Native Window/Console and input | T584 tested native Window and both Console control policies with an isolated desktop. | Current posted-message checks are not a real host mouse-capture, physical chord or interactive RDP pass. Record any unavailable checks explicitly. |
| Complete repository tests | Final-source S21 units passed 714/714 on both widths; registered integrations 47/48 each. The remaining failure was default native MIDI creation on the isolated desktop, under the owner's device-availability exception. | S22 changed only the existing TFT cold-boot integration, passed it on both widths, and ran the other variant through a separate diagnostic. No claim that unavailable MIDI or PCM endpoints passed. |
| Ownership and release | T585 S21 separated Shared and NXVM commits and shipped the PC artifacts. S22 changed only tests. | Preserve user INI/CMOS/media and unrelated App artifacts; inspect final status and record commits. |

The current-artifact matrix should use private copies of the known T584 media
and CMOS, with current `assets/mypc110` executables. A task-owned native
observer on an isolated, non-visible desktop may post the same keyboard
messages as T584. Bound each run, record progress and exit, and distinguish
an observer timeout from a demonstrated guest defect. This does not establish
interactive RDP, a real host pointing-device capture or audible sound.

At S1, `query session` showed the active local Console and an RDP listener but
no active RDP session. The current PC110 x64 and x86 artifact hashes match
those recorded in T585 S21; S22 changed tests only. The native-MIDI creation
failure recorded in S21 remains an environmental exception, not a pass.

## S1 actual current-product matrix

The current deployed EXE for each width was copied into a separate test-owned
directory with T584 media/CMOS copies. On an isolated, non-visible desktop the
observer selected each display, started the real process, observed the DOS
calibration screen, sent native Escape, entered Personaware World Clock with
Enter, paused and resumed, exited the application, and verified normal product
exit. Each driver completed 6/6 steps; each observer exited zero; both source
disk copies retained their original SHA-256. No user media was written.

| Variant | Width | DOS calibration observed | Driver log SHA-256 | Observer log SHA-256 |
| --- | --- | --- | --- | --- |
| Derived DSTN | x64 | 182.281 s | `4e4c0da826ffbf12db80deda2624b00feefe1238516d4199c0004d7d8d767cec` | `0ffea22e9f4c43735220dad9fec563391263ab9afc0b5862a4143ddbde12f645` |
| TFT | x64 | 182.313 s | `2f77f38055c32360ba0c6bfaa75aeec121aaa8600dd9e69863e0a49aefaba3c9` | `d2bb58cdb9f2e64554fcba51b57a6afca931eaeF2fe79e1b1cddedc8c8d155c6` |
| Derived DSTN | x86 | 228.000 s | `55b4723cc1255581ad9f11f209b782e092beb61e53efc6f5f3c2da4e545124b7` | `33d62e7d3a809cf99c4b8a4e64e346b54386f05761c6004660fb329e5026a84d` |
| TFT | x86 | 227.078 s | `22b332568bba19f25a3f832acb166d815024aa2fb726f2f3e318774d2a5fe37a` | `4d5c057b6c29faebd89bb44113077e9b87c50f7badc0c43fd36474615189bb87` |

The calibration frame's common SHA-256 was
`07987e8e9b2aedabd72e0caf5bd24cf618a55a11f716a4e0ada076bd5d40acf3`.
The aggregate observer log hash was
`d69f0e5ea158a777a531d96510001195d9a6cfcdf6f452e8bcc1a5d179ca764c`.
Durations reflect two concurrent instances and are observations, not a
performance guarantee. This matrix uses posted native messages; it does not
establish physical keyboard, raw mouse capture, RDP or audible output.

## S2 first restart attempt: observer defect

Both x64 variants reached DOS after `stop`/`start`, and each displayed the
known new POST frame before the second DOS frame. The observer then timed out
waiting for the reset completion; neither attempt is a reset pass. Its second
pause snapshot exposes the cause of an invalid observation: its chosen last
`Machine paused.` was earlier in the Console buffer than the second
`Machine started.`. The task-only observer had checked against a later
`Machine resumed.` but not a later `Machine started.`. It therefore sent the
next command without confirming a fresh pause. This is a confirmed observer
defect; the timeout alone does not establish whether product reset is faulty.
The retry must require a pause newer than both start and resume notifications.

The first-run aggregate log SHA-256 is
`acf00024560e487040d60cf51c858f3360040ba89e402e27ad3643f12c389b73`;
DSTN x64 observer/driver logs are respectively
`3bc1ecdcbea817d1a5c78c94631f65801989a7aeBCD6e75d0732d6174eca3c27` and
`a15c314e7b692d682a9e883df3801a038a37c04b428e1d43b6c4823fcf6d5ab8`;
TFT x64 observer/driver logs are respectively
`9fd4396badf7773070570bc185a517c05681de747fc89114120bd09770121c86` and
`d518ad3d56d86e672c63afa68bab10a29adf299719a079fbb093a2e6dbac67f3`.

## S2 corrected current-product restart matrix

After correcting the observer's stale-pause check, each current EXE ran with
both display variants and private copies of the existing DOS disks and CMOS.
For each `stop`/`start` and `reset`/`resume`, the driver required the explicit
new POST frame `94c5272ffead1ebf7b989d7e8fb0bcce5019dbe8ae55be0819dfdc9b4308a515`
and then the DOS calibration frame
`07987e8e9b2aedabd72e0caf5bd24cf618a55a11f716a4e0ada076bd5d40acf3`.
All four drivers completed 5/5, all observers exited zero, and both private
source-disk hashes were unchanged. The test used posted native messages on
isolated desktops, not physical keyboard input or RDP.

| Variant | Width | DOS after stop/start | DOS after reset/resume | Driver log SHA-256 | Observer log SHA-256 |
| --- | --- | --- | --- | --- | --- |
| Derived DSTN | x64 | 343.047 s | 519.563 s | `a12e6c153686955f3f9447a559e4a136cfdb2bee069235eb8a171920501d6018` | `c4bdc63a272539d0b6dd5b85fa6844a761315edf45924e3377a1c8f4a70984c9` |
| TFT | x64 | 353.469 s | 529.860 s | `eabb26db403988acdb4963dbebd3899ed9c8a46f19fd184dddab13c9ddce0715` | `954c2d9b1321147cd89a1e7703efc3bf81ea1e389337f3fe8623bcf287905482` |
| Derived DSTN | x86 | 439.547 s | 656.406 s | `562f3112d1dc2018a5c719eeda6a0004f256e3428ece812cdd4fde6752d628cb` | `2a95f1950cd4ed84e199079d93660c30f961b2b5af68f643748c7af12596b21c` |
| TFT | x86 | 445.250 s | 667.312 s | `cd26f05bdae8ec8679e6dec7c59299796add68edcb3097ab704957624a7dbd68` | `21525d9d3d8265783a77f92480d975e3abb687a3a7ea6af55156e5a192b3216d` |

Times are from observer start with two concurrent instances; they are not
performance guarantees. The retry aggregate log SHA-256 was
`6f279620c9d06b7c521338e04feaa0025cb7f023e6442023cb345f8da1d52c95`.
The first observer failures above remain part of the record and were not
counted as passes.
