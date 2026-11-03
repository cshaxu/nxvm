# M5 T586 — PC110 current-product qualification ledger

This ledger consumes the owner request to finish the first four PC110 tasks
and the [final qualification proposal](../proposals/m5-pc110-final-qualification.md).
It does not equate an L1/L2 device model with physical PC110 fidelity.
The current product is one selectable DSTN/TFT build in each host width;
the DSTN firmware is derived, not a verified factory-original image.

| Requirement | Earlier evidence | Current disposition and next receiver |
| --- | --- | --- |
| Build and artifact identity | T585 S21 built all 14 EXEs and verified PE width; PC110 deployed files matched both builds. | Recheck current paired PC110 artifacts and unchanged production after S22; S1. |
| Firmware cold and resume handoff | T585 S21 final integrations reached real firmware cold and resume handoff on both widths. | S22 extended the existing TFT cold test with actual BIOS APM; both widths pass. Both derived DSTN probes also exercised that APM route. |
| First Setup, CMOS, DOS, stop/start and reset/resume | T583 exercised both variants and widths with private media and guest-created CMOS. | Those observations predate S21 COM1; current actual-product checks must not be inferred solely from firmware handoff. |
| Personaware and guest input | T584 reached World Clock and exercised input and pause/resume on both variants and widths. Four-corner calibration used the Machine path. | Repeat actual current-product DOS-to-Personaware and lifecycle on the four matrix cells using disposable copies and isolated desktops; retain distinction between posted native messages and physical input. |
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
