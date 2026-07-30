# Open Questions

1. Which components are guest code, machine runtime, host platform, or coupled?
2. Which startup dependencies require the original `NTVDM.EXE` host?
3. Which dependencies use private Windows ABI or version-specific behavior?
4. Can any useful configuration remain non-invasive and independently usable?
5. Does the compatibility value exceed the cost of expanding the owned DOS
   runtime?
