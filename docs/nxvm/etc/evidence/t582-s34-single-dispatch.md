# T582 S34 — single-owner instruction dispatch candidate

The existing CPU initializes per-instance primary and 0F dispatch arrays of
256 function pointers each. A task-only candidate replaces both arrays and
their initialization with one direct switch for each instruction space;
repeated 0F undefined entries share the default. It does not add an executor
or retain a second handler mapping. The original initializer selected the
66h/67h V30 handlers at initialization time, rather than on every profile
binding. Two private `lib_bool` fields in the production implementation
preserve that selection and the zero-initialized, not-yet-initialized state
instead of silently using the later profile. These fields replace the existing
table-owned initialization state; there is still one dispatch implementation.

The generated mappings cover all 256 entries of each original table; 168 0F
entries originally used the undefined handler. `sizeof(t_cpuins)` decreased
from 17,144 to 13,056 bytes on x64 and from 15,064 to 13,024 on x86. At the
same fixed 10,000,000-instruction / 79,964,294-tick / `0003dbc5` checkpoint,
the corrected candidate's one forward baseline/candidate pair was
18,824/17,778 ms x64 and 24,928/23,625 ms x86. The reverse pair was
candidate/baseline 19,552/19,398 ms x64 and 22,501/21,600 ms x86. This
variation does not prove an execution speedup or the absence of a small
regression. Directly linked diagnostic executable sizes fell from 3,117,613
to 3,092,983 bytes on x64 and from 3,273,199 to 3,256,955 bytes on x86.

Six existing execution/lifecycle, decode, prefetch, ALU and V30 tests passed
for the candidate on both widths. A task-only real-CPU differential compared
both variants on 28,672 primary/0F opcode and representative ModR/M cases per
width, spanning seven profiles and recording selected registers, exceptions,
memory and timing observations; grouped hashes matched. Separate uninitialized
and 66h/67h profile-rebinding observations also matched. The initial probe
fixture lacked port completion callbacks and crashed on the baseline; those
results were discarded, the fixture was corrected, and both variants then
matched. This differential is not exhaustive CPU-state, exception, protected
mode, concurrent-use, or full-receiver proof.

An x64 private-media complete-frame Game baseline/candidate comparison exited
normally at 268.05/268.06 seconds, with the same stable-frame SHA-256
`E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.
Both reached the frame 30 one-second samples after Enter. This pair shows no
observable Game speedup; it does not establish complete guest-state equality
or native Window/RDP behavior.

The instance-space and source-code reductions merit qualification under the
owner's explicit criterion even without a demonstrated speedup. The patch must
preserve all CPU profiles, initialized/uninitialized and profile-binding
semantics, instruction and 0F mappings, exceptions and timing; update source
and test manifests; pass the cached full dual-width Shared receiver and check
the 14 product artifacts. Neither this candidate nor the existing comparisons
establish interactive native Window/RDP usability.

The single-dispatch implementation has now been applied to the existing X86
Core CPU files. A repeated bounded differential with the final two-boolean
representation matched on both widths, including rebind and uninitialized
observations. The existing lifecycle test now asserts these boundaries; six
focused tests passed on each width. The source and test manifests were updated.
The cached dual-width receiver completed: x64 had 711/712 units initially
pass; the remaining 8086 decoder-ledger check expected the removed pointer
table syntax. Its NXVM-owned checker now checks the direct cases while
retaining its timing and ledger assertions, and its targeted rerun passed.
x86 passed 712/712 units with that correction. Default integration passed
48/48 on each width; the 5160, 5170 and DeskPro profile integrations passed
3/3, 5/5 and 6/6 respectively on each width. MyRaidenII's existing startup
and product-console probes passed on both widths; they do not establish game
playability. Documentation governance passed. The
[artifact inventory](t582-s34-single-dispatch-artifacts.json) records fourteen PE widths and
hashes; default NXVM, PC110, MyNES and MyRaidenII deployed files matched their
corresponding built binaries. No Game speedup or native/RDP usability is
claimed.
