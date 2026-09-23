# Project Goals

MyNes aims to provide an understandable, testable NES
emulator with the ownership discipline and coding style established by NXVM.

1. Execute NES software with explicit, evidence-backed hardware behavior.
2. Separate host integration, machine integration, application policy and
   emulated hardware so each responsibility can be developed and tested clearly.
3. Provide a Windows application with reproducible builds and bounded failures.
4. Preserve a portable core and provenance-reviewed, MIT project source.
5. Maintain product-neutral Lib/Common source and tests that SoftPC and NXVM
   can adopt unchanged through their own machine and debugger adapters.
6. Advance compatibility through documented contracts and regression evidence,
   without treating a successful game boot as proof of complete hardware support.

Concrete ownership belongs to [Architecture](ARCHITECTURE.md); staged outcomes
belong to [Roadmap](ROADMAP.md).
