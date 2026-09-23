# Tool Layout

- `shared/` contains repository-wide governance and neutral maintenance tools.
- `nxvm/` contains NXVM-only build, test, research, and release tooling.
- `mynes/` contains MyNES-only build, test, and release tooling.

Product tools may consume their own product documentation and assets, but may
not define a second implementation of shared Lib, Common, or x86 behavior.
