# M5 T130 S1: Explicit Console Context

Every private NXVM Console parser and command helper accepts the same explicit
`nxvm_product_console_context *` passed to `vm_product_console_main`. The
thread-local `consoleContext` selector is removed; Console grammar and target
callbacks are unchanged.

`nxvm-product-console-lifecycle-smoke` passed with the retained help and info
command sequence. The developer artifact is `build/output/nxvm_0_5_0130.exe`
(`64573575008CE7D0B2575548BFB798941247392A203248C6C8BF6714C77D0BD2`).
