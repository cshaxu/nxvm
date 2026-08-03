# M5 T131 S2: Explicit Wait Smoke

The full GCC build found that `core_product_wait_smoke` still named the deleted
ambient wait enter/leave API. It now invokes two caller-owned scopes directly
and verifies that calls remain isolated, including a null-scope no-op.

The complete Windows GCC build and all 54 configured smoke executables pass.
