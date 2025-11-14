# spleeter-cxfreeze

Self-contained frozen Python application for [Spleeter](https://github.com/deezer/spleeter) audio source separation.

Following the proven approach of [demucs-cxfreeze](https://github.com/stemrollerapp/demucs-cxfreeze).

## Features

- ✅ **Self-contained**: Python 3.10 + TensorFlow 2.12.1 + Spleeter 2.4.2
- ✅ **No installation required**: Just download and run
- ✅ **Multiple stem modes**: 2/4/5 stems
- ✅ **Isolated build**: Clean virtual environment ensures reproducibility
- ✅ **Cross-platform**: macOS, Windows, Linux (with platform-specific builds)

## Download

**Pre-built releases** available on the [Releases](https://github.com/ostinsolo/spleeter-cxfreeze/releases) page.

### Size
- Bundle: ~2.4 GB (includes TensorFlow)
- Executable: 152 KB

## Usage

### macOS

```bash
cd spleeter-cx
./spleeter separate -i audio.mp3 -p spleeter:4stems -o output/
```

### Windows

```cmd
cd spleeter-cx
spleeter.exe separate -i audio.mp3 -p spleeter:4stems -o output
```

### Available Models

| Model | Stems | Description |
|-------|-------|-------------|
| `spleeter:2stems` | 2 | Vocals, Accompaniment |
| `spleeter:4stems` | 4 | Vocals, Drums, Bass, Other |
| `spleeter:5stems` | 5 | Vocals, Drums, Bass, Piano, Other |

### Examples

**4-stem separation:**
```bash
./spleeter separate -i song.mp3 -p spleeter:4stems -o output/
```

**2-stem separation (vocals/accompaniment):**
```bash
./spleeter separate -i song.wav -p spleeter:2stems -o output/
```

**Process first 60 seconds:**
```bash
./spleeter separate -i song.mp3 -p spleeter:4stems -d 60 -o output/
```

**MP3 output with custom bitrate:**
```bash
./spleeter separate -i song.mp3 -p spleeter:4stems -c mp3 -b 256k -o output/
```

## Building from Source

### Prerequisites

- Python 3.10+
- Internet connection (for downloading dependencies)

### macOS / Linux

```bash
./build_isolated.sh
```

### Manual Build

```bash
# Create virtual environment
python3 -m venv venv_spleeter_build
source venv_spleeter_build/bin/activate  # On Windows: venv_spleeter_build\Scripts\activate

# Install dependencies
pip install spleeter cx-Freeze SoundFile

# Freeze application
cxfreeze main.py --target-dir=dist --target-name=spleeter-cx --packages=tensorflow --includes=spleeter.separator

# Copy SoundFile data (macOS/Linux)
cp -r venv_spleeter_build/lib/python*/site-packages/_soundfile_data dist/lib/

# On Windows
# copy venv_spleeter_build\Lib\site-packages\_soundfile_data dist\lib\

# Create wrapper script (macOS/Linux only)
# See build_isolated.sh for wrapper script creation
```

## Dependencies

### Bundled
- Python 3.10
- Spleeter 2.4.2
- TensorFlow 2.12.1
- NumPy, Pandas, and all Spleeter dependencies

### Not Bundled (External)
- **Models**: Download automatically to `~/.spleeter/` on first run
- **FFmpeg**: Required, must be in PATH or specify with `-a` option

#### Installing FFmpeg

**macOS:**
```bash
brew install ffmpeg
```

**Windows:**
- Download from [ffmpeg.org](https://ffmpeg.org/download.html)
- Add to PATH

**Linux:**
```bash
sudo apt install ffmpeg  # Debian/Ubuntu
sudo yum install ffmpeg  # RHEL/CentOS
```

## Technical Details

### Build Method

Following [demucs-cxfreeze](https://github.com/stemrollerapp/demucs-cxfreeze) approach:

1. **Isolated Virtual Environment**: Fresh venv with only required packages
2. **Minimal Dependencies**: Only `spleeter`, `cx-Freeze`, `SoundFile`
3. **Simple Freeze Command**: `cxfreeze main.py`
4. **SoundFile Data**: Copy `_soundfile_data` to `dist/lib/`
5. **UTF-8 Wrapper**: Script sets required environment variables (macOS/Linux)

### Advantages

- ✅ **Fast builds**: Clean environment avoids conflicts
- ✅ **Reproducible**: Same result every time
- ✅ **Smaller bundle**: Only what's needed
- ✅ **No system pollution**: Isolated from system Python

### File Structure

```
spleeter-cx/
├── spleeter           # Wrapper script (macOS/Linux)
├── spleeter.exe       # Wrapper script (Windows)
├── spleeter-cx        # Frozen executable
├── lib/               # Python libraries (~2.4 GB)
├── frozen_application_license.txt
└── README.txt
```

## Comparison with Other Tools

| Tool | Size | Backend | Speed | Quality | Stems |
|------|------|---------|-------|---------|-------|
| **Spleeter-CX** | 2.4GB | TensorFlow | ⚡⚡ ~5x | Good | 2/4/5 |
| **Demucs-CX** | 1.3GB | PyTorch | ⚡ ~2x | Excellent | 4/6 |
| **SpleeterRT** | 150MB | C/SIMD | ⚡⚡⚡ 16x | Very Good | 4 |

### When to Use Spleeter-CX

✅ Need **flexibility** (2/4/5 stems)  
✅ Good balance of speed and quality  
✅ Industry-standard Spleeter implementation  
✅ Don't have GPU  

## Troubleshooting

### "RuntimeError: Click will abort... ASCII encoding"

**Solution**: Use the `spleeter` wrapper script (not `spleeter-cx` directly). The wrapper sets required UTF-8 environment variables.

### Models not downloading

- Check internet connection
- Models download to `~/.spleeter/pretrained_models/`
- First run takes longer (~200MB per model)

### FFmpeg not found

- Install ffmpeg and add to PATH, or
- Specify custom ffmpeg with `-a` option

### Build errors

- Ensure Python 3.10+
- Use fresh virtual environment
- Check internet connection

## Credits

- **Spleeter**: [Deezer Research](https://github.com/deezer/spleeter)
- **demucs-cxfreeze**: [stemrollerapp](https://github.com/stemrollerapp/demucs-cxfreeze) - Build approach inspiration
- **cx_Freeze**: [Anthony Tuininga](https://github.com/marcelotduarte/cx_Freeze)

## License

This freezing wrapper follows the same license as Spleeter (MIT License).

Spleeter itself is MIT Licensed by Deezer Research.

## Links

- [Spleeter Repository](https://github.com/deezer/spleeter)
- [Spleeter Documentation](https://github.com/deezer/spleeter/wiki)
- [demucs-cxfreeze Reference](https://github.com/stemrollerapp/demucs-cxfreeze)
- [cx_Freeze Documentation](https://cx-freeze.readthedocs.io/)

---

**Built with ❤️ following proven open-source practices**

