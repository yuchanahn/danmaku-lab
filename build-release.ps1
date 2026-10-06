$ErrorActionPreference = 'Stop'

Push-Location $PSScriptRoot
try {
    cmake --preset msvc-release
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    cmake --build --preset build-release --parallel
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    Start-Process -FilePath (Join-Path $PSScriptRoot 'out/build/msvc-release/Release/DanmakuShooter.exe') -WorkingDirectory $PSScriptRoot
}
finally {
    Pop-Location
}
