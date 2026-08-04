# M5 T207: Linux Console Private Naming

## Contract

Linux Console remains one cohesive adapter file. Its private display, keyboard,
color, character, and utility helpers use the `vm_platform_linuxcon_*` owner
prefix. The prior function-like macros are replaced by typed private functions.

## Evidence

- Linux platform source-contract and adapter-hygiene static gates passed; the
  latter rejects retired local-dialect names and macros.
- `current-gates-gcc`: 42/42 CTest smoke and 19/19 static/structure gates.
- Linux native compilation is deferred to a POSIX host.
- Artifact: `build/output/nxvm_0_5_0207.exe`; Console `EXIT` status 0;
  SHA-256 `EC35BA9B8D4A7FEBAEB2E0C04609EFEB8E9B2098ABB7498E1D59647ADD5F4C1F`.
