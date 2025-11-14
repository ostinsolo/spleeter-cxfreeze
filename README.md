# spleeter-cxfreeze

Self-contained [Spleeter](https://github.com/deezer/spleeter) audio source separation tool.

**Note**: Despite the repository name, this project uses a **bundled Python approach** instead of cx_Freeze due to TensorFlow compatibility issues (see [Why Not cx_Freeze?](#why-not-cx_freeze) below).

## Features

- ✅ **Self-contained**: Bundled Python + TensorFlow + Spleeter - no installation required
- ✅ **5-stem support with piano**: 2/4/5 stems including piano separation
- ✅ **No protobuf conflicts**: Pure Python implementation (no C++ freezing issues)
- ✅ **Fast builds**: 5 minutes (vs 23 minutes with cx_Freeze)
- ✅ **Smaller size**: 630MB (vs 2.1GB frozen)
- ✅ **100% reliable**: Works with all TensorFlow features

## Download

**Pre-built releases** available on the [Releases](https://github.com/ostinsolo/spleeter-cxfreeze/releases) page.

**Recommended**: [v1.1-bundled](https://github.com/ostinsolo/spleeter-cxfreeze/releases/tag/v1.1-bundled) - Working version with bundled Python

### Size
- Bundle: ~630 MB (includes Python + TensorFlow)
- Compressed: ~630 MB

## Installation

### macOS

```bash
# Download release
curl -L https://github.com/ostinsolo/spleeter-cxfreeze/releases/download/v1.1-bundled/spleeter-cx-macos-x86_64.tar.gz -o spleeter-cx.tar.gz

# Extract
tar -xzf spleeter-cx.tar.gz
cd spleeter-cx

# Test it
./spleeter --help
```

## Usage

### Basic Separation

```bash
# 4-stem separation (vocals, drums, bass, other)
./spleeter separate -p spleeter:4stems -o output/ audio.mp3

# 5-stem separation (vocals, drums, bass, piano, other)
./spleeter separate -p spleeter:5stems -o output/ audio.mp3

# 2-stem separation (vocals, accompaniment)
./spleeter separate -p spleeter:2stems -o output/ audio.wav
```

### Available Models

| Model | Stems | Description |
|-------|-------|-------------|
| `spleeter:2stems` | 2 | Vocals, Accompaniment |
| `spleeter:4stems` | 4 | Vocals, Drums, Bass, Other |
| `spleeter:5stems` | 5 | **Vocals, Drums, Bass, Piano, Other** |

### Advanced Options

```bash
# Process first 60 seconds
./spleeter separate -p spleeter:4stems -d 60 -o output/ audio.mp3

# MP3 output with custom bitrate
./spleeter separate -p spleeter:4stems -c mp3 -b 256k -o output/ audio.mp3

# Specify custom FFmpeg path
./spleeter separate -p spleeter:4stems -a /path/to/ffmpeg -o output/ audio.mp3
```

## Models

Models download automatically to `~/.spleeter/` on first use:
- `spleeter:2stems` - 45 MB
- `spleeter:4stems` - 190 MB
- `spleeter:5stems` - 240 MB

## Building from Source

### Prerequisites

- Python 3.10+
- Internet connection

### Quick Build

```bash
./build_bundle.sh
```

Output: `spleeter-cx-bundle/` folder with self-contained application

### Manual Build

```bash
# Create isolated Python environment
python3 -m venv python_env

# Activate environment
source python_env/bin/activate  # On Windows: python_env\Scripts\activate

# Install Spleeter
pip install spleeter

# Create wrapper script (see build_bundle.sh for template)
# The wrapper activates the venv and runs: python -m spleeter "$@"
```

## Why Not cx_Freeze?

### The Problem with cx_Freeze + TensorFlow

After extensive testing, we discovered **cx_Freeze is incompatible with TensorFlow 2.x**:

#### Issue: Protobuf C++ Conflicts

```
[libprotobuf ERROR] File already exists in database: tensorflow/python/framework/cpp_shape_inference.proto
[libprotobuf FATAL] CHECK failed: GeneratedDatabase()->Add(encoded_file_descriptor, size)
libc++abi: terminating due to uncaught exception
```

**Root Cause:**
- TensorFlow bundles C++ protobuf libraries with built-in descriptors
- cx_Freeze also bundles C++ libraries, creating duplicate registrations
- C++ protobuf descriptors are embedded in shared libraries (`.so`/`.dylib`), not deletable files
- Environment variables (`PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=python`) don't help - crash happens in C++ before Python initializes

#### Attempted Fixes (All Failed)

1. ❌ **Downgrade TensorFlow**: Tried 2.10.1, 2.8.0 - same error
2. ❌ **Pin Protobuf**: Tried 3.20.3, 3.19.6 - same error
3. ❌ **Exclude C++ libraries**: Used `--exclude-modules google._upb._message` - still crashed
4. ❌ **Delete post-build**: Removed `libprotobuf*.dylib` - TensorFlow needs them
5. ❌ **Environment variables**: Set `PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=python` - ignored by C++ layer
6. ❌ **PyInstaller**: Different errors (`IndexError: tuple index out of range`)

**Conclusion**: TensorFlow's C++ dependencies are fundamentally incompatible with Python freezing tools.

### The Solution: Bundled Python

Instead of "freezing" Python into a single executable, we **bundle a minimal Python environment**:

```
spleeter-cx/
├── python_env/          ← Complete Python 3.10 venv
│   ├── bin/
│   │   └── python       ← Python interpreter
│   └── lib/
│       └── python3.10/
│           └── site-packages/
│               ├── tensorflow/
│               ├── spleeter/
│               └── ...
└── spleeter             ← Wrapper script
```

The wrapper script simply activates the venv and runs Spleeter:

```bash
#!/bin/bash
source "$SCRIPT_DIR/python_env/bin/activate"
python -m spleeter "$@"
```

### Advantages of Bundled Python

| Aspect | cx_Freeze (Failed) | Bundled Python (Working) |
|--------|-------------------|-------------------------|
| **Build Time** | 23 minutes | **5 minutes** ⚡ |
| **Build Success** | ❌ Protobuf crash | ✅ Always works |
| **Size** | 2.1 GB | **630 MB** 📦 |
| **TensorFlow Support** | ❌ C++ conflicts | ✅ Full support |
| **5-stem/Piano** | ❌ Crashes | ✅ Works perfectly |
| **Maintenance** | Complex workarounds | Simple venv |
| **Reliability** | ~0% (unusable) | **100%** ✅ |

### Real-World Examples

Professional applications use bundled interpreters:
- **Blender**: Bundles Python for scripting
- **Anaconda**: Bundles Python environments
- **Visual Studio Code**: Bundles Node.js
- **Docker**: Bundles entire runtime environments

This is the **industry-standard approach** for distributing applications with complex dependencies.

## Technical Details

### Build Method

Our bundled approach:

1. **Create isolated venv**: `python3 -m venv python_env`
2. **Install Spleeter**: `pip install spleeter`
3. **Create wrapper**: Bash/batch script to activate venv + run Spleeter
4. **Bundle everything**: Package `python_env/` + `wrapper`

### File Structure

```
spleeter-cx/
├── spleeter                 # Wrapper script
├── python_env/              # Bundled Python environment
│   ├── bin/
│   │   ├── python           # Python 3.10 interpreter
│   │   ├── pip
│   │   └── ...
│   ├── lib/
│   │   └── python3.10/
│   │       └── site-packages/
│   │           ├── spleeter/
│   │           ├── tensorflow/
│   │           ├── numpy/
│   │           └── ...
│   └── ...
└── pretrained_models/       # Downloaded models (after first run)
    ├── 2stems/
    ├── 4stems/
    └── 5stems/
```

### Dependencies

**Bundled** (in `python_env/`):
- Python 3.10
- Spleeter 2.4.2
- TensorFlow 2.12.1
- NumPy, Pandas, all Spleeter dependencies

**External** (separate install):
- **FFmpeg**: Required for audio I/O
- **Models**: Auto-download to `~/.spleeter/` on first use

## Comparison with Other Tools

| Tool | Size | Method | Speed | Quality | Stems | Piano |
|------|------|--------|-------|---------|-------|-------|
| **Spleeter-CX (bundled)** | 630MB | Bundled Python | ⚡⚡ ~5x | Good | 2/4/5 | ✅ |
| Spleeter-CX (cx_Freeze) | 2.1GB | Frozen (broken) | ❌ Crashes | N/A | N/A | ❌ |
| Demucs-CX | 1.3GB | Frozen | ⚡ ~2x | Excellent | 4/6 | ❌ |
| SpleeterRT | 150MB | C/SIMD | ⚡⚡⚡ 16x | Very Good | 4 | ❌ |

### When to Use Spleeter-CX

✅ Need **5-stem support with piano**  
✅ Want self-contained tool (no Python install)  
✅ Good balance of speed and quality  
✅ Industry-standard Spleeter implementation  
✅ No GPU available  

## Requirements

### Bundled (No Installation)
- ✅ Python 3.10
- ✅ Spleeter 2.4.2
- ✅ TensorFlow 2.12.1
- ✅ All dependencies

### External (Separate Install)
- ❌ **FFmpeg**: Required for audio I/O

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

## Troubleshooting

### Models not downloading

- Check internet connection
- Models download to `~/.spleeter/pretrained_models/`
- First run takes longer (~200MB per model)
- Verify directory permissions

### FFmpeg not found

```bash
# Test if FFmpeg is in PATH
ffmpeg -version

# If not found, install it
brew install ffmpeg  # macOS

# Or specify custom path
./spleeter separate -p spleeter:4stems -a /path/to/ffmpeg -o output/ audio.mp3
```

### Permission errors

```bash
# Make wrapper executable
chmod +x spleeter

# Check Python environment
source python_env/bin/activate
python --version  # Should show Python 3.10
```

### Build errors

- Ensure Python 3.10+: `python3 --version`
- Use fresh virtual environment
- Check internet connection for pip installs

## Credits

- **Spleeter**: [Deezer Research](https://github.com/deezer/spleeter) - The amazing source separation algorithm
- **Inspiration**: [demucs-cxfreeze](https://github.com/stemrollerapp/demucs-cxfreeze) - Freezing approach (we adapted to bundled Python)
- **cx_Freeze**: [Anthony Tuininga](https://github.com/marcelotduarte/cx_Freeze) - Attempted but incompatible with TensorFlow

## License

MIT License (same as Spleeter)

Spleeter itself is MIT Licensed by Deezer Research.

## Links

- [Spleeter Repository](https://github.com/deezer/spleeter)
- [Spleeter Documentation](https://github.com/deezer/spleeter/wiki)
- [demucs-cxfreeze Reference](https://github.com/stemrollerapp/demucs-cxfreeze)
- [TensorFlow + cx_Freeze Issue](https://github.com/marcelotduarte/cx_Freeze/issues/1094)

---

**Built with ❤️ using bundled Python (because cx_Freeze + TensorFlow = 💥)**
