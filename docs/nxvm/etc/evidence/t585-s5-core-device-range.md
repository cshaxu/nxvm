# M5 T585 S5 — Runtime Core device range

An enabled Ricoh memory window may move anywhere in the 24-bit ISA memory
space. Registering one permanent 16-MiB provider would cause Core's existing
plain-span test to treat ordinary RAM within that range as potentially
device-backed even when no card is present. Installing and removing routes
after configuration is frozen is not an allowed Core path.

The existing Core memory owner now permits the owning execution thread to
move or disable an already-installed, single non-raw overlay provider. Zero
bytes disables it without keeping a false active span. The operation does not
allocate another route or backing store, and invalidates the existing CPU
memory view for the old and new ranges. It validates the address limit and
rejects unknown or ambiguous owners. The provider still participates in the
same existing query/read/write resolution and priority rules.
Disabling the range preserves plain-RAM eligibility; the registered provider
still adds an entry to the existing scan, so this is not a claim of zero
runtime overhead. No performance benchmark is claimed in this S.
An unchanged range is a no-op: a device that changes the bytes or meaning of
an unchanged window must use the existing `signal_memory_decode_change` path.

The existing Core registration test now uses an actual 486 CPU and its data
cache: first read ordinary RAM, enable a provider at that address, move it,
and observe RAM again. Repeated guest reads prove a cache hit by observing that
the device callback is not called again; after the provider moves, a guest
read must return RAM rather than its previously cached device byte. It also verifies
provider writes, an unmapped RAM cache fill, disabled range, ambiguous or
unknown owner rejection and overflow. This verifies the Core
mechanism rather than a mock of the cache. The PC110 card attachment, address
overlap policy, separate zero-wait bit and the pre-existing masked-span/provider
fallback debt remain outside this bounded mechanism and require the next
receiver; this S does not claim guest-visible card support.

An earlier unit run was still using its output file when the stronger cache
assertions were added. It was stopped by the executor, its child-process exit
was checked, and it is not counted as final qualification; the final tests use
distinct logs and the strengthened test binary.

The first complete x64 unit run failed `x86-core.machine/chip-boundary`:
the initial test checked CPU cache internals using a private CPU header from
the Machine test. The test was corrected to observe cache hits through the
actual device callback, removing that forbidden dependency. Focused boundary
and cache tests then passed on both widths; the failed full run is retained as
failure evidence and is not counted as final qualification.

The corrected full unit suites passed 712/712 on x64 and x86. Registered
integrations passed 48/48 per width on an isolated desktop. XT, AT and DeskPro
selected integrations passed 3/3, 5/5 and 6/6 respectively on each width;
the selected-ROM MyRaidenII startup and deployed Console probes also passed
on both widths. These probes do not establish gameplay or physical input.
The two stable Release build trees used the repository ccache and were
restored to the default PC profile. All 14 product executables were rebuilt
and verified; MyNES and MyRaidenII outputs were unchanged.

The reviewed Shared change is `175aff537`; product artifacts and this
record follow in the NXVM-scoped commit. Original owner INIs, ROM masters and
disk images were not modified. The previously recorded test failures and
interrupted preliminary run remain part of this evidence.

## Released executable identities

| Product artifact | PE width | SHA-256 |
| --- | --- | --- |
| my5160/my5160_0_5_0582_x64.exe | x64 | `aca76f60ff128d18abfcaf1cc75057e906f79cf24c354eb3ddc78335fe3c40b6` |
| my5160/my5160_0_5_0582_x86.exe | x86 | `13afe39dafcadbec8305f002e265153a2ae152ccb20fd819afba4bf3f0a3a633` |
| my5170/my5170_0_5_0582_x64.exe | x64 | `15979721aac20c2ac22607a2a02a7bc414f5556476c6ddc570bbfbc68829e42f` |
| my5170/my5170_0_5_0582_x86.exe | x86 | `1bacc47c3fb7b67d7fdbcb7760f589b1dba9e4689becda2dc816ae7cc52f0729` |
| mydeskpro386/mydeskpro386_0_5_0582_x64.exe | x64 | `5295bade5d9ea64391665a49802fd05dc988845dbfdfd134a4220b547c96bfe4` |
| mydeskpro386/mydeskpro386_0_5_0582_x86.exe | x86 | `3108962822b5b3d88c785bb529e1204de8ce342cfab5dc29d19e1a2b989577f2` |
| mynes/mynes_0_0_0044_x64.exe | x64 | `b499cd2583ecddd1ae01eed882e21d49d20187558e2138858e5a27117cf74fc8` |
| mynes/mynes_0_0_0044_x86.exe | x86 | `718617af2fbf6d49c36ae3bb10043dc3b2383e35440e7a371e503b8bd4b4ea16` |
| mypc110/mypc110_x64.exe | x64 | `816981b443f8a8116da153cecfaf52545e2908f6243c0d8df7b9d98bac041b8a` |
| mypc110/mypc110_x86.exe | x86 | `66b3c93e7d1a73dd44e5bd74c620323b2258d298f9fac03d7565407e38028108` |
| myraidenii/myraidenii_0_0_0007_x64.exe | x64 | `0e48ffde257e0f6094ed5dbccb3b03c5d2ea7e7fe8a01a0f27a11df1659fc637` |
| myraidenii/myraidenii_0_0_0007_x86.exe | x86 | `5d0181af40715d47630fe3feffbc7bbf2bff1aaaad3825e8d5e95fcfec99f650` |
| nxvm/nxvm_0_5_0582_x64.exe | x64 | `051782e462b8722f1a231162cc8e96a944d8487fc9c9159a483da45428ddbd11` |
| nxvm/nxvm_0_5_0582_x86.exe | x86 | `8e29ab9d153b61cf0549a3e50cea7778d35b8508e4065e552c1d94054b109e98` |
