# M1 NXVM Source Provenance

## Identity

- Subsystem: whole temporary NXVM machine baseline.
- Date: 2026-07-31.
- Milestone, task, and subtask: M1 T2 S1.
- Source repository and remote: sibling `../nxvm`; `https://github.com/cshaxu/nxvm.git`.
- Source commit: `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`.
- Source license: MIT under copyright-holder authorization recorded in `docs/source-policy.md`; the sibling checkout historical license is captured in `docs/baselines/nxvm-6d6b7d7.md`.
- Import class: copied.

## Scope

All 75 files below were copied byte-for-byte from NXVM `src/` into the temporary M1 baseline and preserved in the immutable M1 snapshot. Linux platform units remain retained source but are excluded from the Windows target. Excluded neighboring content: `bochx/`, root build files, root documentation, guest media, and `*.com` payloads.

## Compliance

- Copyright and license notices preserved: every imported source unit retains its Neko 2012-2014 copyright header.
- Third-party notice update: no independent third-party notice was found in the imported source tree.
- Asset exclusion review: no disk image, ROM, Microsoft binary, or guest payload was imported.
- Source authorization: NXVM copyright-holder authorization for root MIT import, as recorded by project policy.
- Intentional deviations after the M1 snapshot: only
  `src/nxvm-baseline/main.c` changes the startup banner to the approved
  `0.4.015d.m1t2s1` build identity. Its source SHA-256 is
  `ec874f6c173e3d8198e321f0eb147ca596a3c391fcf8aa91e914f163be7c93b0`;
  its current destination SHA-256 is
  `ad0dfc4e0b474da6afef7640d935236dc8e1db01abad59766ba794c98bb8246d`.
- Git text/whitespace policy: `src/nxvm-baseline/**` is excluded from text
  conversion and whitespace checking because its snapshot upstream bytes contain
  CRLF and pre-existing space/tab indentation. This does not exempt
  project-owned code or alter imported bytes.

## Per-File Manifest

