param(
    [ValidatePattern('^v?\d+\.\d+\.\d+(?:-[A-Za-z0-9.-]+)?$')]
    [string]$Version
)

$ErrorActionPreference = 'Stop'

Push-Location $PSScriptRoot
try {
    cmake --preset msvc-release
    if ($LASTEXITCODE -ne 0) { throw 'Release configure failed.' }
    cmake --build --preset build-release --parallel
    if ($LASTEXITCODE -ne 0) { throw 'Release build failed.' }

    $packageStamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
    $packageOutput = Join-Path $PSScriptRoot 'out/packages'
    $packageName = if ($Version) { "DanmakuLab-$Version-windows-x64" } else { "DanmakuLab-$packageStamp-windows-x64" }
    $packageFolder = Join-Path $packageOutput $packageName
    if (Test-Path -LiteralPath $packageFolder) { throw "Package already exists: $packageFolder" }
    New-Item -ItemType Directory -Path $packageFolder | Out-Null

    $releaseFolder = Join-Path $PSScriptRoot 'out/build/msvc-release/Release'
    Copy-Item -LiteralPath (Join-Path $releaseFolder 'DanmakuShooter.exe') -Destination $packageFolder
    # Copy current source assets explicitly; build folders can retain deleted files.
    $packageAssets = Join-Path $packageFolder 'assets'
    $packageShaders = Join-Path $packageFolder 'shaders'
    New-Item -ItemType Directory -Path $packageAssets, $packageShaders | Out-Null
    foreach ($assetName in @('player.png', 'Enemy1.png', 'Enemy2.png', 'boss1.png', 'boss2.png',
                             'bg0.png', 'bg1.png', 'bgm_fairy_battles.wav', 'shot_test.wav', 'BGM_LICENSE.md')) {
        Copy-Item -LiteralPath (Join-Path $PSScriptRoot "assets/$assetName") -Destination $packageAssets
    }
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'shaders/Sprite.hlsl') -Destination $packageShaders

    @"
Danmaku Lab — Windows x64

실행: DanmakuShooter.exe
실행 파일과 assets, shaders 폴더를 함께 보관하세요.

요구 환경: Windows 10/11 x64, DirectX 11 지원 그래픽 환경.
Visual C++ 런타임은 실행 파일에 포함되어 별도 설치가 필요하지 않습니다.

Enter: 시작 / 결과 화면에서 타이틀로 복귀
방향키: 이동   Z: 발사   P: 일시정지
F1: 디버그 패널   F2: 밝은/어두운 배경
F4: 모든 적에게 50 데미지   F5: 충돌 방식 전환   F6: 무적 치트
4/5/6: 사각형/원형/Glow 탄환   7/8: Alpha/Additive
최소 게임 화면: 960 x 540
일반 적 10마리 → 중간보스 → 최종보스의 3스테이지 구성.
스테이지 클리어마다 강화 아이템 2개: 청록 공속 / 주황 공격력 / 보라 연속탄.
이동해서 직접 먹으세요. 화면 밖으로 놓친 아이템은 소멸합니다.
클리어/실패 후 다시 시작 가능하며 강화는 새 판에서 초기화됩니다.
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
    $packageHash = (Get-FileHash -LiteralPath $packageArchive -Algorithm SHA256).Hash.ToLowerInvariant()
    "$packageHash  $packageName.zip" | Set-Content -LiteralPath "$packageArchive.sha256" -Encoding ascii
    Write-Output "Package: $packageFolder"
    Write-Output "Archive: $packageArchive"
}
finally {
    Pop-Location
}
