# IBM PC shared package

IBM PC board mechanisms and PC product adaptation live here, separately from
neutral x86 chips/Core/tools. No App-private header or native platform API is
allowed. Existing C symbols/ABI and hardware algorithms remain unchanged.

| Component | Sole responsibility |
| --- | --- |
| board-common | Common board construction/reset/teardown, ports, guest-clock conversion, deadlines, IRQ aggregation, media/display providers and floppy/ROM/profile validation. |
| board-xt | XT PPI/keyboard, DIP and speaker/NMI wiring. |
| board-at | AT KBC/AUX/A20/reset and planar parity/Port B wiring. |
| machine | Bounded executor, pacing, copied input/frame/debug adaptation and Storage-backed media lifetime. |
| product | INI syntax/request, entry/banner, command/hotkey policy and Common composition with an injected App factory. |

Board families consume only public x86/Core/chip contracts and Types; they
never read board-common private layouts. Board-common composes their public
contracts without selecting a Core implementation. Composition selects one
production or observation-enabled Core/board pair. Core alone advances guest
time and owns the board attachment teardown; the Machine adapter borrows it.
Media/display provider contexts remain borrowed until teardown, with one
freeze/publication/rollback path. Concrete topology, firmware, immutable
identity and genuine model-specific state remain App-owned.

Machine consumes board-common, x86/Core/debug, Common Machine and Lib services.
It also consumes Product's copied request/factory contracts, not Product's CLI.
Product consumes Common Session/UI and x86 Debug, not a concrete Machine
implementation. App binds these two contracts; no reverse x86 dependency,
second queue, native presenter or duplicate file backend is introduced.

Headers ending in _interface.h are public; all other headers are owner-local.
Machine's media subdirectory is the same owner, not another component.

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
