# SpleeterRT v1.5 Release Notes

## Build Summary
- Source: `SpleeterRT_restored/spleeter_realtime_test/SpleeterRT/Executable/src/spleeterrt_4stems_offline_wiener.c`
- Compiler: `clang` on macOS 14
- Flags: `-O3 -std=c11 -Wall -Wextra -Isrc -Ithird_party/dr_libs -Ithird_party/libsamplerate`
- Linked libs: `libsamplerate`, `dr_wav/dr_mp3/dr_flac`, Accelerate BLAS, `-lm`, `-lpthread`
- Features:
  - Pure Wiener pipeline (baseline bass available via `--bass-mode=baseline`)
  - High-band projection via `--vocal-drum-sub=1`
  - Drum hiss gate via `--drum-hiss=2`
  - Phase lock & Griffin-Lim optionally enabled via flags (default OFF)
  - High-frequency vocal clamp removed in this version

## Test Inputs
Validated against:
1. `17_12_57_11_11_2025_Stream_sega_bodega_music_|_Listen_to_songs.wav`
2. `17_45_30_11_17_2025_Leftfield_-_Original_-_YouTube.wav`
3. `14_54_45_11_11_2025_Pino_Daniele_-_Chillo_è_nu_buono_guaglione_-_YouTube.wav`

## Distribution
- `spleeterrt-macos-v1.5.tar.gz` → contains `spleeterrt-offline`
- `spleeterrt-models-v1.5.tar.gz` → contains `drum/bass/accompaniment/vocal .dat`
- `code/setupSW+.js` updated to download the v1.5 tarballs

