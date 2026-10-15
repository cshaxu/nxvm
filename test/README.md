# Test ownership

| Directory | Owner and purpose |
| --- | --- |
| [lib](lib/README.md) | Independently reusable Lib tests and fixtures. |
| [emulator](emulator/README.md) | Independently reusable neutral Emulator tests and fake machine. |
| [x86-debug](x86-debug/) | Independently reusable x86 debugger and assembler tests. |
| [x86-ibmpc](x86-ibmpc/) | Shared PC command, keyboard and process-entry tests. |
| [x86-nxvm](x86-nxvm/) | NXVM information/speed contract tests. |
| [core](core/) | NXVM-family chips, execution, boards, machine/Product adapters and their common setup/integration support. |
| app-my5160, app-my5170, app-mydeskpro386, app-nxvm | Fixed-App assertions and App-owned integration/diagnostic cases. |
| [app-mynes](app-mynes/) | MyNES unit, integration and diagnostic cases; no PC-package dependency. |

Each shared package owns an independent CMake registration and its own
fixtures. Tests may consume inward production APIs and the common registration,
manifest and Types checks beside this file, not another package's test source.
Core and App tests follow their actual source owner. Four-PC-family setup
belongs under core/setup; App-specific assertions remain with that App.

Unit cases use code-owned inputs; setup prepares reusable external-test
conditions, integration validates actual product/guest paths, and diagnostic
tools are not silently counted as acceptance. Disposable media and INI fixtures
stay in the test workspace. Tests must not overwrite owner media, INI or
snapshots. Native desktop cases require appropriate desktop access; automated
desktop results do not imply manual RDP or complete guest GUI acceptance.

Root CMake registers the selected products and shared packages once. Reuse
the existing Ninja/ccache graphs and product runners; do not create a fresh
qualification graph for each App. Current packets select exact routes,
budgets and external inputs. See the [NXVM guide](../docs/nxvm/README.md),
[source layout](../docs/nxvm/design/CODING.md), and
[MyNES guide](../docs/mynes/README.md) for product-specific execution.
