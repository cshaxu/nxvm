# T582 S4: DMA arbitration audit

A task-owned wrapper observed the existing PC110 public-factory path on x64
Release, using private copied CMOS and overlay media. During the first
10,000,000 retired instructions, it counted 10,290,326 board DMA advance
calls, 79,987,423 elapsed DMA ticks, 1,407,774 single-tick calls and 8,882,552
multi-tick calls. Both controllers reported no unmasked request and no active
channel at every sampled call entry. This is an early-startup observation,
not proof of all operating phases; querying signals adds probe overhead.

A private candidate executes the original first tick and cascade update, then
checks both controllers' existing signal snapshots. If neither has a pending
unmasked request or active channel it returns; otherwise the original loop
continues. This intentionally does not rely on the cascade-gated pending
query. It introduces no state or API. It is not yet production code: prove
that masked requests and controller enable transitions cannot change without
an externally scheduled operation during the remaining ticks, and exercise
single/dual controller, cascade, active transfer, and callback cases.

| Width | Baseline ms | Candidate ms | Instructions | Guest ticks | PC |
| --- | ---: | ---: | ---: | ---: | --- |
| x64 | 21,782 | 18,714 | 10,000,000 | 79,964,294 | 0003dbc5 |
| x86 | 24,812 | 23,553 | 10,000,000 | 79,964,294 | 0003dbc5 |

Each is one sequential matched Release run with the same compile flags and
private inputs. These are bounded early workload measurements, not realtime
or interactive qualification. The candidate adds approximately eight lines
to board-base/dma_bus.c; use the existing DMA channel test for focused
regression. No production or Shared test changes have yet been made.

## Implemented boundary and focused proof

The board performs the original advance and cascade update first. Once both
controllers report no unmasked request and no active channel, another idle
tick cannot grant or execute a bus callback; masked hardware request bits
remain in the controller until an external port/request operation changes
them. Software requests remain in the existing pending calculation. Any
active transfer or request prevents the shortcut, including when a callback
changes a request on the first tick. The absent secondary controller is
represented by the existing inactive signal result. External clock/device
advancement remains the Core scheduler's owner, not this loop's.

Production diff is +6/-0 in board-base/dma_bus.c; the existing DMA fixture
adds +2/-0 to prove a masked request survives a 1024-tick idle interval and
then completes after unmask. Existing active-transfer, secondary-channel,
cascade, mask and disable cases remain; focused x64/x86 tests pass. Both
source and test manifests verify. Full dual-width qualification subsequently completed as recorded below.

## Receiving qualification

Both architectures completed 712 unit tests and 48 integration tests, plus
XT 3, AT 5 and DeskPro 6 additional profile integrations. Cached Ninja
Release graphs were restored to the default profile. The 14 EXEs were built,
PE-width checked and hashed. MyNES bytes remained unchanged. MyRaidenII
selected-set startup and deployed Console probes passed on both widths; this
does not establish gameplay or audio. Native interactive Window/RDP and
PC110 realtime playability remain unqualified.

| Artifact | SHA-256 |
| --- | --- |
| `assets/nxvm/nxvm_0_5_0582_x64.exe` | `D6E8285977E61F99BDD5FD67A4B0EB3BD7C0751DD1A131E08D339A74AE49DA52` |
| `assets/my5160/my5160_0_5_0582_x64.exe` | `DD9E3D93175285DDE0C062A67C80DE560ED7821C89AADEAD9D6907DCFFF13460` |
| `assets/my5170/my5170_0_5_0582_x64.exe` | `F0F6AD3B3690B3A4CD541832AE37D214EBAE8432A821EA1927C574D34A9DAC0B` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe` | `42FA154D23C4F3E85BC6DF4E4AA171DC07D1C5948EE655F11B8701A76BD2C2A6` |
| `assets/mypc110/mypc110_x64.exe` | `009B12169EC877DAAC2D64F8C81D93C07E1D38EA159924F130579B250D985D47` |
| `assets/myraidenii/myraidenii_0_0_0007_x64.exe` | `AB522F59E28DCB87A08EAAF650AA43DBAC3E599772F9F040C060EBE1132FA55D` |
| `assets/mynes/mynes_0_0_0044_x64.exe` | `B499CD2583ECDDD1AE01EED882E21D49D20187558E2138858E5A27117CF74FC8` |
| `assets/nxvm/nxvm_0_5_0582_x86.exe` | `979398727F4DE816D5013C9776C4A4EDB2DC819597C62CE6CC4E704E00D42EF4` |
| `assets/my5160/my5160_0_5_0582_x86.exe` | `2D54147CF729ECE5AE3EADBEB8D5EC251AABEB9E7BF4B446D77A6D37E8D00E8C` |
| `assets/my5170/my5170_0_5_0582_x86.exe` | `74935C1350CA21D9930E5622991AC80F22D0C62514689A9E9510ACC99C851C8F` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe` | `E806DA4003B000C5F80C1A06F674605E4CEB62633D1038087DAF5DCADB513D41` |
| `assets/mypc110/mypc110_x86.exe` | `F6FB5921988B56BA9490833D40B1B125CC28E03C724FB5742D98D873FDF0D5AF` |
| `assets/myraidenii/myraidenii_0_0_0007_x86.exe` | `D09843F52C30B8C287506AB8DA27E5197AD9D2C8E9BE43271713706E5DF8AD6F` |
| `assets/mynes/mynes_0_0_0044_x86.exe` | `718617AF2FBF6D49C36AE3BB10043DC3B2383E35440E7A371E503B8BD4B4EA16` |

Qualification log SHA-256: `9E39543DB139AC8C05030D8FED3A0EDCEA09E8978563F4EF933CFAA3B425460B`.

External receiver log SHA-256: `D829B6B9159B4FC435B77EA9664536C7FBF0141BD92E96C5ABA8C6E63833C585`.
