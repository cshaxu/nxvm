# M5 T121 S1: Core Executor Authority

`CORE_MACHINE_PROFILE_CUSTOM` now creates its executor immediately and has no
test-memory or test-port allocation. `CORE_MACHINE_PROFILE_TEST_MINIMAL` owns
only clearly named contract-fixture state and cannot execute a guest budget;
its `run` result is `NTVDM64_STATUS_UNSUPPORTED`. It remains available for
memory, port, trace, lifecycle, and VDM-minimal contract tests.

MinGW-w64 GCC 16.1.0 gates passed: core contract, lifecycle, trace, executor
run, VM executor-storage, and `D:/fdd.img` DOS prompt.

Developer artifact: `build/output/nxvm_0_5_0121.exe`, SHA-256
`03FE060751FCAE5AC437651A0301CBAD66949AD01702F961BB12F3AD29833FAC`.
