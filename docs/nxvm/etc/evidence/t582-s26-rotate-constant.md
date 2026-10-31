# T582 S26 — constant-work ROL/ROR

The existing ROL and ROR helpers applied one bit at a time for 8-, 16- and
32-bit operands. The count can reach 255 for the existing 8086/8088 and V30
semantics; later profiles mask it to five bits. The new shared helper retains
that effective count and observation fields, reduces the rotation amount
using the power-of-two operand width, and computes the same value and final
carry in constant work. A zero count leaves flags untouched; a nonzero count
sets carry; a count of one computes overflow and larger counts preserve the
existing undefined-overflow marker. Invalid widths still report the existing
error. There is no new public interface, state or timing path.

The production CPU source is +36/-161 lines, net -125. For fixed-width
operations, the former implementation's steps grew with the effective count
and the new implementation uses a fixed number of operations. The count is
itself an 8-bit value, so both are formally O(1) with that fixed bound. This
is an algorithm and source simplification, not a claim that all guest
instructions run faster.

A task-only iterative model compared 886,784 width, count, value and direction
cases. A separate real CPU differential compared baseline and candidate across
131,072 cases per executable: seven profiles, both directions, each supported
width, all 0–255 count values, operand samples and two initial flag states.
The digest covers registers, flags, decoder observations and timing fields;
both baseline and candidate produced `65df64bcfc23085b` on x86 and x64.
It does not compare every possible machine state. An initial probe mistakenly
cleared V30's native-mode bit and crashed in the baseline's unrelated 8080 I/O
path; restoring that bit corrected the probe. Existing independently linked
CPU tests passed 122/122 per host width against the candidate.
The existing rotate test additionally covers 0, 1, 7, 8, 15, 16, 31, 32,
33 and 255 counts across Intel profiles, applicable widths, both directions
and register/memory operands. V30 already has its own complete count test.

With private PC110 media and 10,000,000 retired instructions, ordered
baseline/candidate/candidate/baseline times were
20,276/19,918/20,030/18,764 ms on x64 and
23,985/24,014/23,941/24,227 ms on x86. All reported 79,964,294 guest ticks
and PC `0003dbc5`. These samples do not show a reliable runtime gain or stable
regression and do not prove complete-state equivalence or Game performance.
The owner's criterion permits the clear source and algorithmic reduction when
other properties do not materially worsen.

Cached x64 unit tests passed 712/712. The x86 run initially passed 711/712;
the remaining source-manifest test failed because the newly generated hash was
lowercase. It passed after the manifest was corrected to the required uppercase
form. The subsequently extended rotate boundary test passed separately on both
widths; both changed manifests were independently verified. Default-profile
integration passed 48 tests per architecture (x86 was run as 23 plus 25).
The 5160, 5170 and DeskPro integrations passed 3, 5 and 6 tests per width.
These include automated desktop-labelled tests, but are not manual desktop or
RDP acceptance. MyRaidenII's external selected-set startup and product-console
probes passed on both widths, without a gameplay claim. Seven products were
built or confirmed current; PE widths and hashes for all 14 executables are
in `t582-s26-rotate-constant-artifacts.json`. MyNES does not link X86 Core and
its release binaries did not change.

A further x64 private-media product-path probe using this implementation
reached the stable Game frame 31 one-second samples after Enter and exited in
255.0 seconds. The frame hash matches the earlier stable Game frame; this is
not a same-state speed comparison, interactive input test or proof of usable
performance. SHA-256 (log, frame):
`66D646D653F575DAACD633B4FB97593332905DBBD46242978A19EC7BEEB125AF`,
`E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.
The PC110 usability goal remains open.
