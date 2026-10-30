# T582 S26 — PC110-EMU same-host exploratory measurement

An independently written task-only harness compiled the unmodified local
PC110-EMU Core at commit `81235b5` with the same host's x64 GCC `-O3`. It
loaded the same owner-supplied DSTN BIOS image used by MyPC110 and attached a
private copy of the same master image. One call requested 10,000,000 loop
steps. The process returned zero in 3.389 host seconds, including creation and
loading; the reference reported 9,029,199 instructions, PC `0003C2B8`, and a
running CPU. It reported 14,396 port-61 reads, 7,198 read-based toggles,
18 BIOS idle-HLT resumes, two synthetic iterations of its F000:4139 loop,
and 471,169 INT 16 calls.

This is not a matched-instruction or end-to-end speed comparison. The reference
uses high-level service hooks and compatibility shortcuts, counts some work
differently, reported a 4,014,080-byte attached image after its own media
interpretation, did not load the external font, and was not supplied the NXVM
CMOS or second disk. Although the input BIOS image is the same, NXVM's existing
PC110 firmware construction applies its L1 POST-suspend compatibility patch to
this image; the reference loads the unmodified input. It reached a different
guest state. Its 3.389 seconds
therefore cannot be divided into NXVM's fixed-work result to claim an
equivalent speedup, nor does it establish that its user interface or clock is
correct. The result nevertheless demonstrates that the local reference can
run this ROM/media combination and that it advances the observed bootstrap
through materially different paths. Its per-read port-61 toggle, host-derived
RTC, INT services, guest-specific repairs and instruction-loop handling are
real source differences; their individual effects have not been isolated.

No external source was imported, no owner media or external repository was
changed, and the temporary executable is not a release artifact. The probe
does not justify weakening the existing time model. Log SHA-256:
`2F49986761A087D765E07E87CC22D2AD9A0D06A5FFE30C8FF9FC6BEF8587F39E`;
reference Core source SHA-256:
`D484454BEB37A9D3EEF5CF716FCEF80E8131772C82532DF3A411704C57BF898A`.
