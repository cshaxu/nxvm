# T539 S41: BOUND Owner Migration

## Original-case receiving map

| Retired S54 family | Sole receiver and observation |
| --- | --- |
| 80186/80286/80386 word success; 80386 dword success | `cpu_bound_smoke.c` executes the instruction through `x86-cpu`, checking width, signed values, IP, flags, all GPRs, segment caches and unchanged bounds memory. |
| 80386 address-size and combined operand/address-size forms | `cpu_bound_smoke.c` checks 67 and 66+67 forms against word and dword bounds. |
| Register-only BOUND, LOCK BOUND, pre-386 size prefixes and bare 8086 BOUND | `cpu_bound_smoke.c` checks terminal #UD, no internal CPU error, full CPU rollback and unchanged bounds memory. It adds the previously missing 80386 32-bit register-only form. |
| Real-mode #BR on 80186 and 80386; signed lower-bound #BR on 80386 | `machine_bound_board_smoke.c` checks delivered vector, guest stack frame, unchanged GPR and handler execution through public machine operations. It additionally covers 80286 real-mode #BR. |
| Protected #BR, DS upper-limit #GP and SS upper-limit fault | `machine_bound_board_smoke.c` loads a guest GDT/IDT, checks exception identity, frame, rollback or terminal #DF as applicable, and handler execution where deliverable. |
| Pending IRQ after successful BOUND | `machine_bound_board_smoke.c` programs the real board PIC, checks delivery without an instruction shadow, IRQ frame, unchanged GPRs, ISR and IRR. |
| Six segment routes, signed equal lower/upper values, 67 SIB/SS route and VM86 | `cpu_bound_smoke.c` checks address/segment selection, signed equality, unchanged CPU state and bounds memory. |

The retired 899-line mixed-owner test is deleted. The CPU receiver links only
`x86-cpu`; the board receiver uses public Core-machine calls and the real
board PIC, not private CPU fields. No second BOUND implementation was added.
The original high half of EAX is retained in word-form success, fault and
IRQ cases, not silently zeroed by the new fixtures. The original source had
no instruction-timing assertions.

## 32-bit invalid-form finding

The original and S18-baseline-identical BOUND handler decoded an eight-byte
memory operand before rejecting ModRM `MOD=3`. The 16-bit register-only
encoding reached #UD, but `66 62 C0` reached internal `VCPUINS_EXCEPT_CE`
through `_d_modrm`/`_kdf_modrm` first. T337's established invalid-instruction
contract requires a terminal #UD. The BOUND owner now peeks the ModRM byte
through the existing code-segment read and rejects the register-only form
before memory-width decode. The former postdecode `flagMem` check is removed;
valid BOUND still uses its sole existing decoder and bounds algorithm. A
source sweep found no other instruction with the same doubled-width ModRM
decode form, so this fix remains BOUND-local and adds no public API.

## Verification and change audit

The eight code/test/build/gate paths add 609 and remove 922 lines (net -313):
the 899-line mixed source becomes 247 CPU and 335 board lines, with one
BOUND-local production correction and exact registration/gate updates. These
counts exclude documentation and generated EXEs. The direct-private `.c`
consumer count falls from 56 to 55; other S42-S51 groups remain pending.

Complete x64 and x86 Debug builds and repository-only unit suites pass
414/414 on each width. Both 67-target specialized gate aggregates pass,
including T317/T332/T337/T344, CPU/PIC authority, documentation governance
and the 413-row direct-compilation matrix. The x64 full-suite run initially
found a transient shared Window modal test failure and both widths found an
incorrect new IRQ FLAGS assertion. The board assertion now accounts for the
architecturally fixed FLAGS bit 1; both cases pass alone on both widths, and
the complete suites then pass. Shared source/test manifests are unchanged.

The production CPU source change rebuilt optimized, stripped T539 executables
for four runnable profiles on both host widths. Each build's deployment gate
checked its PE architecture and Release optimization; each deployed file's
SHA-256 equals its corresponding build-tree target. No adjacent `NXVM.ini`
has a content change. SHA-256 of the eight deployed binaries:

| Profile | x64 | x86 |
| --- | --- | --- |
| XT | `2846C90DE3670A21CA09BDE7331D757C58C8F8F05FE62FA8A6F75AD926168DF6` | `AB5F6CBFB358BD51861A673C02B03018225D5F1F04F4A461E5610086D464372A` |
| AT | `D076CC478C0840159085B67BCB1A7C788162DD8A2D1415C59731EFD19D255B67` | `E85E5EE7F69F390EBB58FD36AB6004B6329762E0291B104477DD5569F21C8A86` |
| Model 40 | `7E5A4F1A2E24F0BD87DC95E49AADB582D9387A5339390F0160D1C01BE87CC38C` | `F470FA456EEA1F67EB328A9EA01AE3ED7A9E47B9E009409525A82429CF091BCC` |
| Default PC/AT | `2BA139C7CCE55A38B535A7CD5E3F73C009E0B43FE66C5667BD7CEF8582F0B442` | `587FB6235907C849450D21615B679085107965119E360C716B66A2662E40561D` |

Actual source commit identity and push equality are recorded at the
coordinator's actual-commit review in CURRENT.md.
