:start
@set choice=
@set /p choice=Are you sure you would like to completely rebuild and redeploy the application? (y/n)
if '%choice%'=='n' goto end
if not '%choice%'=='y' goto start

set QT_DIR=C:\Qt
set QT_VERSION=5.15.2
set QT_INSTALLER_VERSION=4.3
set MSVC_VERSION=2019
set SEVENZIP_APP=C:\Program Files\7-Zip\7z
set PATH=%QT_DIR%\%QT_VERSION%\msvc%MSVC_VERSION%\bin;%QT_DIR%\Tools\QtInstallerFramework\%QT_INSTALLER_VERSION%\bin;%QT_DIR%\Tools\QtCreator\bin;%QT_DIR%\Tools\QtCreator\bin\jom;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%

rmdir /s /q ..\bin32
mkdir ..\bin32
mkdir ..\bin32\pkgconfig
mkdir ..\bin32\plugins

cd ..

rem Uncomment the following line to skip actually building the SW
rem goto skip_build

rmdir /s /q .\build-32_bit-release
mkdir build-32_bit-release
cd build-32_bit-release
call vcvarsall.bat x86
@echo on
qmake.exe ..\src\DMHelper.pro -spec win32-msvc "CONFIG+=qtquickcompiler" 
jom.exe /d /f Makefile qmake_all
jom.exe /d /f Makefile.Release
goto build_done

:skip_build
cd build-32_bit-release

:build_done

xcopy .\release\DMHelper.exe ..\bin32\
xcopy ..\src\binsrc\* ..\bin32\*
xcopy /s ..\src\binsrc\pkgconfig\* ..\bin32\pkgconfig\*
xcopy /s ..\src\binsrc\plugins\* ..\bin32\plugins\*

windeployqt --compiler-runtime --no-opengl-sw --no-angle --no-svg ..\bin32

:end
pause
