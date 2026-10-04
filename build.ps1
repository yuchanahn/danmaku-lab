$ErrorActionPreference = 'Stop'

Push-Location $PSScriptRoot
try {
    cmake --preset msvc-debug
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    cmake --build --preset build-debug --parallel
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    Start-Process -FilePath (Join-Path $PSScriptRoot 'out/build/msvc-debug/Debug/DanmakuShooter.exe')
}
finally {
    Pop-Location
}
