# M5 T312: System-Extension No-Admission

T312 closes the trace-driven 80386 system-extension candidate as a
withdraw/no-op. S1 P0 (`67e73ca8`) audited the candidate families and S2
(`6bbeb8b5`) separated existing handlers and focused probes from actual VM/M5
consumers.

No reproducible VM/M5 consumer required 32-bit TSS switching, task gates or
nested return, broader LDT behavior, virtual-8086, guest debug/test registers,
or another remaining system-instruction form. T312 therefore made no runtime,
CMake, artifact, or product change.

Future admission requires the candidate-specific bounded checkpoint or trace
thresholds in [T312 evidence](../etc/evidence/t312-system-extension-admission.md).
This is not a claim that any system-extension family is complete. M6 remains
unadmitted.

T312 is closed pending no further implementation action; the accepted artifact
baseline remains `build/output/nxvm_0_5_0311.exe`.
