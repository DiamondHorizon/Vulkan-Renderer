@echo off
setlocal enabledelayedexpansion

set "folder=shaders"
set "logFile=updated_files.txt"
set "compiledFlag=0"  :: Flag to track compilations

:: Clear previous log
> "%logFile%" echo Updated files:

:: Loop through all .vert and .frag files in the folder
for %%F in ("%folder%\*.vert" "%folder%\*.frag") do (
    set "sourceFile=%%F"
    set "spvFile=!sourceFile!.spv"

    echo Checking existence of: !spvFile!

    if not exist "!spvFile!" (
        echo No existing SPV file, marking for compilation: !sourceFile!
        echo !sourceFile!>> "%logFile%"
        set "compiledFlag=1"
    ) else (
        for /f %%A in ('powershell -command "(Get-Item '!sourceFile!').LastWriteTimeUtc.ToString('yyyyMMddHHmmss')"') do set "sourceModified=%%A"
        for /f %%B in ('powershell -command "(Get-Item '!spvFile!').LastWriteTimeUtc.ToString('yyyyMMddHHmmss')"') do set "spvCreation=%%B"

        echo Source: !sourceFile! - Modified: !sourceModified!
        echo SPV: !spvFile! - Created: !spvCreation!

        if "!sourceModified!" GTR "!spvCreation!" (
            echo Updated shader detected: !sourceFile!
            echo !sourceFile!>> "%logFile%"
            set "compiledFlag=1"
        )
    )
)
echo Done! Updated .vert and .frag files are stored in "%logFile%"

set "glslc=C:\VulkanSDK\1.4.313.0\Bin\glslc.exe"

:: Process each file in the list (skip the first line)
for /f "skip=1 tokens=* delims=" %%F in ('type updated_files.txt ^| findstr /r /v "^$"') do (
    echo Compiling: "%glslc%" "%%F" -o "%%F.spv"
    "%glslc%" "%%F" -o "%%F.spv"
)

:: Check if anything was marked for compilation
if %compiledFlag%==0 (
    echo No Compilations Needed! >> "%logFile%"
    echo No Compilations Needed!
) else (
    echo Compilation complete!
)