# M5 T586 S3 — final PC110 qualification review

The first three selected tasks are closed: [T583 first-time setup and
restart](../../history/M5-T583-pc110-first-setup.md), [T584 product
interaction](../../history/M5-T584-pc110-product-interaction.md), and
[T585 device and model qualification](../../history/M5-T585-pc110-device-model.md).
T586 S1–S3 have completed their recorded work. The owner accepted the
delivery and directed T586 closure on 2026-11-02.

## Current product and evidence

The S21 cached builds produced and PE-checked all 14 App executables. The
currently deployed PC110 files still have the recorded build hashes:

| Artifact | SHA-256 |
| --- | --- |
| `assets/mypc110/mypc110_x64.exe` | `c2531f73b19939b94a3c3cdbc2324977503d631e0e12d7ec6e115e5565938ac9` |
| `assets/mypc110/mypc110_x86.exe` | `be729f7284655ee4542c28ff0c8698a5a19f230d93c03dcf4db8797c7fccc8f1` |

One pair selects DSTN or TFT while stopped. DSTN firmware is derived, not a
verified original dump. [T586 S1/S2](t586-product-qualification-ledger.md)
ran the current real executable on both widths and both variants with private
media and CMOS copies. All four cells reached DOS and Personaware, exercised
pause/resume, and exited normally. All four also reached fresh POST and DOS
frames after both stop/start and reset/resume. Posted native input on an
isolated desktop is not proof of physical keyboard, mouse capture or RDP.

The latest production-source full unit suites passed 714/714 on each width.
Registered integrations passed 47/48 on each width; the native MIDI output
creation case failed on the isolated desktop without an available native
endpoint, an owner-approved environmental exception, not a pass. S22 later
changed the existing TFT cold-boot test and passed it on both widths; derived
DSTN BIOS APM probes also passed both widths. Selected XT, AT and DeskPro
integrations passed 3/3, 5/5 and 6/6 respectively on each width. See the
[device ledger](t585-s23-device-coverage.md) and S21/S22 evidence for scope.

## Limits and ownership

No interactive RDP session, physical keyboard chord, physical mouse capture,
audible output or physical hardware equivalence was verified in this review.
The device ledger explicitly retains absent external PC Card I/O and host
serial transport, dynamic battery/AC/lid behavior, incomplete ES488/OPL2
modeling and panel fidelity. The UART loopback, APM status and bounded L1/L2
audio models are not claims that these remaining features are implemented.
Additional RAM configurations and startup performance remain in NXVM TODO.

S1 and S2 used test-owned media and preserved its source hashes; no user INI,
CMOS or disk was modified. Reviewed task-owned scratch was removed after its
processes exited and its evidence hashes were recorded. Other tasks' build
artifacts were not removed. No production changes were made in T586.
