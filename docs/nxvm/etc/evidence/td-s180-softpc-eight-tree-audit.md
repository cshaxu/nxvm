# M5 Td S180 SoftPC Eight-Tree Audit

## Frozen Scope

NXVM baseline: `12e79911b`, initially clean. SoftPC committed baseline:
`44cad548`. The eight roots are src/test pairs for Lib, Emulator, X86 Debug
and X86 IBM PC, not the newly extracted X86 Core or NXVM-family package.
No source, test, build, API or executable is modified by this audit.
Architecture-governance precedes coding-governance review. The sibling is
read-only; its committed trees were archived under ignored build storage to
avoid mixing concurrent working-tree edits into a committed baseline.

## Actual Differences

The [complete hash inventory](td-s180-eight-tree-hashes.tsv) contains 272 union
paths: 238 equal, nine same-path differences, seven NXVM-only and 18
SoftPC-only. NXVM contains 254 files; SoftPC contains 265. These counts include
manifests and represent bytes, not normalized line-ending comparisons.

- `src/lib`: 109 versus 116 files; 23 union paths differ.
- `test/lib`: 49 versus 53 files; 11 union paths differ.
- The other six roots are byte-identical at these committed baselines.

Lib changes come from SoftPC's PCM relocation `7f642848`, native PCM contract
`b31b6e17` and MIDI/FM `46ad2e3f`. PCM now lives under audio/pcm; its progress
epoch/counters, notification and channel-gain operations are new. MIDI provides
native short/SysEx transport, bounded copied buffers and optional host volume.
FM explicitly returns UNSUPPORTED; it is not a synthesizer implementation.
Types, component dependencies, CMake, tests and manifests follow these owners.

`git diff --no-index --stat` with Git's rename detection reports source Lib
+1050/-294 (net +756), test Lib +1051/-364 (net +687), including CMake,
README/manifests. Counts are not pure C semantic change or an import estimate.
The source comparison recognizes two PCM moves; test similarity does not
recognize every relocated file as a rename. No duplicate legacy PCM owner is
retained upstream.

## Confirmed Findings And Repair Boundaries

### P2: formatting truncation falsely reports success

Both repositories' emulator/product/monitor.c:461,462,477 check only negative
snprintf results. Nonnegative required lengths at least equal to capacity
are truncation, not success. A one-byte title buffer returns success with an
empty string. Window status with no hotkey rows can similarly return success
after its initial text was truncated.

Use a signed result and reject negative or `(lib_size)result >= capacity` for
each destination. No public API, helper framework or alternate formatting
path is needed. Preserve current partial-output behavior unless its contract
is separately strengthened. Existing monitor_smoke covers successful buffers,
not the individual title capacity boundaries or empty-hotkey truncated status.
Add those exact-fit/one-short/one-byte cases at this owner.

The two format stubs in test/x86-ibmpc/product/entry_smoke.c:165 onward repeat
the false-success rule. Correct the stubs and test entry failure/cleanup;
do not duplicate all formatter tests in the PC package.

The scoped query finds 40 production call sites plus two Types macro aliases,
and 17 test call sites. Dynamic Debug output measures/validates lengths;
disassembly macros check capacities, assembler concatenation has its own
length checks, and fixed short messages or same-capacity prompt copies are
not additional confirmed defects. This is a targeted return-contract sweep,
not a claim to have proved all possible output behavior.

### P2: executable directory violates its own trailing-separator contract

