# M5 T80 S6 Verification

- Nested wait-scope restoration: `M5:T80:S6:WAIT-SCOPE:OK`.
- Facade scan reports no `core_product_wait_bind` production call site.
- Console FDD boot: `M5:T70:S2:DOS-PROMPT:OK`.
- Controlled window boot, stop, and shutdown: exit status `0` after the
  three-second FDD run.

All six T80 cutovers are complete. `nxvm_0_5_0080.exe` is the task artifact.
Its SHA-256 is `C7A08928D93631173AC2CEB9699D1C92A7F59C8BC958D4D391DBAF3AE4434F7C`.
