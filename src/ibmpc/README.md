# IBM PC shared package

IBM PC board mechanisms and PC product adaptation live here, separately from
neutral x86 chips/Core/tools. No App-private header or native platform API is
allowed. Existing C symbols/ABI and hardware algorithms remain unchanged.

| Component | Sole responsibility |
| --- | --- |
| board-common | Common board construction/reset/teardown, ports, guest-clock conversion, deadlines, IRQ aggregation, media/display providers and floppy/ROM/profile validation. |
| board-xt | XT PPI/keyboard, DIP and speaker/NMI wiring. |
| board-at | Immutable AT endpoint grammar, KBC/AUX/A20/reset and planar parity/Port B wiring. |
| machine | Construction finishing/publication, bounded executor, pacing, copied input/frame/debug adaptation and Storage-backed media lifetime. |
| product | INI syntax/request, entry/banner, command/hotkey policy and Common composition with an injected App factory. |

Board families consume only public x86/Core/chip contracts and Types; they
never read board-common private layouts. Board-common composes their public
contracts without selecting a Core implementation. Composition selects one
production or observation-enabled Core/board pair. Core alone advances guest
time and owns the board attachment teardown; the Machine adapter borrows it.
Media/display provider contexts remain borrowed until teardown, with one
freeze/publication/rollback path. Concrete topology, firmware, immutable
identity and genuine model-specific state remain App-owned.

AT port/IRQ/DRQ grammar is shared by three independently configured consumers.
Board-common projects it into its existing copied profile-contract values,
with one structural validator and atomic publication; firmware/media policy
and enabled endpoints are explicit App inputs. Board-at does not depend back
on board-common. Memory decode, display personality and genuine D4 remain
model-specific composition, not a 5170 profile inherited and overridden.

Machine consumes board-common, x86/Core/debug, Common Machine and Lib services.
Product adapts its copied request to the neutral Machine input and consumes
Machine's public creation/INFO/speed APIs. App supplies only fixed values and
its actual Profile preparation operation; Machine does not depend on Product.
There is no reverse x86 dependency, second queue, native presenter or
duplicate file backend.

Headers ending in _interface.h are public; all other headers are owner-local.
Machine's media subdirectory is the same owner, not another component.
Machine's input_interface.h owns copied construction options and borrowed asset
byte views, without a model identifier or concrete Profile declaration.
The App selects a constructor before preparation; its specific observation
contracts do not enter this neutral value boundary.
Preparation consumes one candidate: failure releases its actual App context,
success publishes the existing copied construction for Machine creation.
CMOS/font conversion and media bounds have one finishing path. Floppy
eligibility is an explicit App value, not a model or default-kind inference.

Keep src/ibmpc beside src/x86, src/common and src/lib. Standalone verification:

```text
cmake -S src/ibmpc -B build/ibmpc-corpus -DCMAKE_BUILD_TYPE=Release
cmake --build build/ibmpc-corpus
cmake --build build/ibmpc-corpus --target ibmpc-verify
```

The package requires the existing x86 tools for Product. Its complete
LF-normalized manifest and source/build DAG are independently verified.
Repository-only tests live in test/ibmpc; actual firmware/media scenarios
remain product-owned. No machine or timing qualification is implied by moving
an already accepted implementation.
