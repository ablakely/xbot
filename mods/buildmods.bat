@echo off
REM Description: Build all modules in the current directory
setlocal enabledelayedexpansion

if exist "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" (
    pushd "C:\Program Files\Microsoft Visual Studio 10.0\VC"
    call vcvarsall.bat
    popd

    goto BUILD
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" (
    pushd "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC"
    call vcvarsall.bat
    popd

    goto BUILD
)



:WARNING
echo Visual Studio 2010 not found, make sure to run vcvarsall.bat for your versions of Visual Studio
echo or this script will fail
pause


:BUILD
set subdirs=
for /d %%a in (*) do (
    set subdirs=!subdirs! %%a
)

echo Building modules: %subdirs%

for %%a in (%subdirs%) do (
    echo Building %%a

    if exist "%%a\make.bat" (
        pushd "%%a"
        @echo on
        call make.bat
        @echo off
        popd
    ) else (
        echo make.bat not found in %%a, attempting to build %%a\%%a.c

        if exist "%%a\%%a.c" (
            @echo on
            cl /I..\lib /I..\include\libconfig-1.7.3\lib /LD /EHsc  ..\Debug\xbot.lib ..\include\libconfig.lib %%a\%%a.c
            link /DLL /out:.\%%a.dll %%a.obj ..\Debug\xbot.lib ..\include\libconfig.lib
            @echo off
        ) else (
            echo %%a\%%a.c not found
        )
    )
)

echo Done building modules
