# M5 T11 S3 DMA Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths and hashes:
  `src/device/vdma.c` `a96504b4d67224b7eebe157b54fbcec4d552e4a441744bd8488a633e5934237e`;
  `src/device/vdma.h` `4f378375deabaea86b07171c149a23bcbecc0eca32f8cfb6e43202148180bd6c`.
- M5 destinations: `src/machine/core/vdma.c`
  `b1f9ccaeb55096ae96a6e97417b42878198968b1fb18d10a8f03ec0a0956345c`;
  `src/machine/core/vdma.h`
  `5bd07debcd706be3a9d58bcc127ed10a8f158baaa52015ecc3c666db0cf0a5d9`.
- Changes: preserved DMA logic and copyright notices; adjusted include paths,
  added a forwarding header, and added the implementation to the machine-core
  CMake source set.
- Verification: `docs/verification/m5-t11-s3-dma-move.md`.
