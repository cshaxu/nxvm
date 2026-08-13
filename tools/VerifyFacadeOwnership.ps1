param(
    [Parameter(Mandatory = $true)]
    [string]$RepositoryRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$root = (Resolve-Path -LiteralPath $RepositoryRoot).Path
$sourceRoot = Join-Path $root 'src'
$sourceFiles = Get-ChildItem -LiteralPath $sourceRoot -Recurse -File -Filter '*.c'

$rules = @(
    [pscustomobject]@{ Name = 'core keyboard legacy facade'; Pattern = '\bcore_machine_keyboard_(bind|apply_host_state|receive_key_press|get_modifier)\s*\('; AllowedFiles = @() },
    [pscustomobject]@{ Name = 'core display legacy facade'; Pattern = '\bcore_machine_display_(bind|notify_mode_changed|bind_snapshot_provider|capture_snapshot)\s*\('; AllowedFiles = @() },
    [pscustomobject]@{ Name = 'persistent core debug target facade'; Pattern = '\bcore_product_debug_(bind_target|get_target)\s*\('; AllowedFiles = @() },
    [pscustomobject]@{ Name = 'core wait facade'; Pattern = '\bcore_product_wait_bind\s*\('; AllowedFiles = @() },
    [pscustomobject]@{ Name = 'VM keyboard sink facade'; Pattern = '\bvm_platform_keyboard_bind\s*\('; AllowedFiles = @() },
    [pscustomobject]@{ Name = 'VM execution sink facade'; Pattern = '\bvm_platform_execution_bind\s*\('; AllowedFiles = @() },
    [pscustomobject]@{ Name = 'VM platform mode facade'; Pattern = '\bplatform\.flagMode\b'; AllowedFiles = @() },
    [pscustomobject]@{ Name = 'Win32 global host handles'; Pattern = '\b(hOut|w32aHWnd)\b'; AllowedFiles = @() },
    [pscustomobject]@{ Name = 'display frame mailbox facade'; Pattern = '\bcore_platform_display_(initialize|publish|capture)\s*\('; AllowedFiles = @() },
    [pscustomobject]@{ Name = 'Win32 keyboard sink facade'; Pattern = '\bwin32KeyboardBindStateSink\s*\('; AllowedFiles = @() },
    [pscustomobject]@{ Name = 'block legacy facade'; Pattern = '\bcore_machine_block_(bind_provider|get_geometry|read|write)\s*\('; AllowedFiles = @() }
)

$failures = @()
foreach ($rule in $rules) {
    $matches = @()
    foreach ($file in $sourceFiles) {
        $relative = $file.FullName.Substring($root.Length + 1).Replace('\', '/')
        $content = Get-Content -LiteralPath $file.FullName -Raw
        if ([regex]::IsMatch($content, $rule.Pattern)) {
            $matches += $relative
            if ($rule.AllowedFiles -notcontains $relative) {
                $failures += "$($rule.Name): unexpected call site $relative"
            }
        }
    }
    Write-Output ("M5:T75:FACADE-OWNERSHIP:{0}: {1}" -f $rule.Name, ($matches -join ', '))
}

if ($failures.Count -ne 0) {
    $failures | ForEach-Object { Write-Error $_ }
    exit 1
}

Write-Output 'M5:T75:FACADE-OWNERSHIP:OK'
