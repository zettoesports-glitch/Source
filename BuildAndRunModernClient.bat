@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem Deterministic modernization test workflow:
rem 1) verify critical renderer sources match Git HEAD
rem 2) rebuild the exact Global Release x86 solution output from scratch
rem 3) deploy that exact Main.exe to Cliente\Main.exe
rem 4) verify both files are byte-identical with SHA256
rem 5) clear/stamp ModernBMD validation logs
rem 6) run the deployed client with Cliente as working directory

set "ROOT=%~dp0"
set "SOLUTION=%ROOT%Main\Main.sln"
set "BUILD_EXE=%ROOT%Main\Global Release\Main.exe"
set "CLIENT_DIR=%ROOT%Cliente"
set "CLIENT_EXE=%CLIENT_DIR%\Main.exe"
set "MSBUILD="
set "GIT_HEAD=<unknown>"

for /f "usebackq tokens=*" %%H in (`git -C "%ROOT%" rev-parse HEAD 2^>nul`) do set "GIT_HEAD=%%H"

git -C "%ROOT%" diff --quiet -- ^
  "Main/source/New_ModelBMD.h" ^
  "Main/source/New_ModelBMD.cpp" ^
  "Main/source/New_RenderBMD.cpp" ^
  "Main/source/Render/Model/BMDModernObjectGuard.cpp" ^
  "Main/source/Render/Model/BMDModernRuntime.cpp"
if errorlevel 1 (
    echo [ERROR] Critical ModernBMD renderer sources have local modifications.
    echo         Refusing to build a binary that does not match Git HEAD.
    echo         Modified renderer files:
    git -C "%ROOT%" diff --name-only -- ^
      "Main/source/New_ModelBMD.h" ^
      "Main/source/New_ModelBMD.cpp" ^
      "Main/source/New_RenderBMD.cpp" ^
      "Main/source/Render/Model/BMDModernObjectGuard.cpp" ^
      "Main/source/Render/Model/BMDModernRuntime.cpp"
    exit /b 8
)

if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    for /f "usebackq tokens=*" %%I in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
        if not defined MSBUILD set "MSBUILD=%%I"
    )
)

if not defined MSBUILD if exist "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild.exe" set "MSBUILD=C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild.exe"
if not defined MSBUILD if exist "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
if not defined MSBUILD if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"

if not defined MSBUILD (
    echo [ERROR] MSBuild.exe nao encontrado.
    exit /b 2
)

echo [1/6] Git HEAD: %GIT_HEAD%
echo [2/6] MSBuild: "%MSBUILD%"
echo [2/6] Rebuilding from scratch: "%SOLUTION%"
"%MSBUILD%" "%SOLUTION%" /t:Rebuild /p:Configuration="Global Release" /p:Platform=x86 /m /v:m
if errorlevel 1 (
    echo [ERROR] Rebuild falhou. Cliente nao foi alterado.
    exit /b 3
)

if not exist "%BUILD_EXE%" (
    echo [ERROR] Rebuild terminou, mas o output esperado nao existe:
    echo         "%BUILD_EXE%"
    exit /b 4
)

echo [3/6] Deploying exact rebuild output...
copy /Y "%BUILD_EXE%" "%CLIENT_EXE%" >nul
if errorlevel 1 (
    echo [ERROR] Falha ao copiar Main.exe para Cliente.
    exit /b 5
)

echo [4/6] Verifying SHA256 source/deployed...
set "BUILD_HASH="
set "CLIENT_HASH="
for /f "tokens=1" %%H in ('certutil -hashfile "%BUILD_EXE%" SHA256 ^| findstr /R /V "hash CertUtil"') do if not defined BUILD_HASH set "BUILD_HASH=%%H"
for /f "tokens=1" %%H in ('certutil -hashfile "%CLIENT_EXE%" SHA256 ^| findstr /R /V "hash CertUtil"') do if not defined CLIENT_HASH set "CLIENT_HASH=%%H"

if not defined BUILD_HASH (
    echo [ERROR] Nao foi possivel calcular SHA256 do output.
    exit /b 6
)
if /I not "%BUILD_HASH%"=="%CLIENT_HASH%" (
    echo [ERROR] SHA256 diferente depois do deploy.
    echo Build : %BUILD_HASH%
    echo Cliente: %CLIENT_HASH%
    exit /b 7
)

echo       SHA256: %BUILD_HASH%

echo [5/6] Clearing and stamping ModernBMD validation logs...
del /Q "%CLIENT_DIR%\ModernBMD_boot.log" 2>nul
del /Q "%CLIENT_DIR%\ModernBMD_render.log" 2>nul
del /Q "%CLIENT_DIR%\Data\ModernBMD.log" 2>nul
> "%CLIENT_DIR%\Data\ModernBMD.log" echo [ModernBMD] build-head=%GIT_HEAD%; build-mode=Rebuild; sha256=%BUILD_HASH%

echo [6/6] Running deployed client from Cliente working directory...
pushd "%CLIENT_DIR%"
start "" "%CLIENT_EXE%"
popd

echo.
echo Deployed and launched the exact Global Release rebuild.
echo Git HEAD : %GIT_HEAD%
echo SHA256   : %BUILD_HASH%
echo Expected diagnostics after startup:
echo   %CLIENT_DIR%\ModernBMD_boot.log
echo   %CLIENT_DIR%\ModernBMD_render.log
echo   %CLIENT_DIR%\Data\ModernBMD.log

endlocal
exit /b 0