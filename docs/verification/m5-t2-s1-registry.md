# M5 T2 S1 Registry Verification

`nxvm-runtime-registry-smoke` passed with GCC and emitted
`M5:T2:S1:REGISTRY:OK`. It exercised profile/provider ABI validation, duplicate
profile rejection, an unmet CPU capability floor, provider/profile compatibility
lookup, registry freeze, and rejected late mutation. No firmware bytes, host
path, external ROM provider, or new machine profile was introduced.
