# T593 S5 generic Core read mapping reuse

Baseline `7c6106379`. The reference designs were read locally in 86Box
`4fef696` (`src/cpu/386_common.h`, `src/mem/mem.c`) and Bochs 2.6
(`cpu/access.cc`, `cpu/cpu.cc`). The implementation is NXVM-owned and
uses its existing memory backing, route geometry, and decode lifecycle.

## Selection and proof

For frozen route registrations, the candidate keeps the last ordinary
physical read-page decision. It admits only a full page of installed RAM
clear of address-line suppression, aliases, and device windows. Reads out
of the page or on an unqualified page continue through the existing route
resolver. A qualifying read uses the shared backing copy and parity code.
Writes continue through the existing write handler. No guest bytes,
operational query results or CPU linear translations are cached.

Address-line changes, runtime decode notifications, owner removal, RAM
allocation and reset invalidate the decision. Alias permission changes
within an already installed range cannot qualify that same page. The two
regression additions prime reads before moving an overlay and masking A20.

An earlier partial-provider overlap candidate reduced synthetic whole-range
queries from 17 to 1, but the short guest loop observed slower medians for
the fixed-ROM mode on both host widths. That candidate and the optional
MyOB300 card prototype are excluded from this batch.

## Short execution check

The scratch pair uses the same Release libraries and ABI layout. The stock
variant substitutes the original memory implementation, and the candidate
uses the cached production build. Both execute the same four-instruction
load, increment, store and branch loop with a neutral AM386SXLV Machine.
Each sample retires 200,000 instructions, advances 850,000 guest ticks and
leaves 50,000 in guest RAM. Three alternating trials per side use a high
resolution host timer. No ROM, device media, display or DOS boot is needed.

| Host | Code and data pages | Stock median | Candidate median |
| --- | --- | ---: | ---: |
| x64 | Same | 0.146004 s | 0.135327 s |
| x64 | Different | 0.131921 s | 0.115986 s |
| x86 | Same | 0.180916 s | 0.186106 s |
| x86 | Different | 0.199006 s | 0.160519 s |

The x86 same-page samples overlap substantially. This is a short throughput
check with three samples, so the measured direction does not establish a
small regression or an exact gain there. These samples neither qualify the
original MyOB300 workloads nor establish parity with another emulator.

Dual-width focused build and seven tests per width passed, covering memory
registration and inspection, prefetch, paging, RAM/port context, MyPC110 PCIC
and the MyOB300 board.

## Receiving qualification

Source delivery: `63cd192ad`. Cached Release builds reused the original
per-width Ninja trees. x64 and x86 each passed 717 unit tests and all
48 registered integrations in one aggregate per width, with the existing
deadlines. The product publish target built and deployed sixteen EXEs and
the artifact check verified each PE machine field and expected name/version.

The original MyOB300 usability timing, interactive RDP, and comparison
against reference emulators were not rerun for this batch. They remain
separate qualification work. Decoded execution sequences have no production
implementation in S5.

| Artifact | PE machine | SHA-256 |
| --- | --- | --- |
| `nxvm/nxvm_0_5_0593_x64.exe` | `8664` | `6b79985c22db0331653bc8ff85d79feea541ce6f3852417c176de8067c745b42` |
| `nxvm/nxvm_0_5_0593_x86.exe` | `014C` | `78185f200bbd338cd34888df053bcb060f7c433ff04daa8c038bd78d9b3746e1` |
| `my5160/my5160_0_5_0593_x64.exe` | `8664` | `20b936f348f290abded91d32ef496318603b10863ac2940f4e208e09352ee2ae` |
| `my5160/my5160_0_5_0593_x86.exe` | `014C` | `2b268862fe167277603b4f8cbe9e03113417648505159554283e1fdb765cc6ca` |
| `my5170/my5170_0_5_0593_x64.exe` | `8664` | `6d043396abacaf302d2d016db4140b31dcdd4a48cc0967a220b027973ca69fbd` |
| `my5170/my5170_0_5_0593_x86.exe` | `014C` | `99ed6f330faf46d84e541918c09689f2f0f19c5b8a961c648142b8f45d0edad8` |
| `mydeskpro386/mydeskpro386_0_5_0593_x64.exe` | `8664` | `d19c3a809bd1f7e905695f014e344d48e0acfe9df2f1c7ec06e7851f052bc7d8` |
| `mydeskpro386/mydeskpro386_0_5_0593_x86.exe` | `014C` | `48489afdfaa9590f1b1e17945e3f2409253767a1623545a492dd77a251d68a6b` |
| `mypc110/mypc110_x64.exe` | `8664` | `133c3cd69b46d77823dd3aa34639fd50a361b4f51ba2e4a3037f75b9a727934a` |
| `mypc110/mypc110_x86.exe` | `014C` | `1e9a18c228ac9765ca9d75944c81fc6cd1f46dc0d8da0290ac3cd8f7bf797c95` |
| `myob300/myob300_0_5_0593_x64.exe` | `8664` | `c0aacb5b256aa03223d255ff77c103ecbb143c8db0a6b409fddc5719a278170a` |
| `myob300/myob300_0_5_0593_x86.exe` | `014C` | `588e1eb2f2980f542a1423da7a4e5574f4b2102620c4357d539f66571a64d885` |
| `mynes/mynes_0_0_0044_x64.exe` | `8664` | `14ec659fd44cc59cd1bbdf4a224fc480c35aa5914d3daed04280edc7439c1354` |
| `mynes/mynes_0_0_0044_x86.exe` | `014C` | `b460e28f9ce04a5e4c4ab4507ee5fbf49ee8c5634a533e39625c78c1714e6f4a` |
| `myraidenii/myraidenii_0_0_0007_x64.exe` | `8664` | `4d37b4d86422dd78af3d5ed1f2237f65003ed9eb21783d9145beb209ffe18731` |
| `myraidenii/myraidenii_0_0_0007_x86.exe` | `014C` | `b31cd56c898afbca94ca85369881f183ce62e3cdb63ee1e246410491b780617e` |
