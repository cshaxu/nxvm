# Fixed product session templates

Each retained machine has one canonical runtime template:

- `ibm-5160-model-268-360k.ini`
- `ibm-5170-model-339-1200k.ini`
- `compaq-deskpro-386-model-40-1200k.ini`
- `default-pc-at-80386-1440k-hdd.ini`

The executable selects its machine at build time.  `NXVM.ini` only supplies
runtime memory, media access and presentation settings; it never selects a
CPU, firmware image or another machine profile.  Paths are relative to the
INI file.

Release builds deploy an EXE and its adjusted `NXVM.ini` together under both
`build/output/<profile>/` and `assets/sessions/<profile>/`.  Those generated
directories are ignored: the four root `.ini` files remain the canonical
templates used by CMake and integration tests.
