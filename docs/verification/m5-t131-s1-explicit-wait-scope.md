# M5 T131 S1: Explicit Wait Scope

`core_product_wait_milliseconds` and `core_product_utils_sleep` now receive a
caller-owned scope. The former thread-local selection and enter/leave APIs are
deleted. VM platform threads use their run context, firmware uses its injected
profile context, and debugger operations use their caller-owned debug context.

Windows GCC build, retained Console lifecycle, and
`nxvm-vm-dos-prompt-smoke D:\\home\\repos.hobby\\fdd.img` passed. The artifact
is `build/output/nxvm_0_5_0131.exe`
(`4F267598453BEC9F5CA1334AE54CEBD89F1A4110AFBAFB3BC4A73F33623BE177`).
