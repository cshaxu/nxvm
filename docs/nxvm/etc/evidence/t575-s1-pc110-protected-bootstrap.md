# T575 S1: PC110 protected-bootstrap segment-cache boundary

## Reproducer

The admitted external TFT firmware is executed through the ordinary MyPC110
construction and Core executor.  An execute watchpoint at `F6045` stops after
the preceding `POP DS` at `F6044`; the retirement observer preserves the
pre-instruction snapshot.  The diagnostic is
`mypc110.diagnostic/real-firmware-post` and uses only the existing Core debug
and retirement-observation contracts.

Both x64 and x86 report the same boundary:

```text
CR0=60000011  CS=0040/F0000  ES=0048/100000
DS=0008/00D8A0  SS=0058/000000  ESP=0000FFF3
EIP=00006044  GDTR=000000D8A0 limit=00000088
GDT[0048]=0000000000000000
```

After executing the instruction, both widths report `DS=0048/00100000` and
stop at `F6048` without a delivered exception.

## Disposition

The strict protected-mode loader correctly rejects the zero descriptor; this
is not a ROM, CMOS, mapping, interrupt or PC/AT-facade defect.  Open-Source
PC110 and PC110-QEMU independently describe the firmware as relying on a
loose protected-mode behavior, but their source is L2 research only and is not
copied.

Core now provides one explicit L2 board choice:
`X86_CORE_CPU_PROTECTED_DATA_RELOAD_VISIBLE_CACHE`.  It applies only to a
protected data-segment reload where the descriptor is exactly zero and another
visible data register already has the exact selector with a valid non-code
cache.  It copies that existing cache; it never synthesizes a descriptor,
relaxes code/system/privilege rules, recognises PC110, or changes the strict
default.  MyPC110 is the sole current opt-in consumer.

## Verification

- `x86-core.cpu/cpu_segment_selector` proves strict rejection and the narrow
  cache-reload success case for 80386 and 80486SX.
- `mypc110.diagnostic/real-firmware-post` proves the real firmware reaches and
  completes this exact instruction through its App-selected configuration.
- Both tests passed in `gcc-x64-release` and `gcc-x86-release`.

This closes only the first protected-bootstrap segment-cache boundary.  It does
not establish C000 video execution, POST, setup, storage, DOS boot or a
release-qualified PC110 product.

## Qualification And Receiving Artifacts

The stable Ninja/ccache graphs completed the entire repository-only unit suite
and the owner-managed external-ROM/disk integration suite after the final
source and manifest changes:

| Architecture | Unit suite | Integration suite |
| --- | --- | --- |
| x64 | 661/661 | 27/27 |
| x86 | 661/661 | 27/27 |

`X86 Core` is consumed by all four runnable PC Apps, so both profiles of each
receiving App were rebuilt and deployed at the T575 identity. The PC110 App is
deliberately excluded: it is not yet an admitted runnable product.

| Artifact | PE machine | SHA-256 |
| --- | --- | --- |
| `my5160_0_5_0575_x64.exe` | `8664` | `6F849CA168A4F52FDE5F8C1A2CA4CD6BC8495DD502F75773C931E1EA93F6AC72` |
| `my5160_0_5_0575_x86.exe` | `014C` | `D7D9CB8AB6DF808D56E6D328180530405A2224F76BA592722B45D569968FB851` |
| `my5170_0_5_0575_x64.exe` | `8664` | `41AF350ECFD907388E729A735486EF3B254B8B267CC5886983B6DEEFEA9CE705` |
| `my5170_0_5_0575_x86.exe` | `014C` | `5A477F9F5736A623215C070A30C60CA90103DC163A23408C44AAE7B1A7D4AA16` |
| `mydeskpro386_0_5_0575_x64.exe` | `8664` | `399583625A17FE845AECE03F815378C6E87EEBA9C2ED46E38042A029DC765D2B` |
| `mydeskpro386_0_5_0575_x86.exe` | `014C` | `92227BF927AEDA62B60812D0159B2E92FC1BE938F88901DED6C7DF7B7D68A08A` |
| `nxvm_0_5_0575_x64.exe` | `8664` | `2FE4B9E952A98F72BE8995E36F0B06BC67301D18EDA9747C5D3FD522E2C43417` |
| `nxvm_0_5_0575_x86.exe` | `014C` | `895E9DD0CE491D7E5E06489700E74C70A034C52DBAA556C6B2A79FA4F6761752` |

The physical PC110 panel, RDP and a guest boot were not claimed or manually
verified by this receiver.
