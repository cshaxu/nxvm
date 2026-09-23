param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('x64', 'x86')]
    [string]$Architecture
)

$ErrorActionPreference = 'Stop'
$repository = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$compilerVariable = 'MYNES_CC_' + $Architecture.ToUpperInvariant()
$compiler = [Environment]::GetEnvironmentVariable($compilerVariable)
if (-not $compiler -or -not (Test-Path -LiteralPath $compiler)) {
    throw "$compilerVariable must identify the compiler executable."
}
$originalPath = $env:PATH
$cmake = (Get-Command cmake -ErrorAction Stop).Source
$ctest = (Get-Command ctest -ErrorAction Stop).Source
$ninja = (Get-Command ninja -ErrorAction Stop).Source
try {
    $env:PATH = (Split-Path -Parent $compiler) + ';' + $originalPath
    foreach ($entry in @('lib', 'common', 'test/lib', 'test/common')) {
        $name = $entry.Replace('/', '-')
        $build = Join-Path $repository "build/standalone-$Architecture-$name"
        New-Item -ItemType Directory -Force -Path $build | Out-Null
        $log = Join-Path $build 'verification.log'
        $source = if ($entry.StartsWith('test/')) {
            Join-Path $repository $entry
        }
        else {
            Join-Path $repository "src/$entry"
        }
        $options = @('-S', $source, '-B', $build,
            '-G', 'Ninja', "-DCMAKE_MAKE_PROGRAM=$ninja", "-DCMAKE_C_COMPILER=$compiler",
            '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
            '-DCMAKE_EXE_LINKER_FLAGS=-static -static-libgcc',
            '-DLIBRARY_STRICT_WARNINGS=ON')
        if ($entry -eq 'common') {
            $options += "-DCOMMON_LIB_ROOT=$repository/src/lib"
        }
        if ($entry.StartsWith('test/')) {
            $options += "-DLIB_ROOT=$repository/src/lib"
        }
        if ($entry -eq 'test/common') {
            $options += "-DCOMMON_ROOT=$repository/src/common"
        }
        & $cmake @options *> $log
        if ($LASTEXITCODE -ne 0) { Get-Content $log -Tail 50; throw "$entry configure failed" }
        $compilerInfo = Get-ChildItem -LiteralPath (Join-Path $build 'CMakeFiles') -Recurse -Filter CMakeCCompiler.cmake |
            Select-Object -First 1
        $expectedWidth = if ($Architecture -eq 'x64') { '8' } else { '4' }
        if (-not $compilerInfo -or (Get-Content -Raw $compilerInfo.FullName) -notmatch
                ('set\(CMAKE_C_SIZEOF_DATA_PTR "' + $expectedWidth + '"\)')) {
            throw "$entry compiler architecture mismatch"
        }
        & $cmake --build $build --parallel 8 *>> $log
        if ($LASTEXITCODE -ne 0) { Get-Content $log -Tail 50; throw "$entry build failed" }
        $commands = Get-Content -Raw (Join-Path $build 'compile_commands.json') | ConvertFrom-Json
        foreach ($command in $commands) {
            if ($command.command -notmatch '-std=c11(?: |$)' -or
                    $command.command -notmatch '-Werror(?: |$)') {
                throw "$entry compile command does not enforce C11/strict warnings"
            }
        }
        if ($entry.StartsWith('test/')) {
            & $ctest --test-dir $build --output-on-failure --timeout 30 --no-tests=error -j 1 `
                -E 'types-layout-selftest' --output-junit (Join-Path $build 'results.xml') *>> $log
            if ($LASTEXITCODE -ne 0) { Get-Content $log -Tail 50; throw "$entry tests failed" }
            if ($entry -eq 'test/lib') {
                & $ctest --test-dir $build --output-on-failure --timeout 180 --no-tests=error -j 1 `
                    -R 'types-layout-selftest' *>> $log
                if ($LASTEXITCODE -ne 0) { Get-Content $log -Tail 50; throw "$entry types-layout selftest failed" }
            }
            Get-Content $log | Select-String '100% tests passed'
        }
        Write-Output "$Architecture $entry independent verification passed"
    }
}
finally {
    $env:PATH = $originalPath
}
