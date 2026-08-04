# M5 T205: Console Adapter Cohesion

## Contract

`console_machine_adapter.c` owns the typed selected-session borrow helper as a
private implementation detail. It is the only direct
`core_product_session_manager_borrow_selected()` call in VM composition/session
sources. The Console machine-provider vtable and callback behavior are
unchanged.

## Evidence

- Focused Console/session CTest subset: 5/5 passed.
- `current-gates-gcc`: 42/42 CTest smoke and 19/19 static/structure gates;
  `M5:T205:S1:CONSOLE-ADAPTER-CLOSURE:OK` passed.
- Artifact: `build/output/nxvm_0_5_0205.exe`; Console `EXIT` status 0;
  SHA-256 `5EF205323D9A774E11A1CA30F4120C755729696380807180EE85D3A0542BBB6F`.
