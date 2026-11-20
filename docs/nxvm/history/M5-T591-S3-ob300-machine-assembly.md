# M5 T591 S3: OmniBook original-card Machine assembly

Coordinator review checked the actual scoped source and test changes
from `2617d595f` through `00c565d81`. The App owns the external card
and original mappings and releases the card after Core teardown.
The original firmware passed the protected-mode and display stops.
Both host widths booted ROM DOS, persisted a guest-created C file,
and executed VER through the actual keyboard chain. Generation-aware
worker probes independently covered cold start, pause/resume,
reset/resume, and stop/start on both widths.

Both full unit sets passed 717/717 after the last Shared change.
All fourteen receiving EXEs were rebuilt, verified and pushed. The
[verification evidence](../etc/evidence/t591-s3-ob300-machine-assembly.md)
records the fingerprints and current boundaries. The same tests show
cold/restarted DOS startup exceeds the frozen usability targets.
S4 owns the controlled baseline and Directory/input-latency checks.
Product UI and Windows remain in their ordered receiver work.

## Archived packet

# M5 T591 S3 - Active: OmniBook original-card Machine assembly

[S2 is closed](../history/M5-T591-S2-ob300-pit-readback.md).
[S1 original-card checkpoints](../etc/evidence/t591-s1-ob300-original-rom.md)
identify the protected-mode stop. The
[S3 assembly evidence](../etc/evidence/t591-s3-ob300-machine-assembly.md)
records its resolution, receiver publication, and later original-ROM checkpoints.
Original-ROM probes now reach ROM DOS startup using production receivers.
The App maps the two option images and reports C-slot presence. Shared
CRTC readback, ATA startup and Am386SXLV data-limit changes passed focused
x64 and x86 regressions. The original card with the matching read-only C
image passes the previously traced stops. The App handles the observed
SLPCTL write. With the active primary partition in the disposable image,
both widths complete LapLink startup and reach a later ROM copy at
FDBADh. The shared receiver batch and fourteen existing executables are
verified and pushed through 595b7c97a. The App's high System Card
aperture now passes the original ROM copy on both widths. Windows startup
reports insufficient extended memory at the fixed 2 MiB configuration.
Both host widths reach the ROM DOS prompt and persist a guest-created
file on disposable C media with Windows autostart disabled in the
fixture. Original DOS executes VER through the KBC on both widths with
the new output-register latch enabled. The full unit sets pass on both
widths and all fourteen receiving executables were rebuilt and
verified. Repeated lifecycle and desktop qualification remain open.

| Field | Required record |
| --- | --- |
| Identifier Mode | M5 T591 S3 after the public 8254 fix and all receiver publication. |
| Admission And Approval | Owner authorized ordered OmniBook implementation and single-session coordinator/executor execution. Owner approved the x86-nxvm A20 repair and the necessary x86-nxvm and x86-core repairs identified during original-card assembly. |
| Objective | Own the external System Card through existing Machine construction/teardown; validate real guest boot, VGA and writable medium routing at their established owners. |
| Non-goals | No embedded Microsoft payload, alternate executor, replacement ROM/DOS, product UX or guessed port/geometry. |
| Reference Baseline | S1 original guest evidence, S2 shared correction and the HP 5965-0244 service manual. |
| Candidate Proposal | proposals/m5-ob300-rom-dos.md T591 S3; quantified obligations from m5-ob300-architecture.md. |
| Files And ABI Surface | NXVM App board/Machine preparation, tests and evidence. Shared x86-nxvm Board profile services add one borrowed fast-A20 callback/context, using the existing Port A latch. The CRTC and ATA receivers use existing registers and the ATA config carries two power-on latch bytes. X86 Core retains the Am386SXLV real-address data-segment limit. MyOB300 selects optional empty 8042 data-port read retention at board construction. Publish existing receivers in NXVM, MyNES and MyRaidenII scope. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, Architecture/Layout and source policy, shared Execution/Documentation/Architecture/Coding. |
| Verification | Cached x64/x86; cover card lifetime, VGA frame, CMOS export, and the EEh/92h/KBC latch relationship. Recheck the original card and protected-mode stop, identify the C medium route, run the full repository unit suites and verify fourteen cached Shared receiving artifacts. |
| Expected Markers | Machine owns immutable card bytes until after Core teardown. Original-card execution progresses past the demonstrated stop with unchanged firmware. Writable guest media is distinct from the read-only card. |
| Asset Needs | Read-only owner 1.0 ABA System Card and matching C drive inputs; no downloading or distributing firmware/media. build/t591-s3 owns probes and disposable copies, 256 MiB, 300 seconds per native probe. |
| Reporting Requirements | Compare real-card and fixture evidence, exact source hashes, known versus modeled semantics, and scope of remaining guest checks. |
| Stop Conditions | Continue independent App work while original media is unavailable; do not claim a real ROM/C-media pass or choose an unobserved receiver. |
| Exit Criteria | Exact external-card lifetime verified, real protected-mode stop resolved, observed media/VGA path verified on both widths, all repository unit tests passed and review pushed. |
| Original Owner Request | Implement HP OmniBook 300 following MyPC110 boundaries: hardware, wiring, original-ROM Machine, then Product and testing. |
| Similar-Issue Sweep | Wrong-sized/short card, candidate failure lifetime, clean stop/start and reset, protected descriptor versus board window, immutable System Card versus C medium. |
