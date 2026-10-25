# T580 S51 - Cold POST to firmware-requested suspend

## Baseline and method

Baseline is pushed `267b67a0b`, with the S49 FDC and S50 Flash receivers.
The admitted TFT BIOS/font remain unchanged. The existing integration
executable temporarily records copied retirement facts and uses the existing
execute watchpoint; no guest register, memory, instruction or control flow is
forced. All temporary C is removed before delivery. Source/test delta is
therefore +0/-0; no Shared manifest, API, artifact or owner input changes.

This batch reconciles the cold initializer, subsequent choice, SMM request,
checksum and power exit. A sampled checksum PC alone was not a deadlock.

## Actual causal chain

| Stage | Direct observation and interpretation |
| --- | --- |
| Cold register image | Existing App construction seeds Block 2 FB=18h from the recorded live capture, which is L2, not a power-on specification. |
| Firmware initialization | On both widths, the first observed FB write is zero at 3E3D7. The retained call chain includes 3800:0BA6, in the real cold initializer. The sampled run ends at 5,177,344 instructions, with no processor reset or exception yet. |
| Later normal guard | Both widths execute the TEST at F000:3FF8 and pause at F000:3FFA after 27,207,296 instructions and three staged resets. AL=00, FLAGS=0046, so the zero branch to F4020 is taken. Stack return 3FBB identifies its caller. |
| Request | F4020's branch prepares the global vendor request. Actual x64 OUT at F000:DD10 carries AX=5380, BX low word 823F, CX=FF02 through the configured ASMI port 15EE. The vendor handler reads saved AX=5380. |
| Dispatch | The actual SMM caller 3800:250F invokes the power handler at 3911A. The entry has DS/ES/SS=3000, SP=233C. This is the firmware's selected service, not a fabricated host wake or keyboard input. |
| Checksum | Initial x64 containment at 32,000,607 instructions has completed 28,165 of the 32,768 LODSW iterations, CX=11FC and SI=DC0A. A revised 33M containment allows the known finite loop to finish; it is not a production fix or success criterion. |
| Power exit | At 32,019,076 instructions, the x64 loop has exactly 32,768 iterations and one exit. The guest writes CMOS shutdown 0F=0E, configures its existing 22h/23h protocol, writes Block 2 FF=82, and executes HLT at 39333. Core returns WAITING_FOR_INTERRUPT at 39334, FLAGS=0012 (IF clear), without a terminal fault. |

The first FB clear is normal, executed firmware initialization, not a random
host key or damage introduced by S50. The later branch consumes that zero.
Neither this sequence nor a live FB=18 capture proves that FB bit 3 is a
read-only strap, should always be high, or may be forced high to skip suspend.
An explicit source-qualified power/status model is still needed.

The four delivered protected-memory GP probes remain visible in the later
runs; they are not terminal Core faults or suppressed exceptions. No INT 19h,
Setup loader or guest boot is observed by this batch.

## Source reconciliation and design disposition

The immutable BIOS establishes the software-visible sequence, not a complete
hardware implementation or physical timing grade. Relevant read-only sources:

| Source | SHA-256 |
| --- | --- |
| Open-Source-PC110 Discovery/Chipset/readme.md | `70E05E07FDFCF5786A5DE6AB5787B6726032016151E0A6A19B1B61E65794F806` |
| Open-Source-PC110 Discovery/Chipset/block2_config.txt | `EE53C5EF8EF0E898809FBE7D1D19018DDE4B351391477E179B4FB40CDE327E69` |
| PC110-QEMU qemu/hw-misc/pc110-chipset.c | `950985A7C280F10D9E028317F853A0B0D53D4BD771FB8283EF3F85A95682028B` |

The hardware record identifies B8 bit 3 as a resume strap, FB bit 3 as an
incompletely understood power/status choice, and FF=82 followed by HLT as
suspend. It does not give a complete FB input/write-mask or wake/reset truth
table. The reference QEMU register file is plain read/write; its separate
firmware redirects are not hardware evidence. PC110-EMU's Setup-specific
read forcing likewise cannot justify a production register special case.
No reference implementation text is imported or transliterated.

The required next receiver is one coherent SL-compatible power contract:

1. Distinguish existing guest-writable configuration from hardware-produced
   status and resume causes; specify FB/B8/FF and mode-22h interactions.
