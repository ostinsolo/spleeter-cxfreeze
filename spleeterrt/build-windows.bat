@echo off
REM SpleeterRT Windows Build Script
REM Requires: MSYS2 UCRT64 with gcc and OpenBLAS installed
REM   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-openblas

setlocal
set GCC=C:\msys64\ucrt64\bin\gcc.exe
set BUILD_DIR=build_win

echo ===============================================
echo SpleeterRT Windows Build Script
echo ===============================================

REM Check if GCC exists
if not exist "%GCC%" (
    echo ERROR: GCC not found at %GCC%
    echo Please install MSYS2 UCRT64 with: pacman -S mingw-w64-ucrt-x86_64-gcc
    exit /b 1
)

REM Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

echo.
echo [1/4] Compiling main source files...
%GCC% -O3 -std=c11 -fopenmp -DUSE_OPENBLAS -Wall -Isrc -Ithird_party/dr_libs -Ithird_party/libsamplerate -c src/spleeterrt_4stems_offline_wiener.c -o %BUILD_DIR%/main.o
if errorlevel 1 goto :error

%GCC% -O3 -std=c11 -fopenmp -DUSE_OPENBLAS -c src/spleeter.c -o %BUILD_DIR%/spleeter.o -Isrc
%GCC% -O3 -std=c11 -fopenmp -DUSE_OPENBLAS -c src/stftFix.c -o %BUILD_DIR%/stftFix.o -Isrc
%GCC% -O3 -std=c11 -fopenmp -c src/codelet.c -o %BUILD_DIR%/codelet.o -Isrc
%GCC% -O3 -std=c11 -c src/cpthread.c -o %BUILD_DIR%/cpthread.o -Isrc
%GCC% -O3 -std=c11 -fopenmp -DUSE_OPENBLAS -c src/gemm.c -o %BUILD_DIR%/gemm.o -Isrc -IC:\msys64\ucrt64\include
%GCC% -O3 -std=c11 -c src/im2col_dilated.c -o %BUILD_DIR%/im2col.o -Isrc
if errorlevel 1 goto :error

echo [2/4] Compiling libsamplerate...
%GCC% -O3 -std=c11 -Ithird_party/libsamplerate -c third_party/libsamplerate/samplerate.c -o %BUILD_DIR%/samplerate.o
%GCC% -O3 -std=c11 -Ithird_party/libsamplerate -c third_party/libsamplerate/src_sinc.c -o %BUILD_DIR%/src_sinc.o
if errorlevel 1 goto :error

echo [3/4] Linking with OpenBLAS...
%GCC% -O3 %BUILD_DIR%/main.o %BUILD_DIR%/spleeter.o %BUILD_DIR%/stftFix.o %BUILD_DIR%/codelet.o %BUILD_DIR%/cpthread.o %BUILD_DIR%/gemm.o %BUILD_DIR%/im2col.o %BUILD_DIR%/samplerate.o %BUILD_DIR%/src_sinc.o -o %BUILD_DIR%/spleeterrt-offline.exe -lopenblas -lm -fopenmp
if errorlevel 1 goto :error

echo [4/4] Copying required DLLs...
copy C:\msys64\ucrt64\bin\libopenblas.dll %BUILD_DIR%\ >nul
copy C:\msys64\ucrt64\bin\libgomp-1.dll %BUILD_DIR%\ >nul
copy C:\msys64\ucrt64\bin\libwinpthread-1.dll %BUILD_DIR%\ >nul

echo.
echo ===============================================
echo BUILD SUCCESSFUL!
echo ===============================================
echo.
echo Output files in %BUILD_DIR%\:
dir /b %BUILD_DIR%\*.exe %BUILD_DIR%\*.dll
echo.
echo To test, copy model files (*.dat) to %BUILD_DIR%\ and run:
echo   %BUILD_DIR%\spleeterrt-offline.exe input.wav output_dir\
echo.
goto :end

:error
echo.
echo ===============================================
echo BUILD FAILED!
echo ===============================================
exit /b 1

:end
endlocal
