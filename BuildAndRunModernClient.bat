@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem Deterministic modernization test workflow:
rem 1) build the exact Global Release x86 solution output
rem 2) deploy that exact Main.exe to Cliente\Main.exe
rem 3) verify both files are byte-identical with SHA256
rem 4) clear ModernBMD validation logs
rem 5) run the deployed client with Cliente as working directory

set "ROOT=%~dp0"
set "SOLUTION=%ROOT%Main\Main.sln"
set "BUILD_EXE=%ROOT%Main\Global Release\Main.exe"
set "CLIENT_DIR=%ROOT%Cliente"
set "CLIENT_EXE=%CLIENT_DIR%\Main.exe"
set "MSBUILD="

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

echo [1/5] MSBuild: "%MSBUILD%"
echo [1/5] Building: "%SOLUTION%"
"%MSBUILD%" "%SOLUTION%" /p:Configuration="Global Release" /p:Platform=x86 /m /v:m
if errorlevel 1 (
    echo [ERROR] Build falhou. Cliente nao foi alterado.
    exit /b 3
)

if not exist "%BUILD_EXE%" (
    echo [ERROR] Build terminou, mas o output esperado nao existe:
    echo         "%BUILD_EXE%"
    exit /b 4
)

echo [2/5] Deploying exact build output...
copy /Y "%BUILD_EXE%" "%CLIENT_EXE%" >nul
if errorlevel 1 (
    echo [ERROR] Falha ao copiar Main.exe para Cliente.
    exit /b 5
)

echo [3/5] Verifying SHA256 source/deployed...
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

echo [4/5] Clearing ModernBMD validation logs...
del /Q "%CLIENT_DIR%\ModernBMD_boot.log" 2>nul
del /Q "%CLIENT_DIR%\ModernBMD_render.log" 2>nul
del /Q "%CLIENT_DIR%\Data\ModernBMD.log" 2>nul

echo [5/5] Running deployed client from Cliente working directory...
pushd "%CLIENT_DIR%"
start "" "%CLIENT_EXE%"
popd

echo.
echo Deployed and launched the exact Global Release build.
echo Expected diagnostics after startup:
echo   %CLIENT_DIR%\ModernBMD_boot.log
echo   %CLIENT_DIR%\ModernBMD_render.log
echo   %CLIENT_DIR%\Data\ModernBMD.log

endlocal
exit /b 0
