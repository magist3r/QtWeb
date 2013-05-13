@ECHO OFF
 
set QTDIR=%cd%\src\qt\
set PATH=%SystemRoot%;%SystemRoot%\system32;%QTDIR%\bin;%PATH%
 
echo Setting OpenSSL Env.
set OPENSSL=%cd%\src\qt\openssl\
set PATH=%OPENSSL%\bin;%PATH%
set LIB=%OPENSSL%\lib
set INCLUDE=%OPENSSL%\include
 
echo Setting Windows SDK Env.
set WindowsSdkDir=C:\Program Files\Microsoft SDKs\Windows\v7.1\
set PATH=%WindowsSdkDir%\Bin;%PATH%
set LIB=%WindowsSdkDir%\Lib;%LIB%
set INCLUDE=%WindowsSdkDir%\Include;%INCLUDE%
set TARGET_CPU=x86
 
echo Setting MSVC2010 Env.
set VSINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio 10.0\
set VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC
set DevEnvDir=%VSINSTALLDIR%\Common7\IDE
set PATH=%VCINSTALLDIR%\bin;%VSINSTALLDIR%\Common7\Tools;%VSINSTALLDIR%\Common7\IDE;%VCINSTALLDIR%\VCPackages;%PATH%
set INCLUDE=%VCINSTALLDIR%\include;%INCLUDE%
set LIB=%VCINSTALLDIR%\lib;%LIB%
set LIBPATH=%VCINSTALLDIR%\lib
 
echo Setting Framework Env.
set FrameworkVersion=v4.0.30319
set Framework35Version=v3.5
set FrameworkDir=%SystemRoot%\Microsoft.NET\Framework
set LIBPATH=%FrameworkDir%\%FrameworkVersion%;%FrameworkDir%\%Framework35Version%;%LIBPATH%
set PATH=%LIBPATH%;%PATH%

echo Env. ready.
title Qt Framework 4.8.4 Development Kit.