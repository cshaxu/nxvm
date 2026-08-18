# M5 T411: CPU Prefetch Window

T411 accepts P1 `a9be57f8`: a Core-private, persistent 15-byte instruction
prefetch window. It refills through the existing T409/T410 provenance and
external-cycle paths, supplies consumed instruction bytes from the window, and
invalidates after reset or a changed sequential CS:EIP expectation. No VM
interface, duplicate memory transaction route, hardware timing scalar, or
DeskPro-specific publication was introduced.

Focused transaction proof covers a control transfer, reset and replacement code;
T359 S4 and the 80286 instruction timing ledger preserve the existing timing
contract. The isolated GCC current-gate replay and documentation governance gate
passed. The developer artifact is `vm-0-5-0411`, SHA-256
`E4A0786E8C80CFCCFBB37EE7BB14B502DDBA5A02C825041B8FA89A3074DFC366`.

This closes only the prefetch prerequisite. Original D4 2 KiB page, row-hit and
row-miss waits, BWAIT, write/DMA/refresh arbitration and a Model-40 L3 decision
remain in the DeskPro physical-cycle proposal.