# T559 S6 CPUID And Flags

## Baseline And Source Contract

Implementation baseline: `1951cb6fd`. Executor qualification is complete,
pending coordinator acceptance; this is not production 486 admission.

Intel Embedded Intel486 SX datasheet 272769-004 (August 2004), printed pages
23--24, defines CPUID leaves 0/1, GenuineIntel, family 4/model 2, VME=1/FPU=0,
and a writable ID bit cleared by reset. The local source hash is
`a35a0a428168ed45428dd886876238424a228b8db0583ee74a1815a88bf421cd`.
The stepping B candidate comes from the historical PC110 042B observation
recorded in T558; this does not prove that the complete later datasheet feature
set applies to the physical PC110 package. The existing Core machine profile
gate continues to reject 486 production assembly. The system receiver must
resolve applicability and implement VME before any product advertises it.

Leaf 0 returns maximum basic leaf 1 and GenuineIntel; leaf 1 returns candidate
042B and feature mask 2. Reserved output registers and undefined leaves above
1 return deterministic zeros. These zeros are an explicit emulator policy for
undefined values, not a claimed silicon measurement. Reset EDX carries the
signature, not the feature mask. ID's current value does not enable/disable
CPUID execution. No extended-leaf feature set is invented.

The original 1992 Intel486 Family Programmer's Reference Manual, hash
`d86207aaebd9b10464f2058efc61559e6141d3b072db100e89bfe17c8187e143`,
sections 4.1.1 and 21.3.1 and POPF/POPFD at printed 26-235, specifies AC and
the inherited flag/privilege rules. POPFD can change AC; POPF preserves the
upper half. IOPL changes only at CPL0 and IF only when CPL permits. POPF does
not change RF or VM. ID is the later datasheet extension. Real alignment
checking (#AC/CR0.AM), VIF/VIP and CR4/VME effects remain system obligations,
not effects established by merely storing flags.

## Owners And Changes

- Existing CPU metadata, dispatch handler and reset identity implement CPUID;
  no second decoder, public API, persistent state or profile-selection path.
- One private reserved-mask helper selects the existing 386 mask or its
  486 AC/ID extension. Existing PUSHF/POPF, event image, real/protected/VM86
  IRET and task-load paths consume it. Instruction-local privilege masks stay
  with their instructions. Earlier-family mask/load behavior is retained.
- The candidate task-load boundary clears reserved bits; VIF/VIP are not
  treated as implemented writable hardware state in this instruction batch.
- No Shared-six, MyNES, App policy, firmware, INI or external media changes.

## Direct Proof And Regression

The new CPU-owned CPUID/flags test directly executes the existing handlers:

- Real, protected CPL0--3 and VM86 CPUID; independent code-size and 66/67
  combinations; ID both clear and set; leaves 0, 1 and undefined ranges.
- Full CPU equality except EAX/EBX/ECX/EDX and instruction pointer; no writes.
- Every truncated preview and every byte of a prefixed execution-fetch failure;
  failed fetch preserves the full CPU and makes no data write.
- Actual old-386 and LOCK UD delivery. Earlier-profile previews do not accept
  a complete CPUID; 8086's one-byte POP CS is not misclassified as CPUID.
- Reset signature and cleared ID/AC; 386/486 PUSHF/POPF transfer, 16-bit upper
  preservation, clear/set AC/ID and reserved-bit rejection.
- All CPL/IOPL combinations for 486 POPF, both widths and independent AC/ID.
- Real/VM86 IRET clear/set and width preservation.

Existing protected IRET tests now cross 486 AC/ID patterns with same/outer
return, width, CPL, old/new IOPL, IF and RF. Existing task-switch completion
tests cross 386/486 and independent AC/ID/RF/TF with direct JMP, nested CALL,
gate CALL, gate JMP and interrupt task gate. These reuse existing fixtures,
not a new execution harness. Existing earlier-family and VM86/task regression
tests remain part of qualification; green old tests alone are not new 486 proof.

Focused x64/x86 tests passed. The first x64 full run executed 597 units in
214.12 seconds: 594 passed, two registrations of the same profile test failed
because they still expected CPUID metadata to be invalid, and the test manifest
detected the subsequent source correction. The corrected assertion requires
valid metadata with minimum CPU 486SX; it does not relax old-profile rejection.
After rebuilding both registrations and updating the manifest, all three
affected tests passed in 1.84 seconds. Production code did not change between
these runs, so the other 594 passing results remain applicable. Logs are
`build/t559-s6-x64-unit.log` and `build/t559-s6-x64-recheck.log`; the first log
is deliberately retained as a failed run, not relabeled as a clean sweep.

Actual production C/H changes are +46/-15, net +31. Test C/CMake changes are
+309/-14, net +295, including the new 280-line CPU test. There is one private
mask helper, no new public interface, state mirror, thread or execution path.
The whole-diff review checked all reserved-mask consumers, reset signature,
metadata/dispatch and the two board metadata expectations. Old profile masks
and production profile rejection remain unchanged.

The x86 complete run passed 597/597 in 43.64 seconds; its log is
`build/t559-s6-x86-unit.log`. Both widths include all five Apps, Core and
Shared units, including three registered desktop-labelled unit cases each.
That is not manual desktop/RDP or external firmware boot qualification.
Eight 0559 PC EXEs were rebuilt and deployed, each passing the real-product
INI identity integration, PE-width and embedded version checks. Both reusable
Ninja graphs finish on the default NXVM profile. Core manifests, documentation
governance and whitespace checks pass. No owner INI/media/ROM or MyNES changes.

| Artifact under `assets/` | SHA-256 |
| --- | --- |
| `my5160/my5160_0_5_0559_x64.exe` | `8FDCBCDF2D25109552DEBB91EB908694D1ED237E4186D23293BB1BEE1302FA90` |
| `my5160/my5160_0_5_0559_x86.exe` | `400564F3F3E757E9242E0142BF8454F4879567813196AB6476ED542C58D4598A` |
| `my5170/my5170_0_5_0559_x64.exe` | `4F1AE6E62700C5895ECD7FA7F5BBC3EEBB80697A424A53F657A7E0AFDB699576` |
| `my5170/my5170_0_5_0559_x86.exe` | `B81A97FCB73F5C19D0A674CDF521D414D85EB1A91663589D90E7CC56587817DA` |
| `mydeskpro386/mydeskpro386_0_5_0559_x64.exe` | `1F7D8C87AA80E05E489816BE6994C8E2E9D263E61CEAA434AC2BF1A65C306194` |
| `mydeskpro386/mydeskpro386_0_5_0559_x86.exe` | `C3F2C824E8B96A389F5ACC93615E7F2D7C6204C81A16485873D1C4C4B70EA8C4` |
| `nxvm/nxvm_0_5_0559_x64.exe` | `0A851E54E1926B9D5B9119752BC90CDFB4AE2DEF50499E313737ACB864AF13EC` |
| `nxvm/nxvm_0_5_0559_x86.exe` | `A91943D971CAE716F13D5B6073D43E44C860DB63155838DEB1D62476C6D42478` |
