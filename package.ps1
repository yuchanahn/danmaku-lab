$ErrorActionPreference = 'Stop'

Push-Location $PSScriptRoot
try {
    cmake --preset msvc-release
    if ($LASTEXITCODE -ne 0) { throw 'Release configure failed.' }
    cmake --build --preset build-release --parallel
    if ($LASTEXITCODE -ne 0) { throw 'Release build failed.' }

    $packageStamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
    $packageOutput = Join-Path $PSScriptRoot 'out/packages'
    $packageName = "DanmakuLab-$packageStamp"
    $packageFolder = Join-Path $packageOutput $packageName
    New-Item -ItemType Directory -Path $packageFolder -Force | Out-Null

    $releaseFolder = Join-Path $PSScriptRoot 'out/build/msvc-release/Release'
    Copy-Item -LiteralPath (Join-Path $releaseFolder 'DanmakuShooter.exe') -Destination $packageFolder
    Copy-Item -LiteralPath (Join-Path $releaseFolder 'assets') -Destination $packageFolder -Recurse
    Copy-Item -LiteralPath (Join-Path $releaseFolder 'shaders') -Destination $packageFolder -Recurse

    @"
Danmaku Lab — Windows x64

실행: DanmakuShooter.exe
실행 파일과 assets, shaders 폴더를 함께 보관하세요.

요구 환경: Windows 10/11 x64, DirectX 11 지원 그래픽 환경,
Microsoft Visual C++ v14 x64 Redistributable.
런타임 안내: https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist

Enter: 시작 / 결과 화면에서 타이틀로 복귀
방향키: 이동   Z: 발사   P: 일시정지
F1: 디버그 패널   F2: 밝은/어두운 배경
4/5/6: 사각형/원형/Glow 탄환   7/8: Alpha/Additive
최소 게임 화면: 960 x 540
보스 HP 절반 이하에서 원형 패턴, 클리어/실패 후 다시 시작 가능.
최소화 중 전투/렌더링 정지. BGM은 일시정지/최소화 중에도 재생.

개발 검증 모드: --smoke-test (숨김 창으로 검사 후 종료)
이 패키지는 현재 개발 PC에서 리소스 로딩과 전투 규칙을 검사했습니다.
다른 PC의 드라이버/런타임 호환성을 모두 검증한 것은 아닙니다.
"@ | Set-Content -LiteralPath (Join-Path $packageFolder 'README.txt') -Encoding utf8

    $smokeOutput = Join-Path $packageOutput "$packageName-smoke.stdout.log"
    $smokeError = Join-Path $packageOutput "$packageName-smoke.stderr.log"
    $smokeProcess = Start-Process -FilePath (Join-Path $packageFolder 'DanmakuShooter.exe') `
        -ArgumentList '--smoke-test' -WorkingDirectory ([System.IO.Path]::GetTempPath()) `
        -WindowStyle Hidden -PassThru -Wait `
        -RedirectStandardOutput $smokeOutput -RedirectStandardError $smokeError
    if ($smokeProcess.ExitCode -ne 0) {
        if (Test-Path -LiteralPath $smokeError) { Get-Content -LiteralPath $smokeError }
        throw "Packaged smoke test failed with exit code $($smokeProcess.ExitCode)."
    }
    Get-Content -LiteralPath $smokeOutput

    $packageArchive = Join-Path $packageOutput "$packageName.zip"
    Compress-Archive -LiteralPath $packageFolder -DestinationPath $packageArchive
    Write-Output "Package: $packageFolder"
    Write-Output "Archive: $packageArchive"
}
finally {
    Pop-Location
}