| Source path | Destination path | SHA-256 |
| --- | --- | --- |
| `src/console.c` | `src/nxvm-baseline/console.c` | `9efdc2822f4ceef5b772b59d69a2669d88ee050508ae76b91cd8e3aa8c4203fd` |
| `src/console.h` | `src/nxvm-baseline/console.h` | `9a0833dd5fd4fdb5f8ea3153d891a5ab1716859816a59f41d07d978f7f0d7fdf` |
| `src/debug.c` | `src/nxvm-baseline/debug.c` | `358f82b3b858b754b2b686cf734d83d045ca469d7f41027cf3bc3dd8f886013c` |
| `src/debug.h` | `src/nxvm-baseline/debug.h` | `63bb17b4db5b07baa06757bb83966c7b90a0c662a839a1d97f106edcf1bfa949` |
| `src/device/device.c` | `src/nxvm-baseline/device/device.c` | `e36141d957de3667e9acb850b4a188d33eb52f862621dc726e6cc37ddc2c61ee` |
| `src/device/device.h` | `src/nxvm-baseline/device/device.h` | `82d72413d37e0146b7c5d1a88673d409c1295213dcd1a631322c0678e827a18b` |
| `src/device/qdx/qdcga.c` | `src/nxvm-baseline/device/qdx/qdcga.c` | `18959ae2a9434c3618d4bdcd99b3152e3866ae4b8a5fa9e37faaf0e3e253ffe9` |
| `src/device/qdx/qdcga.h` | `src/nxvm-baseline/device/qdx/qdcga.h` | `1d9756a47d8ec08025d43d1524278a0416814aa3ed91a73647ae4ca44d2ba96a` |
| `src/device/qdx/qddisk.c` | `src/nxvm-baseline/device/qdx/qddisk.c` | `218c43d6e667d7dab3580d8d936b178953b813ff527922e37117b4fac5be4965` |
| `src/device/qdx/qddisk.h` | `src/nxvm-baseline/device/qdx/qddisk.h` | `6548a5861cbbcd82b01068fdbba93082e9e2f9f3d8890068d151eae8d9ddc64c` |
| `src/device/qdx/qdkeyb.c` | `src/nxvm-baseline/device/qdx/qdkeyb.c` | `edc86fc26786e18d90dc630500b5fe2647353a9bb0240fd3e65f32d3b9c9247e` |
| `src/device/qdx/qdkeyb.h` | `src/nxvm-baseline/device/qdx/qdkeyb.h` | `5169503f59323809c36e85f1cc7ca10de18e856805a7e1089ec6d870f4dd3a07` |
| `src/device/qdx/qdx.c` | `src/nxvm-baseline/device/qdx/qdx.c` | `3a3efcb6036783e2976948e7eaf840e682dd3e12a6426567f0a621b191b8e1aa` |
| `src/device/qdx/qdx.h` | `src/nxvm-baseline/device/qdx/qdx.h` | `8cba5e34f52ca59023b1bc240d73b3d139421e7dd7e3646b95f97c2bd432e895` |
| `src/device/vbios.c` | `src/nxvm-baseline/device/vbios.c` | `e35d89471192974ad894197af95ab70c5353ae837b8ff28cfcec48df82379b39` |
| `src/device/vbios.h` | `src/nxvm-baseline/device/vbios.h` | `ac519a4462538b63b8e4a22d6024d8d1b72cf7e079279a5de62eac9c600af274` |
| `src/device/vcmos.c` | `src/nxvm-baseline/device/vcmos.c` | `1c3c67767a6d2d2912ac41720c4032fb07e0f71d00814462c1b8f751785ad0b5` |
| `src/device/vcmos.h` | `src/nxvm-baseline/device/vcmos.h` | `33552c1d4339b9471a89030f936f09fa5009a0a13e0e5dacaa1527ebcdf23f74` |
| `src/device/vcpu.c` | `src/nxvm-baseline/device/vcpu.c` | `cffb72b6ffd69e5a24c98b3f3e7be62af0cba466a499bbae14c507b45fd20484` |
| `src/device/vcpu.h` | `src/nxvm-baseline/device/vcpu.h` | `d57c9f7dd12ce656374c7338e4103b8a5fe2303008ef33da579539af236eef58` |
| `src/device/vcpuins.c` | `src/nxvm-baseline/device/vcpuins.c` | `f29b15e79d7e0d2073f49bb6ae6878d0d403c6b7907413168865e674c2472730` |
| `src/device/vcpuins.h` | `src/nxvm-baseline/device/vcpuins.h` | `1cc76ffcb93bc7b50400cb96b131b54b1182b85ed5b87091e7d3cd0bdc453b87` |
| `src/device/vdebug.c` | `src/nxvm-baseline/device/vdebug.c` | `d8459e926b70de31b38351d71acffa01ec330fd290261960b4eaaf1af020ca8a` |
| `src/device/vdebug.h` | `src/nxvm-baseline/device/vdebug.h` | `939705cb1827ea0959c41ed638e7995ffb15b74075d1cb8c9835c3770dc14b51` |
| `src/device/vdma.c` | `src/nxvm-baseline/device/vdma.c` | `a96504b4d67224b7eebe157b54fbcec4d552e4a441744bd8488a633e5934237e` |
| `src/device/vdma.h` | `src/nxvm-baseline/device/vdma.h` | `4f378375deabaea86b07171c149a23bcbecc0eca32f8cfb6e43202148180bd6c` |
| `src/device/vfdc.c` | `src/nxvm-baseline/device/vfdc.c` | `99d1efe23d06cdcb1940dea632bbf8239d644dea92ffa0bbb3ba3737c3872534` |
| `src/device/vfdc.h` | `src/nxvm-baseline/device/vfdc.h` | `e709014475b0b58f9b35973d52745eb37ec18d21045ac9c8227ca251bb637e94` |
| `src/device/vfdd.c` | `src/nxvm-baseline/device/vfdd.c` | `83a27f0aac3fe546f3e2216d6f3380d360c595807f773d1599c95d55f11feced` |
| `src/device/vfdd.h` | `src/nxvm-baseline/device/vfdd.h` | `69d4b3b27f22e1be36f7bad94c2895171cbc9d5d3f8ec9f90e30329fc99c397d` |
| `src/device/vglobal.h` | `src/nxvm-baseline/device/vglobal.h` | `b86223d94f3161866df5b1df763479cb23b6fcaf8a5fbec112814047df3f1b59` |
| `src/device/vhdc.c` | `src/nxvm-baseline/device/vhdc.c` | `782c337a65d2e1453dbeee8e6c42170b4c66036fed3689c24b1243e9ed77d4c9` |
| `src/device/vhdc.h` | `src/nxvm-baseline/device/vhdc.h` | `3831df0d8d5eb1faebc06825df88e24ab950c73c3881378e213b02fcc2425328` |
| `src/device/vhdd.c` | `src/nxvm-baseline/device/vhdd.c` | `e07ed4cb0f8346eb6a136947ba6a81ccd4609d4efb55523ccf5c8b0bc7c485f8` |
| `src/device/vhdd.h` | `src/nxvm-baseline/device/vhdd.h` | `59651423f245f0b51c2292a1a67963ddafcdbddfafceebcf0ac15d5c7e68c9b1` |
| `src/device/vkbc.c` | `src/nxvm-baseline/device/vkbc.c` | `91ca494a6751b5822f3fb1f33ca59cba0dbae472c510e01cb4389fd1b2f83fa7` |
| `src/device/vkbc.h` | `src/nxvm-baseline/device/vkbc.h` | `bc8db00cbebd910673b145bcd5dd3a0c21759dfdc43f29824be2e2b73d6539a5` |
| `src/device/vmachine.c` | `src/nxvm-baseline/device/vmachine.c` | `1370145753fde187837e9b0395ad1fa90bf3484d99277b9c77ced2aa56abfacc` |
| `src/device/vmachine.h` | `src/nxvm-baseline/device/vmachine.h` | `19900bd57e3c09938cab61bfc93f0c6000a3ea4b30b271f99c1f02e2cb9ebd85` |
| `src/device/vpic.c` | `src/nxvm-baseline/device/vpic.c` | `89a1f10a0a4fcc8d1ae1aa634d515921c0e68b0349998c4c36d57005ff66fcc2` |
| `src/device/vpic.h` | `src/nxvm-baseline/device/vpic.h` | `aecaefc42df7fdb475e358089042a48f53763b9f366f4f5a604364071eda23de` |
| `src/device/vpit.c` | `src/nxvm-baseline/device/vpit.c` | `0d076ea2740cf70ea699bd534294a1c295abe5b2bb09f1a384b35823654614e3` |
| `src/device/vpit.h` | `src/nxvm-baseline/device/vpit.h` | `a69679041d16a11eb9ac043227bd9a744c3ff4cb80cc22c5a078e4c605f439e4` |
| `src/device/vport.c` | `src/nxvm-baseline/device/vport.c` | `748ed589394b0626d2219d3298e4fb21966e04db48132334c4090403263ea184` |
| `src/device/vport.h` | `src/nxvm-baseline/device/vport.h` | `fea86e8826834b209a626795b7a6ff6cb3db0594c7a6f6f81a3307eef96d82fd` |
| `src/device/vram.c` | `src/nxvm-baseline/device/vram.c` | `a3a850d257db155d0bb5e0356dc94ba21d88fd667c9e8ef457c6586aa4715229` |
| `src/device/vram.h` | `src/nxvm-baseline/device/vram.h` | `b46f4eeacc0e8367839c122854da37f208245cd48505eb2b30c34e66abeb2274` |
| `src/device/vvadp.c` | `src/nxvm-baseline/device/vvadp.c` | `712e38e8051192e4b897ce60b101743e5709ecbdc821b78c3701e2a9be1edfc8` |
| `src/device/vvadp.h` | `src/nxvm-baseline/device/vvadp.h` | `655c55701c6805919636dacd5f88e3025d03f45c3a7ccd160b11f31bf41871fb` |
| `src/global.h` | `src/nxvm-baseline/global.h` | `af05213397fbe4692e998b6cc361d33f0f1379c843aa21a426fb245429ec22a9` |
| `src/machine.c` | `src/nxvm-baseline/machine.c` | `753c08a6db8c75e5143acccd7a8707e637536c8b90069003f48054325f5d126d` |
| `src/machine.h` | `src/nxvm-baseline/machine.h` | `6883521b2fc46360bfef7ed6742fc0306dbabfb9ef6ff16287b1b813423afa4e` |
| `src/main.c` | `src/nxvm-baseline/main.c` | `ec874f6c173e3d8198e321f0eb147ca596a3c391fcf8aa91e914f163be7c93b0` |
| `src/platform/linux/linux.c` | `src/nxvm-baseline/platform/linux/linux.c` | `5fd83deffa119d40b4971574fe4b97bb770fed909d99b0d05e83a8b165f1f578` |
| `src/platform/linux/linux.h` | `src/nxvm-baseline/platform/linux/linux.h` | `26c95b9cb0e403194a46bfea508bd8ae3c27c125948a656ae31009f30f01de51` |
| `src/platform/linux/linuxcon.c` | `src/nxvm-baseline/platform/linux/linuxcon.c` | `f57f4c8b6205bea8e300d0f037b5c79985f716357a1d04adcb57a470e616058e` |
| `src/platform/linux/linuxcon.h` | `src/nxvm-baseline/platform/linux/linuxcon.h` | `755db004bf280c431ed2a5e6d72dea266e3a20f0b60605ea3f3d0f009f8ab2e2` |
| `src/platform/platform.c` | `src/nxvm-baseline/platform/platform.c` | `407c722066bfd6e3a5e80f35105f41063cf56603920e069e63c3f54bb9ebd46e` |
| `src/platform/platform.h` | `src/nxvm-baseline/platform/platform.h` | `d0b80296e610ff7688cdfb37d70add01a8fb9089b37e70b8871941b13b0d579f` |
| `src/platform/win32/w32adisp.c` | `src/nxvm-baseline/platform/win32/w32adisp.c` | `39749d43d2bf52d360ef5695604900598711e65367e8abbd7bc6c97d210d92b5` |
| `src/platform/win32/w32adisp.h` | `src/nxvm-baseline/platform/win32/w32adisp.h` | `48377d86065091f70189def0b7d65e09abae575f055d432e479ddad89e14362f` |
| `src/platform/win32/w32cdisp.c` | `src/nxvm-baseline/platform/win32/w32cdisp.c` | `81178bd1436c28e603ec5012a962715f3d472df046d27cf59484c1a5c7b9b1fe` |
| `src/platform/win32/w32cdisp.h` | `src/nxvm-baseline/platform/win32/w32cdisp.h` | `d17e696b715d5f2d83eb75f07d57e9221e7d6ec98ed0f9687b996bfb26aabae0` |
| `src/platform/win32/win32.c` | `src/nxvm-baseline/platform/win32/win32.c` | `0af11bbb52bc1a62de216f0874e5abd9b391df600fdba6e6e8c6855ee2a12590` |
| `src/platform/win32/win32.h` | `src/nxvm-baseline/platform/win32/win32.h` | `b6ab8d4d26dd7ca48a122e03b7e0a87b296d62b94f95a7070210a313f9092b69` |
| `src/platform/win32/win32app.c` | `src/nxvm-baseline/platform/win32/win32app.c` | `c31a5e680a54e666d8fe9b49749d48a793ef9820be7ac6724586413957d52cbf` |
| `src/platform/win32/win32app.h` | `src/nxvm-baseline/platform/win32/win32app.h` | `45328fd5a0f2787619e4c692eba0c888059628e019ad99b6defd550443d8b0e5` |
| `src/platform/win32/win32con.c` | `src/nxvm-baseline/platform/win32/win32con.c` | `2e9790578b419df2ce7a9670fe3396f1704f475921871a253e5c543d17b2f6a6` |
| `src/platform/win32/win32con.h` | `src/nxvm-baseline/platform/win32/win32con.h` | `966d6a0aa31e5963c1fd51a35f9568c3a9a79cc64a8909abb2ba49834233357f` |
| `src/utils.c` | `src/nxvm-baseline/utils.c` | `a571c424c5b9f8ac7b9e486ebb465cb884a3308ebddc1200b31de9e444c461d8` |
| `src/utils.h` | `src/nxvm-baseline/utils.h` | `620b5f0255ec17ff66ee11378fe586b3628da5ef49db3dd8084295c0c671cfd8` |
| `src/xasm32/aasm32.c` | `src/nxvm-baseline/xasm32/aasm32.c` | `6d1460dd1f0da9ba2d4000a392b953625ba8177a44ab8494406483d55abe2c51` |
| `src/xasm32/aasm32.h` | `src/nxvm-baseline/xasm32/aasm32.h` | `d97ec9453e668a611d10c635a868b93de4101066aa2538309f842259e7b34458` |
| `src/xasm32/dasm32.c` | `src/nxvm-baseline/xasm32/dasm32.c` | `296022257569460083019431861b64b107827f843c18d1695b4e94d995f15755` |
| `src/xasm32/dasm32.h` | `src/nxvm-baseline/xasm32/dasm32.h` | `9ecada211b912faf8a9f93413569d6624be99cea5069d888ee96ff287459c957` |
