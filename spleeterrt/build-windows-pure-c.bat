@echo off
REM SpleeterRT Windows Build Script - PURE C VERSION (no OpenBLAS)
REM This build is slower but has no DLL dependencies (smaller package)

setlocal
set GCC=C:\msys64\ucrt64\bin\gcc.exe
set BUILD_DIR=build_pure_c

echo ===============================================
echo SpleeterRT Windows Build - PURE C (no OpenBLAS)
echo ===============================================

if not exist "%GCC%" (
    echo ERROR: GCC not found
    exit /b 1
)

if not exist %BUILD_DIR% mkdir %BUILD_DIR%

echo.
echo [1/3] Compiling source files (pure C, no BLAS)...
%GCC% -O3 -std=c11 -fopenmp -Wall -Isrc -Ithird_party/dr_libs -Ithird_party/libsamplerate -c src/spleeterrt_4stems_offline_wiener.c -o %BUILD_DIR%/main.o
%GCC% -O3 -std=c11 -fopenmp -c src/spleeter.c -o %BUILD_DIR%/spleeter.o -Isrc
%GCC% -O3 -std=c11 -fopenmp -c src/stftFix.c -o %BUILD_DIR%/stftFix.o -Isrc
%GCC% -O3 -std=c11 -fopenmp -c src/codelet.c -o %BUILD_DIR%/codelet.o -Isrc
%GCC% -O3 -std=c11 -c src/cpthread.c -o %BUILD_DIR%/cpthread.o -Isrc
%GCC% -O3 -std=c11 -fopenmp -c src/gemm.c -o %BUILD_DIR%/gemm.o -Isrc
%GCC% -O3 -std=c11 -c src/im2col_dilated.c -o %BUILD_DIR%/im2col.o -Isrc
if errorlevel 1 goto :error

echo [2/3] Compiling libsamplerate...
%GCC% -O3 -std=c11 -Ithird_party/libsamplerate -c third_party/libsamplerate/samplerate.c -o %BUILD_DIR%/samplerate.o
%GCC% -O3 -std=c11 -Ithird_party/libsamplerate -c third_party/libsamplerate/src_sinc.c -o %BUILD_DIR%/src_sinc.o
if errorlevel 1 goto :error

echo [3/3] Linking (pure C, no OpenBLAS)...
%GCC% -O3 %BUILD_DIR%/main.o %BUILD_DIR%/spleeter.o %BUILD_DIR%/stftFix.o %BUILD_DIR%/codelet.o %BUILD_DIR%/cpthread.o %BUILD_DIR%/gemm.o %BUILD_DIR%/im2col.o %BUILD_DIR%/samplerate.o %BUILD_DIR%/src_sinc.o -o %BUILD_DIR%/spleeterrt-offline.exe -lm -fopenmp
if errorlevel 1 goto :error

echo.
echo ===============================================
echo PURE C BUILD SUCCESSFUL!
echo ===============================================
echo.
echo Output: %BUILD_DIR%\spleeterrt-offline.exe
echo Size: 
dir %BUILD_DIR%\spleeterrt-offline.exe | findstr spleeterrt
echo.
echo NOTE: This version is SLOWER but has no DLL dependencies
goto :end

:error
echo BUILD FAILED!
exit /b 1

:end
endlocal
