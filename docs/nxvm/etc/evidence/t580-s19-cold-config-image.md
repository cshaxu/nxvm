# M5 T580 S19 — PC110 Cold Configuration Image

## Decision

MyPC110 initializes the existing address-neutral `x86_vl82c420` with an
App-owned L2 cold image before the real TFT BIOS starts.  No Core interface,
new device, port route, clock, or renderer is added.  The existing chip API
is the only mutation path; `x86_vl82c420_reset_protocol()` continues to reset
only protocol state, so a warm reset preserves the selected configuration
bank contents.

## Source and grade

The values are hardware observations, not imported third-party source code.
They are recorded in the external, read-only Open-Source-PC110 checkout:

| Input | SHA-256 |
| --- | --- |
| `Discovery/Chipset/scamp_config.txt` | `5C2B4F6AD6E451FC17C65B64ECBD820274B5E7A4AA49529886B187A956C2CBC5` |
| `Discovery/Chipset/block2_config.txt` | `EE53C5EF8EF0E898809FBE7D1D19018DDE4B351391477E179B4FB40CDE327E69` |
| `Discovery/Chipset/eced_config.txt` | `F099D32E762248709D81E9213828CEADBDC3943540DF74CFEA7D4A1FD434EEC4` |

The evidence is L2: the captures are live hardware state, not a vendor
power-on table.  The 20 MiB MyPC110 configuration uses the source-documented
EC/ED bank-geometry value `0xCC` at index `0x03`; the captured no-module
value is intentionally not used.  This is not a claim of physical timing or
complete chipset emulation.

## S18 post-C000 disposition

The previously proposed post-C000 first-I/O diagnostic was not retained.
Existing two-width evidence proves the finite `F000:EA86..EA92` port-61 wait
returns.  The verified BIOS bytes immediately after that return execute
`F000:EA74..EA7F`, which performs only `in 61h` and `out 61h`; that receiver
already exists.  A new test would therefore only duplicate an established
route and would not select a panel, power, storage, or lifecycle receiver.

## Regression boundary

`config_route_smoke` asserts the SCAMP signature, block2 timing/cold strap,
and EC/ED geometry/ROM configuration through the actual App port routes, and
asserts representative values survive a machine reset.  It does not claim
the real firmware completes POST or that every table register has a decoded
meaning.

## Two-width result

Both x64 and x86 passed all eight MyPC110 unit tests.  The existing bounded
real-firmware observation also produced the same result on both widths:

```text
status=0 reason=2 pc=000f6064 executed=1712815 resets=0 exceptions=0
cr0=60000011 cs=0040/000f0000 ds=0008/0000d8a0
```

The existing reset/C000 diagnostic likewise completed with no fault on both
widths, reached its established `F000:EA86` port-61 loop, and returned to
`F000:EA84`.  These results prove that the cold image preserves the admitted
path; they do not prove panel/power behavior, a guest boot, or full POST.
