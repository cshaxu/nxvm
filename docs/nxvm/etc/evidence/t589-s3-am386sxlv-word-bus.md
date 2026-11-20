# T589 S3: Am386SXLV 16-bit external bus

Baseline: `d6651f3a0`. AMD 21020 Rev A, printed page 28, specifies a
fixed 16-bit external data bus, BHE/BLE byte lane selects, and two-byte
prefetch transfers. The board-specific wait-state schedule remains open.
[AMD data sheet](https://www.amd.com/content/dam/amd/en/documents/archived-tech-docs/datasheets/21020.pdf)

- Reads, writes, page walks and fetches use the existing CPU transaction
  path. The aligned cycle helper chooses one word or one byte. At the
  24-bit boundary those same cycles preserve the S2 rollover. The refill
  continues to bound its request by the page and segment; the decoder
  can retain several bytes obtained from separate bus cycles.
- Port admission checks permission for the architectural operand before
  device callbacks. The existing input and output cycle routines own each
  transfer and its completion. On a failed transfer the current cycle
  cancels and no later cycle is sent. An accepted output prefix remains
  visible externally. SXLV input assembles successful lanes before
  updating the destination. V30 keeps its established odd-word lane order.
- Public interfaces, other device models and the guest clock are retained.
  Transfer sizes identify modeled bus activity. This work does not qualify
  absolute OB300 cycle timing, AMD SMM or real-ROM boot.

The existing address-span fixture verifies the aligned and odd byte, word
and dword memory rows, and an odd three-byte page-end refill. The scalar
port fixture checks both directions, each lane value and address, completion
ordering, and failure after an accepted prefix. The focused gate includes
the previous reset/mapping, V30 memory/port and string I/O tests. Full
qualification includes the fourteen receiving EXEs and both unit aggregates.

## Related path review

Searched CPU memory/port provider calls with
`rg -n 'bus->(read_memory|write_memory|transfer_port)|_kma_(read|write)_physical\(|_p_(input|output)\(' src/x86-core/cpu`
and reviewed the scalar, string, page-table, descriptor and refill callers.
Scalar and string I/O use the same operand admission. Paging and descriptor
traffic enters the same physical read/write split, and the instruction refill
keeps its existing page and segment bounds. V30 byte lanes and the older
profiles remain covered by the focused regression. DMA is board-originated
and does not enter the CPU word-lane path. AMD SMM requires the separate
address-space contract described below.

## S4 firmware handoff

Static inspection of the admitted, hash-verified System ROM 1.0 ABA
common-memory dump found a restore routine at dump offset `0x800884`.
The preceding instruction sequence clears bit 12 in the dwords at DS
offsets `0x2c` and `0x30`, sets EDI to zero and executes AMD RES3.
The routine also accesses indexed ports ECh/EDh. This supplies a real
guest consumer for a writable save area and a board-side follow-up for
port semantics. Isolated RES3 byte matches in the ordinary BIOS windows
were in table-like data and do not establish another execution path.

[AMD preliminary datasheet 16305](https://www.ardent-tool.com/CPU/docs/AMD/386/386SX/16305.pdf)
states that ES:EDI points to physical 60000h for RES3. This resolves
the shortened 6000h parenthetical in the later edition alongside its
60000h save region and diagram. The primary data sheets specify the
two external save spans and the transfer count. They do not enumerate
every register offset. The observed firmware edits make the two debug
slots a concrete compatibility requirement. S4 must distinguish a
verified slot from any remaining modeled layout. The dump offset does
not establish the HP board's SMI reset decode or ECh/EDh bit meanings.
Inspection extracted no permanent firmware files.

The next Core design must preserve three distinct routes. Ordinary memory
in AMD SMM uses SMIADS space, UMOV deliberately uses normal memory, and
ordinary IN/OUT still uses the existing I/O space. The 486SX's
`system_management_active` callback publishes a mode transition. It cannot
by itself distinguish these individual AMD transactions. Establish that
distinction before adding save/restore, and test same-address separation
with an isolated CPU/Machine fixture before choosing HP decode addresses.

## Qualification

Final focused results were 6/6 on each width. All fourteen published EXEs
match their reported hashes and expected PE machine identifiers. App versions
were retained. The x64 aggregate reached its 300-second outer deadline at
699/716, with no failing test result. The exact complement of the 699
passing names against the 716-name CTest inventory passed 17/17 in 249.28
seconds on the same unchanged x64 tree, completing 716/716. Three serial
verifiers accounted for most of the remaining wall time, including the
Lib Types fixture at 142.72 seconds. The x86 aggregate passed 716/716 in
293.89 seconds. Individual test budgets are unchanged. These are background
unit results. Real-ROM boot and desktop verification belong to later stages.

CPU production delta: +58/-28 (net +30). Existing test source and fixture
delta: +137/-5 (net +132). The two manifests add and remove four lines each.

Source manifest SHA-256: 77da41846c5ce8bcc634b04e0f16e8c8371215aa65ce3bdae331c05e3f847684

| App | Host | File | SHA-256 |
| --- | --- | --- | --- |
| my5160 | x64 | my5160_0_5_0582_x64.exe | 7d7fc2498e5802d314d8f7e11fd46be13c9ec2f38b4684cff3a67a359f900e31 |
| my5160 | x86 | my5160_0_5_0582_x86.exe | 56c077385b9b6101716579ee093278b6e9e264453a4be5e8a81fde1577a24010 |
| my5170 | x64 | my5170_0_5_0582_x64.exe | b02a89aba7f67d9e2083dd6dac066b4c3e46bb466c4cdc657e72b5be68b9b2f1 |
| my5170 | x86 | my5170_0_5_0582_x86.exe | 60a1c6fcd6743da500789118a79b9a1ac8b7af9fe6a862d419c773cc98cf44b3 |
| mydeskpro386 | x64 | mydeskpro386_0_5_0582_x64.exe | 988b2748f97bf6ebe7e3b51d387a041d221e55f60c35b8b6164eacc4aa4b7b95 |
| mydeskpro386 | x86 | mydeskpro386_0_5_0582_x86.exe | 4122e73fdce99ae9f25627c758696b08c4affc88f10c1d400c438179206cc50c |
| mynes | x64 | mynes_0_0_0044_x64.exe | b6f85c649f9fefdbfe600b47cc90dd3f23455efae90801efa36ba670e6b06ba0 |
| mynes | x86 | mynes_0_0_0044_x86.exe | 9c7da67f6929b8afe576c7493176346b3a1870d6c0e664d80eff3c1a85a2e0ba |
| mypc110 | x64 | mypc110_x64.exe | b09acd25c1833e633a03c5116c90d27591551f3d6f61fa7e092406848c108536 |
| mypc110 | x86 | mypc110_x86.exe | ac664e19b7e442714e5cbc3e82ae0a78ee92126eae4526cdbd76ba1da34e9378 |
| myraidenii | x64 | myraidenii_0_0_0007_x64.exe | eea8ddf0e4694e605d4c605a078ccac402e666e8e15f0530b03c314702dd116a |
| myraidenii | x86 | myraidenii_0_0_0007_x86.exe | 40744a0a3b45ed4eb41125471b09280c11c3ab5972d20cb7c96015b183540a06 |
| nxvm | x64 | nxvm_0_5_0582_x64.exe | 9d2282786f78eb3c29a76d9551916da98a462baa48fd0e7e340fb72d3a0ab881 |
| nxvm | x86 | nxvm_0_5_0582_x86.exe | 21d2161fcf1538f42c5a6bc3fbb9324e40a8ef49afd901a924b9cddc240dc167 |
