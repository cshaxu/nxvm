# T591 S4: Original ROM DOS qualification baseline

## Controlled worker run

Production baseline `1302c66be`, existing cached GCC Release graphs, and
an AMD Ryzen AI 9 HX 370 (12 cores, 24 logical processors). The workers
ran sequentially in Turbo. Each loaded the same verified System 1.0 ABA
card and separately created its own disposable C medium from the owner
backup. The fixture retained the original DOS CONFIG.SYS and replaced
Windows autostart in AUTOEXEC with a DOS file-write marker. The master
archives stayed read-only. The external ROM, backup and initial/post-run
C identities are in the [S3 assembly evidence](t591-s3-ob300-machine-assembly.md).

The probe submitted neutral KVM make/break events to the official
Emulator Machine and read text from its published frame. The cold interval
includes the F1 response to the blank-CMOS warning, one pause/resume,
and submitting VER. The result is the first published frame with the
actual Hewlett-Packard DOS version text. The earlier S3 Turbo results
validate fresh generations after reset and stop/start.

| Measurement | x64 | x86 | Gate |
| --- | ---: | ---: | --- |
| Cold start through VER | 81.813 s | 96.407 s | Missed: 60 s x64, 90 s x86 |
| Pause request to PAUSED | 15 ms | 15 ms | Passed: at most 1 s |
| Key a | 92 ms | 77 ms | Passed: at most 250 ms |
| Key b | 16 ms | 29 ms | Passed |
| Key c | 15 ms | 46 ms | Passed |
| Key d | 16 ms | 31 ms | Passed |
| Key e | 32 ms | 30 ms | Passed |
| Key f | 32 ms | 31 ms | Passed |
| Key g | 15 ms | 32 ms | Passed |
| Key h | 31 ms | 31 ms | Passed |
| Key i | 30 ms | 30 ms | Passed |
| Key j | 32 ms | 47 ms | Passed |
| DIR CONFIG.SYS to ready prompt | 4.363 s | 4.586 s | Missed: at most 1 s |

DIR starts at Enter admission after the command is typed. It ends with
the matching FAT directory entry visible and the cursor back at the
ready prompt. Each key interval starts when the probe submits its make
and ends when the next frame shows the additional character. The frame
consumer polls in 8 ms sleeps, so these are end-to-end samples with host
poll and presentation latency. They do not isolate guest execution time
or establish a faster lower bound. The 5-second observation cap does
not change the 1-second gate.

The two C images contain the original 690-byte CONFIG.SYS. After both
workers shut down, the FAT16 checker independently verified the new
directory record, allocation and exact 25-byte DOS marker payload.
Both final image hashes were
`1b9b2eabba5407772baa7066ed9204a63964f17df7393fec9283b1112abac5e6`.
A separate fixture ran DIR D:\COMMAND.COM in the guest and redirected
the listing to C. An independent FAT16 read after both worker processes
exited confirmed an original D: directory header, COMMAND.COM with the
matching 47,863-byte file size, and a complete 176-byte output file.
Both output disk hashes were
`bf6ed4bbb9dbc109e38e40054061a38dda00aa23a63615c9ca3c6e447f194a5e`.
These two parallel probes later timed out in their supplementary
prompt-timing loops. Directory persistence had already completed.
The sequential successful runs supply the reported timing baseline.

| Controlled replay | SHA-256 |
| --- | --- |
| `qualify-x64.log` | `5cf28da121b4e11961add685167b7b806a297d03a254698e6d70142b5a5d36f7` |
| `qualify-x86.log` | `da6b965d63111dac15521bf314296914e9f8636a49c8539e5e04f7197125360b` |

| D directory probe | SHA-256 |
| --- | --- |
| `directory-x64.log` | `fb6feb9f3df30b307cb85e8a102403908e4d11f477b3d3e047315793a82bd860` |
| `directory-x64.err` | `26c39ac353af8ec9ac402ad5ed6ada229923fef28a822ed19c97b41c23354c99` |
| `directory-x86.log` | `e7c3f7550f2ffece1f03a0bb3b0454e53543501fab87e55f33410ac04ef32516` |
| `directory-x86.err` | `723d790bae47383afd2ad7e675308cb10e6a7defb88092061f3b2e6d8b49e6a7` |

The headless baseline does not establish desktop responsiveness,
complete power-management behavior, or Windows application operation.
Startup and DIR misses go to the queued
[performance receiver](../../proposals/m5-ob300-performance.md) after
[product assembly](../../history/M5-T592-ob300-product-qualification-proposal.md).
The product receiver retains the same lifecycle and storage acceptance.

S3 cleanup verified all probes had exited, removed its exact directory
with the shared safety tool and reclaimed another 200,892,240 bytes
(556 files) after the two earlier disposable comparison images.

S4 removed its exact task directory after both probe processes exited.
The shared safety tool removed 27 files totaling 169,400,931 bytes.
The persistent Ninja and ccache trees were preserved.
