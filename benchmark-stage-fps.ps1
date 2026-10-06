param([switch]$Compare, [switch]$Grid, [switch]$Pool)
$ErrorActionPreference = 'Stop'
if (([int]$Compare.IsPresent + [int]$Grid.IsPresent + [int]$Pool.IsPresent) -gt 1) { throw 'Choose one of -Compare, -Grid, -Pool.' }
Push-Location $PSScriptRoot
try {
    cmake --preset msvc-release
    if ($LASTEXITCODE -ne 0) { throw 'Release configuration failed.' }
    cmake --build --preset build-release --parallel
    if ($LASTEXITCODE -ne 0) { throw 'Release build failed.' }
    $benchmarkArgument = if ($Pool) { '--benchmark-pool' } elseif ($Grid) { '--benchmark-grid' } elseif ($Compare) { '--benchmark-stage-fps-compare' } else { '--benchmark-stage-fps' }
    $benchmarkPattern = if ($Pool) { 'pool-compare-*.csv' } elseif ($Grid) { 'grid-compare-*.csv' } elseif ($Compare) { 'stage-compare-*.csv' } else { 'stage-fps-*.csv' }
    $benchmarkProcess = Start-Process -FilePath (Join-Path $PSScriptRoot 'out/build/msvc-release/Release/DanmakuShooter.exe') -ArgumentList $benchmarkArgument -WindowStyle Hidden -Wait -PassThru
    if ($benchmarkProcess.ExitCode -ne 0) { throw "Benchmark failed: $($benchmarkProcess.ExitCode)" }
    Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'out/benchmarks') -Filter $benchmarkPattern |
        Sort-Object LastWriteTime -Descending | Select-Object -First 1 |
        ForEach-Object { Import-Csv -LiteralPath $_.FullName | Format-Table stage,repeat,mode,fps,bullets_mean,collision_ms_per_tick,candidates_per_tick,pool_timing,spawn_ms,release_ms }
} finally {
    Pop-Location
}
