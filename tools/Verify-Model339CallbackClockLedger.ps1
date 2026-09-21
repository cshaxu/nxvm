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
$machine = Join-Path $root 'src/core/devices/machine.c'
$scheduler = Join-Path $root 'src/core/devices/machine_scheduler.c'
$profile = Join-Path $root 'src/core/profiles/default_profile/pc_at_profile.c'

Require-Pattern $profile '596591u, 4000000u' 'Model-339 PIT rational clock'
Require-Pattern $profile '64u, 15625u' 'Model-339 RTC rational clock'
Require-Pattern $profile '32768u' 'Model-339 RTC second divisor'
Require-Pattern $scheduler 'core_machine_arbitration_advance' 'arbitration callback'
Require-Pattern $scheduler 'core_machine_readiness_advance' 'readiness callback'
Require-Pattern $scheduler 'core_machine_peripheral_advance' 'peripheral callback'
Require-Pattern $scheduler 'core_machine_clock_domain_advance\(&machine->dma_clock, source_ticks\)' 'DMA callback cadence'
Require-Pattern $scheduler 'core_machine_clock_domain_advance\(&machine->pit_clock, source_ticks\)' 'PIT callback cadence'
Require-Pattern $scheduler 'core_machine_clock_domain_advance\(&machine->rtc_clock, source_ticks\)' 'RTC callback cadence'
Require-Pattern $scheduler 'core_machine_clock_domain_advance\(&machine->kbc_clock, source_ticks\)' 'KBC callback cadence'
Require-Pattern $scheduler 'core_machine_clock_domain_advance\(&machine->vadp_clock, source_ticks\)' 'VADP callback cadence'
Require-Pattern $scheduler 'core_machine_fdc_advance_at\(&machine->fdc, due_tick\)' 'FDC readiness call'
Require-Pattern $scheduler 'core_machine_hdc_advance_elapsed\(&machine->hdc, source_ticks\)' 'HDC readiness call'
Require-Pattern $scheduler 'core_machine_clock_domain_advance\(&machine->provider_clock,' 'provider clock route'
Require-Pattern $machine 'core_machine_clock_domain_reset\(&machine->dma_clock\)' 'DMA reset phase'
Require-Pattern $machine 'core_machine_clock_domain_reset\(&machine->provider_clock\)' 'provider reset phase'

$advanceUsers = @(
    Get-ChildItem -Path (Join-Path $root 'src') -Recurse -Filter '*.c' |
        Select-String -Pattern 'core_machine_advance_time\('
)
foreach ($match in $advanceUsers) {
    $relative = $match.Path.Substring($root.Length).TrimStart('\', '/').Replace('\', '/')
    if ($relative -notin @(
        'src/core/devices/machine.c'
    )) {
        throw "Unexpected machine-time publisher: $relative"
    }
}

Write-Output 'M5:T375:S7:MODEL339-CALLBACK-CLOCK-LEDGER:OK'
