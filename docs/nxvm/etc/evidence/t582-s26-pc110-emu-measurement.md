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

## Isolating the port-61 shortcut

A task-only x64 build changed just the existing port-61 refresh-status read
from guest-time toggling to a per-read toggle. It intentionally changes
device timing and is not a production candidate. With private disk and CMOS
copies and the Release trace setting, the original model reached the fixed
10,000,000-instruction checkpoint (79,964,294 ticks, PC `0003DBC5`) in
21.49 seconds. The per-read version did not reach that checkpoint before
the bounded 150-second run was terminated and reaped.

A second 45-second observation of that candidate logged 7,596,998 executed
instructions and linear PC `000F699A`; for the remaining logged interval the
instruction count and PC remained unchanged while guest ticks advanced.
The selected BIOS input at the preceding addresses contains `CLI; HLT; RET`;
PC110-EMU separately has a specific compatibility path that resumes the HLT
at `F000:6999`. The experiment supports an interaction between the shortcut
and that firmware wait, but did not capture the live CPU halted flag or prove
the complete causal chain. It does show that importing only the per-read
status behavior does not reproduce a successful reference boot in this setup.
No timing policy, ROM, public code or released binary was changed. The
45-second observer was intentionally terminated and reaped after collecting
the bounded data, so it is not a completed boot. Log SHA-256:
`BFDFC5AC4DEF1A63796A09171E0046380FF95104D554617529D422A254699F7C`.

## Combining the refresh and HLT shortcuts

A further task-only candidate combined the per-read refresh toggle with a
signature-checked temporary firmware copy that replaces the `F000:6999`
HLT with a no-op. Both deliberately change guest behavior and are not
production candidates. With private media and CMOS, it reached 10,000,000
instructions in 18.984 seconds at 73,857,919 guest ticks and PC `000F6943`;
the ordinary model's checkpoint is a different PC and tick count, so those
durations do not compare equivalent work.

In a separate product-path run, the candidate did not reach the Personaware
menu. At the final sampled quantum it had executed 121,090,236 instructions
and remained in the BIOS's port-61 bit-5 polling region (`F000:693D` through
`F000:6943`). The task-owned process was deliberately stopped after 186.66
host seconds; this is a failed/incomplete observation, not a completed boot.
The local PC110-EMU reference also contains an address-specific escape for
this very loop that forces the ready condition after a fixed number of visits.
The current experiment does not prove a defect in NXVM's PIT model: this path
was entered after two artificial changes, and no PIT state was captured.
These results show that carrying over the first two shortcuts alone does not
deliver a working accelerated boot. No production source, public interface,
owner media or released executable changed. Observed log SHA-256:
`0476DE80F6ED48E5273AB5FD4200CA9175FACBBCC6E6326683322A384035BE61`.
