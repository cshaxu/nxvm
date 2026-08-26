# Td S153 Unit-Task Queue Hardening

`M5:TD:S153:UNIT-TASK-QUEUE-HARDENING:ACCEPTED`

| Candidate | Frozen unit | Required stages | Aggregate boundary |
| --- | --- | --- | --- |
| T486 | Intel 8088 instruction/timing surface | Original source, List 1, List 2, one private Core-owner batch | No board or controller behavior. |
| XT board | IBM 5160 oscillator, memory, ROM and ISA contract | Original source, List 1, List 2, one Core transaction/time-owner batch | Consumes CPU and controller contracts. |
| XT PIC | 8259A plus selected IBM wiring | Original source, List 1, List 2, one Core PIC-owner batch | No AT cascade or APIC. |
| XT DMA | 8237A plus selected IBM wiring | Original source, List 1, List 2, one Core DMA-owner batch | No FDC or Xebec personality. |
| XT PIT | 8254 plus selected IBM wiring | Original source, List 1, List 2, one Core PIT-owner batch | No RTC or host pacing. |
| XT PPI | 8255, keyboard and NMI route | Original source, List 1, List 2, one Core PPI/NMI-owner batch | No AT 8042 route. |
| XT FDC | 8272A plus selected floppy-media chain | Original source, List 1, List 2, one Core FDC/media-owner batch | No separate DMA or media owner. |
| XT video | CGA through VADP | Original source, List 1, List 2, one VADP-owner batch | No renderer-owned state or EGA/VGA work. |
| XT fixed disk | Xebec/ST-506 chain | Original source, List 1, List 2, one Core HDC/media-owner batch | No ATA shim or duplicate CHS/cache. |
| XT final audit | IBM 5160-268 composition | Audit only | Cannot repair a unit; transfers a gap to its earliest receiver. |
| Pre-Windows audit | M5 selected profile composition | Audit only | Cannot repair a unit; blocks later consumers. |

The review found no candidate that combines a CPU or controller implementation
with another controller's lifecycle. The board candidate has a separate
machine-integration responsibility and each selected controller/display has a
separate earliest implementation owner. T486 S3 was paused before its first
P so the revised hard rule applies before its List-2 audit resumes.

Review covers Current, Queue, the shared admission program, T486 and every
queued XT implementation proposal. The two later entries are expressly audits,
not repair receivers. No code, build input, artifact, source material or
runtime ownership changed.
