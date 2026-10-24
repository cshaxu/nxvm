# T573 S1 — PC110 TFT First Firmware Boundary

## Reproducible Boundary

The normal App-local PC110 diagnostic runs the embedded, hash-validated TFT
firmware under the ordinary X86-NXVM executor.  It has no BIOS interception,
synthetic POST service, PC/AT facade, runtime firmware loader or alternate
execution path.

Both x64 and x86 Release runs reproduce the same route:

```text
first reset:  0x20000000 at F000:6044 (CPU shutdown)
post-reset:   F000:FFF0 ... F000:4CA9 (INT 10h)
INT 10h:      IVT[10h] = 0000:0000, then an invalid opcode in zero RAM
```

The copied execution trace proves that Core correctly performs the
processor-only reset and restarts the real BIOS at `F000:FFF0`. The BIOS then
reaches `F000:4CA9`, executes `INT 10h`, and obtains an all-zero vector from
the IVT:

```text
F000:4CA9  CD 10             INT 10h
0000:0000  1F EB 00 F0       stale/zero IVT target
```

At the reset boundary, a stopped-state copied memory read confirms that BDA
word `0040:0072` is `0000h`, the full Flash's C000 header is its real bank-0
loader (`55 AA 00 EB`), and no instruction retires from C000 before or after
reset (`c000=0/0`). The counter for `F000:4CA9` is `int10=0/1` (pre/post).

## Rejected Narrow Probes

Two diagnostic-only probes were run and then removed; neither is product code.

| Probe | Result | Reason rejected |
| --- | --- | --- |
| Write BDA `0040:0072 = 1234h` after the stopped reset boundary | Changes later invalid-opcode address/count, but still reaches `F000:4CA9`; C000 remains unexecuted. | Warm-reset state is consequential but insufficient. |
| Map Flash bank 2 (`55 AA 47 EB`) directly at C000 | The VBIOS header becomes visible, but the real loader still never transfers there. | A static ROM alias is not the PC110's POST-controlled decode/shadow contract. |

## Source Classification

Two independent external PC110 research implementations identify BDA
`0040:0072 = 1234h` as the warm-reset contract before the same KBC-driven
CPU-only reset:

| Source | Relevant evidence | Grade |
| --- | --- | --- |
| `PC110-EMU/Sources/PC110Core/pc110_core.c` | `pc110_kbc_perform_cpu_reset()` writes `34 12` at physical `0472` before CPU reset. | L2 reference implementation |
| `Open-Source-PC110/Discovery/BIOS-Flash/readme.md` | Four 64 KiB Flash banks; bank 2 contains VBIOS and bank 3 system BIOS. Banks 0/1 are a compressed loader and are not raw CPU windows. | L2 reverse-engineered/live corroborated |
| `Open-Source-PC110/Discovery/Chipset/readme.md` | The VL82C420 controls C000--FFFF decode; POST configures C-segment shadow and runtime C000--C7FFF contains the VBIOS shadow. | L2 reverse-engineered/live corroborated |
| `PC110-QEMU` | Copies Flash offset `0x20000` to C000, but also uses a TCG POST completer and interrupt services. | L2 implementation reference; its completer is inadmissible |

The local ROM has no direct literal store of the simple encodings searched for
at `0040:0072`; that does not disprove its reliance on the contract, but it
means this is not yet an L3 ROM-derived fact. The external sources establish
the missing owner and broad behavior, but not a complete, independently
verifiable reset/default-register/write-sequence contract for a truthful
VL82C420 ROM/shadow device.

## Disposition

The failure is not an X86 decoder defect and not proof that `INT 10h` needs a
host interception. It is an unmodelled, profile-specific chipset/firmware
mapping boundary. A future receiver must establish one App-local VL82C420
C-segment bank/shadow contract from primary material or a sufficiently
complete live trace, then run the unchanged firmware normally. It must not
introduce firmware-address dispatch, an automatic warm-signature write, a
generic guest-memory backdoor, a POST completer or a second executor.

## Verification Performed

- x64 and x86 Release `mypc110-real-firmware-post-diagnostic` runs: same
  shutdown, BDA value, no-C000 route and post-reset #UD.
- x64 Debug trace: same path, including the `F000:4CA9 -> 0000:0000`
  transition.
- The probes and all product behavior remain unmodified. No PC110 executable
  is published and no guest-boot claim is made.