Both lib/base/process.c:19 truncate at the first filename character, leaving
the separator before it. For `C:\apps\demo.exe` the result is `C:\apps\`;
for `/opt/demo` it is `/opt/`. process_interface.h requires no trailing
separator except a filesystem root. The current test checks only nonempty
success and invalid capacity, so it cannot detect this mismatch.

Trim the separator for ordinary directories while preserving POSIX, drive
and UNC root semantics; keep failure output unchanged. Test via the existing
private platform-path seam, not a new public path parser. Cover slash/backslash,
root and nested paths, capacity and platform failure.

The supplied report's external-caller list is incomplete: in addition to
SoftPC app-softpc/product/config.c:61, NXVM x86-nxvm/product/startup.c:42
and app-mynes/product/config.c:116 append a backslash unconditionally.
All therefore produce duplicate separators today for ordinary directories.
After fixing Base, root directories still require the appenders to avoid an
extra separator. Linked App changes need explicit receiver admission, not a
hidden shared-package edit or a new general path framework.

### P3: stale Console ownership and unclear Linux symbol prefix

lib/console/console_interface.h uses LIB_BASE_CONSOLE_INTERFACE_H and says
Host alone binds native I/O. The actual owner is Console Broker. Rename the
guard to the Console owner and correct the comment; add a narrow owner-local
regression to the existing naming/dependency gate, without scanning outer
components from Lib.

`kvm_linkvm_*` / `KVM_LINKVM_*` in kvm-base/linux/input.c,h and its contract
test are internal Linux decoder names, not another VM implementation. A
mechanical Linux-owner prefix cleanup is justified for clarity, with callers,
tests and manifest updated together, no compatibility alias. No behavioral
failure or duplicate header guard is established by this spelling.

### Not a violation: approved package identity and native scalar vocabulary

`nxvm_shared_library` is the owner-approved formal project name. The historical
NXVM shared-family package naming, including `nxvm_shared_emulator`, is not a
dependency on the NXVM App. Do not rename it merely to remove the word NXVM;
a new branding decision would be separate scope.
Native `float` used at the WASAPI gain boundary is explicitly permitted by
Lib Types' current vocabulary; inventing lib_f32 would contradict that rule.
The MIDI byte-size MAX constant is a real accepted-message upper bound, not
the forbidden default/MAX naming pattern.

### P1 fault-path risk: ignored PCM control completion signal

SoftPC audio/pcm/stream.c:120 and 170 ignore control_done signal failure after
processing clear/gain or flush. Its callers wait indefinitely at 405 and 449.
NXVM's old audio/stream.c has the corresponding clear/flush pattern at 75/111
and waits at 282/311. This is an existing shared defect class, extended to gain
upstream, not proof that it causes any previously observed timeout.

If the completion signal fails, the synchronous caller has no delivered
completion and the code does not establish a terminal wake path. The existing
signal-injection fixture can express this failure, but its current assertions
do not prove a control-completion failure releases the waiter. Repair must
couple terminal failure/cancellation and the existing wait contract, not add
timeouts, a recovery thread or silent retry. Review all request publication,
completion and error notifications together before implementation. A hang was
not dynamically injected in this read-only audit.

## Audio Admission And Minimalism Review

The PCM/MIDI/FM separation has real distinct transport responsibilities;
native handles remain internal and Types owns native declarations. Copied
SysEx storage remains alive until unprepare/close; failure retains ownership.
There is no guest Sound Blaster/OPL/MPU parser added to Lib. FM's unsupported
capability is honest but no reason to claim working FM output. These are
positive architectural findings, not complete runtime acceptance.

Raw import alone breaks MyNES: machine/driver.h still includes the deleted
lib/audio/stream_interface.h, driver.c calls lib_audio_stream_* and its CMake
links the removed audio target. An eventual import must update that receiver
to audio-pcm and prove reset/clear, accepted prefixes, inactive output,
shutdown and snapshot behavior. No compatibility aliases or second PCM path.
MyNES changes and its artifacts require the corresponding declared scope.
The new optional progress/notify/gain APIs need not be used by MyNES merely
because upstream added them.

## Concurrent Uncommitted Emulator Work

During the audit SoftPC added machine/machine.c, machine_interface.h and
test/emulator/machine_wait_smoke.c edits, followed by their two manifests.
The first intermediate observation had an invalid source manifest; this was
a moving worktree, not a defect in committed 44cad548. The later observed diff
disables heartbeat before PAUSED and reenables it for resume/private state-read
continuation; tests assert this without an extra public RUNNING notice.
This respects the existing executor owner and adds no new callback/API.
It changes receiver behavior and needs NXVM/MyNES driver regression before
import. It is not included in the frozen byte-equality conclusion or granted
final acceptance while upstream is editing it.

## Verification And Disposition

Both frozen eight-tree manifest sets have matching hashes, complete path
coverage and no duplicate entries. Current production Types layout, Lib
source dependency checks, Emulator/Debug/IBM PC corpus gates and all four test
package ownership/Types checks pass for both frozen baselines. These static
checks do not prove failure semantics or native output behavior.

No new dynamic suite, dual-width build, desktop/RDP, physical audio or MIDI
device verification ran. The supplied previous 94/94 is not counted as fresh
qualification. Public tree inventory is complete; detailed semantic inspection
targets the reported classes and changed audio mechanisms, not every line.

Recommendation: do not declare the latest worktree ready for unqualified
byte-for-byte adoption. First finish/pin the upstream Emulator batch, resolve
or explicitly admit the listed shared failures, and plan the required MyNES
PCM receiver update. No source repair or import is authorized by this audit.
