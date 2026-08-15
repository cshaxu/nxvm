param(
    [Parameter(Mandatory = $true)]
    [string]$RepositoryRoot
)

$ErrorActionPreference = 'Stop'

function Require-Pattern {
    param([string]$Path, [string]$Pattern, [string]$Description)
    if (-not (Select-String -Path $Path -Pattern $Pattern -Quiet)) {
        throw "Missing $Description in $Path"
    }
}

$root = (Resolve-Path $RepositoryRoot).Path
$machine = Join-Path $root 'src/core/machine/machine.c'
$profile = Join-Path $root 'src/vm/profile/default_profile/pc_at_profile.c'
$virtualTime = Join-Path $root 'src/vm/composition/session/virtual_time.c'

Require-Pattern $profile '596591u, 4000000u' 'Model-339 PIT rational clock'
Require-Pattern $profile '64u, 15625u' 'Model-339 RTC rational clock'
Require-Pattern $profile '32768u' 'Model-339 RTC second divisor'
Require-Pattern $machine 'core_machine_arbitration_tick' 'arbitration callback'
Require-Pattern $machine 'core_machine_readiness_tick' 'readiness callback'
Require-Pattern $machine 'core_machine_peripheral_tick' 'peripheral callback'
Require-Pattern $machine 'core_machine_clock_domain_advance\(&machine->dma_clock, 1u\)' 'DMA callback cadence'
Require-Pattern $machine 'core_machine_clock_domain_advance\(&machine->pit_clock, 1u\)' 'PIT callback cadence'
Require-Pattern $machine 'core_machine_clock_domain_advance\(&machine->rtc_clock, 1u\)' 'RTC callback cadence'
Require-Pattern $machine 'core_machine_clock_domain_advance\(&machine->kbc_clock, 1u\)' 'KBC callback cadence'
Require-Pattern $machine 'core_machine_clock_domain_advance\(&machine->vadp_clock, 1u\)' 'VADP callback cadence'
Require-Pattern $machine 'core_machine_fdc_advance\(&machine->fdc\)' 'FDC readiness call'
Require-Pattern $machine 'core_machine_hdc_advance\(&machine->hdc\)' 'HDC readiness call'
Require-Pattern $machine 'core_machine_clock_domain_advance\(&machine->provider_clock,' 'provider clock route'
Require-Pattern $machine 'core_machine_clock_domain_reset\(&machine->dma_clock\)' 'DMA reset phase'
Require-Pattern $machine 'core_machine_clock_domain_reset\(&machine->provider_clock\)' 'provider reset phase'
Require-Pattern $machine 'core_machine_timeline_schedule\(&machine->timeline, 1u,' 'tick-one callback reset schedule'
Require-Pattern $virtualTime 'core_machine_advance_time\(session->core_machine, source_ticks\)' 'composition-owned non-retirement publisher'

$advanceUsers = @(
    Get-ChildItem -Path (Join-Path $root 'src') -Recurse -Filter '*.c' |
        Select-String -Pattern 'core_machine_advance_time\('
)
foreach ($match in $advanceUsers) {
    $relative = $match.Path.Substring($root.Length).TrimStart('\', '/').Replace('\', '/')
    if ($relative -notin @(
        'src/core/machine/machine.c',
        'src/vm/composition/session/virtual_time.c'
    )) {
        throw "Unexpected machine-time publisher: $relative"
    }
}

Write-Output 'M5:T375:S7:MODEL339-CALLBACK-CLOCK-LEDGER:OK'