2. Define the actual wake source, retention/reset behavior, and observable
   transition together. A source-labelled bounded L2 model is permitted;
   a ROM-address escape or unconditional fabricated completion is not.
3. Keep chipset-owned state in the existing configuration/power owner and
   physical wiring/input in the App. Reuse the existing Core SMM, clock and
   interrupt/reset capabilities where their contracts fit. Do not duplicate
   the App's mode register or invent a second clock/state manager.
4. Prove cold entry, suspend, wake and return as one receiver batch, then
   return to the unchanged BIOS's normal Setup/boot route. Do not change
   generic HLT semantics or Lib/Emulator to make this one firmware proceed.

This S admits no speculative production repair: the writable-mask and wake
contract are not yet resolved. The next batch must settle them before choosing
an interface or implementation. No physical-unit evidence is demanded as a
blanket prerequisite; a justified, falsifiable L2 model remains allowed.

## Verification limits and rejected diagnostic attempts

The x64 complete detailed run proves the power exit above. Two x86 long
observations exceed their original 180-second wall budget; the owned job
trees are terminated, not counted as passes. The later rearmed watchpoint
captures the actual guard identically on both widths. A planned second
watchpoint edit is rejected with INVALID_STATE after that capture because
the paused instruction still has an incomplete retirement wait. The valid
first copied guard is retained as evidence, but the two-stage diagnostic is
not reported as a successful transaction and no pending state is bypassed.

An earlier watchpoint draft omitted rearming after processor resets and
obtained no guard. Its zero-initialized fields are not register observations.
Its x64 unobserved prefix nevertheless ends at the same actual instruction
count and HLT as the detailed run; this is a limited observation-neutrality
cross-check, not proof of every instrumentation configuration. All drafts
are removed, not retained as a second test or production entry.

The final first-FB-write observations pass on both widths and produce
byte-identical logs. They stop on the actual write, not an instruction ceiling.
After restoring the original harness, both original graphs rebuild it and
the four previews. Existing App units pass 11/11: x64 1.08 s, x86 1.07 s.
All production and retained test inputs are identical to accepted S50, so its
complete 712-case-per-width unit qualification remains applicable; the
unchanged suite is not rerun solely for this documentation-only delivery.
No new complete external integration, desktop/RDP, original-DSTN, panel,
guest or product qualification is claimed. Preview hashes remain those in
[S50](t580-s50-lower-flash-selection.md); there is no new deployable PC110 EXE.

## Cleanup

All owned processes have exited, including the two deadline-terminated jobs.
Nine logs total 12,472 bytes. After retaining the following facts/hashes,
official preview reports no deletion and Apply removes exactly those nine
files/12,472 bytes from `build/t580-s51-handoff`; caches and owner inputs
are not deletion targets.

| Temporary log | Bytes | SHA-256 |
| --- | ---: | --- |
| x64-fb-writer.log | 717 | `27898C42EA0976F09C237FD38F0472C916A0CCBA8743AD588F2AAD8C83A227CE` |
| x86-fb-writer.log | 717 | `27898C42EA0976F09C237FD38F0472C916A0CCBA8743AD588F2AAD8C83A227CE` |
| x64-handoff.log | 3258 | `A5A291D050170630F8905B8A83328B856D9FDAFBE5BABDE6D6A9BCA5F079B374` |
| x64-power-exit.log | 2848 | `660A169CF284DF0195A58EE78D91917180B5559DB85F7F64DB79A38D477EFEDB` |
| x86-power-exit.log | 485 | `F94629AB75BCECD97E4165D06BEA60C691D1EA7BCBB3DFF7F9E5970A7FA20BFB` |
| x64-guarded.log | 1562 | `89CDE462ED689550660665F4CC2675AC8D6AF5910D39AD4C8021B72416535535` |
| x86-guarded.log | 485 | `FAD6179A31F52F414B539D3F8A2A3EBDC7C18709C8FEBDDCD47B2968A341232F` |
| x64-source-guards.log | 1200 | `E727DDD8A07AE5A2D74C34F4932C29B063E05EA7C457532840E0CF0689318A72` |
| x86-source-guards.log | 1200 | `62A42787BC9BC532AE854EDB0499EA143E0A9F008743D2284F347CAB6BCA1213` |
