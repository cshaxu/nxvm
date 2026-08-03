# M5 T125 S1: Profile Registry Removal

The former runtime descriptor/registry API was not consumed by NXVM or VDM
composition. It and its descriptor-only tests are removed. Default-profile
firmware remains directly composed by `vm_session`; VDM minimal composition
now has no inert profile metadata. T127 must reject any new registry,
descriptor, selector, or metadata layer that does not influence a formal
composition route.

Verification: `cmake --build --preset nxvm-current-gates-gcc` passed. The
task artifact is `build/output/nxvm_0_5_0125.exe`
(`E361149FD1DCC2C2055F43B240C290A81453255E47ADDD94F0E7AC8E2E511F20`).
