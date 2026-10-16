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
$machine = Join-Path $root 'src/x86-core/machine/machine.c'
$scheduler = Join-Path $root 'src/x86-core/machine/machine_scheduler.c'
$board = Join-Path $root 'src/core/board-base/board_advance.c'
$profile = Join-Path $root 'src/app-my5170/machine_config.c'

Require-Pattern $profile '596591u, 4000000u' 'Model-339 PIT rational clock'
Require-Pattern $profile '64u, 15625u' 'Model-339 RTC rational clock'
Require-Pattern $profile '32768u' 'Model-339 RTC second divisor'
Require-Pattern $scheduler 'x86_core_machine_arbitration_advance' 'arbitration callback'
Require-Pattern $scheduler 'machine->attachment\.before_internal_advance\(' 'Board readiness callback'
Require-Pattern $scheduler 'machine->attachment\.after_internal_advance\(' 'Board peripheral callback'
Require-Pattern $board 'x86_core_machine_clock_domain_advance\(&board->dma_clock, source_ticks\)' 'DMA callback cadence'
Require-Pattern $board 'x86_core_machine_clock_domain_advance\(&board->pit_clock,' 'PIT callback cadence'
Require-Pattern $board 'x86_core_machine_clock_domain_advance\(&board->rtc_clock, source_ticks\)' 'RTC callback cadence'
Require-Pattern $board 'x86_core_machine_clock_domain_advance\(&board->kbc_clock, source_ticks\)' 'KBC callback cadence'
Require-Pattern $board 'x86_core_machine_clock_domain_advance\(&board->vadp_clock, source_ticks\)' 'VADP callback cadence'
Require-Pattern $board 'core_machine_fdc_advance_at\(board->fdc, due_tick\)' 'FDC readiness call'
Require-Pattern $board 'core_machine_hdc_advance_at\(board->hdc, due_tick\)' 'HDC readiness call'
Require-Pattern $scheduler 'x86_core_machine_clock_domain_advance\(&machine->provider_clock,' 'provider clock route'
Require-Pattern $board 'x86_core_machine_clock_domain_reset\(&board->dma_clock\)' 'DMA reset phase'
Require-Pattern $machine 'x86_core_machine_clock_domain_reset\(&machine->provider_clock\)' 'provider reset phase'

$advanceUsers = @(
    Get-ChildItem -Path (Join-Path $root 'src') -Recurse -Filter '*.c' |
        Select-String -Pattern 'x86_core_machine_advance_time\('
)
foreach ($match in $advanceUsers) {
    $relative = $match.Path.Substring($root.Length).TrimStart('\', '/').Replace('\', '/')
    if ($relative -notin @(
        'src/x86-core/machine/machine.c'
    )) {
        throw "Unexpected machine-time publisher: $relative"
    }
}

Write-Output 'M5:T375:S7:MODEL339-CALLBACK-CLOCK-LEDGER:OK'
