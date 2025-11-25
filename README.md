# Spleeter Audio Separation - Unified Distribution

Self-contained [Spleeter](https://github.com/deezer/spleeter) audio source separation tools for macOS.

This repository provides **two implementations** of Spleeter, each optimized for different use cases:

## 🎵 Available Variants

### 1. Spleeter-CX (Python Bundle)
**Best for: Piano separation, maximum flexibility**

- ✅ 2/4/5 stems (including piano separation)
- ✅ Bundled Python + TensorFlow (no installation required)
- ✅ Progress reporting during separation
- ✅ ~3.6x real-time speed
- 📦 Size: 630 MB (compressed: 456 MB)

### 2. SpleeterRT (C Implementation)
**Best for: Maximum speed, low overhead**

- ✅ 4 stems only (vocals, drums, bass, other)
- ✅ Ultra-fast C implementation (~1.4x real-time)
- ✅ Apple Accelerate framework optimization
- ✅ Self-contained executable (no dependencies)
- 📦 Size: 150 MB models + 163 KB executable

---

## 📥 Download

**Pre-built releases** available on the [Releases](https://github.com/ostinsolo/spleeter-cxfreeze/releases) page.

**Latest SpleeterRT Release**: [v1.5](https://github.com/ostinsolo/spleeter-cxfreeze/releases/tag/v1.5)  
**Latest Spleeter-CX Bundle**: [v1.3-unified](https://github.com/ostinsolo/spleeter-cxfreeze/releases/tag/v1.3-unified)

### Release Assets

| Asset | Size | Description |
|-------|------|-------------|
| `spleeter-cx-macos-v1.3.tar.gz` | 456 MB | Python bundle (2/4/5 stems + piano) *(v1.3-unified)* |
| `spleeterrt-macos-v1.5.tar.gz` | 72 KB | C executable (4 stems, ultra-fast) *(v1.5)* |
| `spleeterrt-models-v1.5.tar.gz` | 139 MB | Models for SpleeterRT *(v1.5)* |

---

## 🚀 Quick Start

### Option 1: Spleeter-CX (Python - with Piano)

```bash
# Download and extract
curl -L https://github.com/ostinsolo/spleeter-cxfreeze/releases/download/v1.3-unified/spleeter-cx-macos-v1.3.tar.gz | tar -xz
cd spleeter-cx

# 5-stem separation (with piano!)
./spleeter separate -p spleeter:5stems -o output/ audio.mp3

# 4-stem separation
./spleeter separate -p spleeter:4stems -o output/ audio.mp3

# 2-stem separation (vocals + accompaniment)
./spleeter separate -p spleeter:2stems -o output/ audio.mp3
```

### Option 2: SpleeterRT (C - Ultra-Fast)

```bash
# Download and extract executable
curl -L https://github.com/ostinsolo/spleeter-cxfreeze/releases/download/v1.5/spleeterrt-macos-v1.5.tar.gz | tar -xz

# Download and extract models
mkdir -p ~/Documents/Max\ 9/SplitWizard/ThirdPartyApps/Models/spleeterrt
cd ~/Documents/Max\ 9/SplitWizard/ThirdPartyApps/Models/spleeterrt
curl -L https://github.com/ostinsolo/spleeter-cxfreeze/releases/download/v1.5/spleeterrt-models-v1.5.tar.gz | tar -xz

# Run separation (4 stems only)
cd ~/Documents/Max\ 9/SplitWizard/ThirdPartyApps/spleeterrt
./spleeterrt-offline /path/to/audio.wav
```

---

## 📊 Comparison

| Feature | Spleeter-CX (Python) | SpleeterRT (C) |
|---------|---------------------|----------------|
| **Speed** | ⚡ 3.6x RTF | ⚡⚡⚡ 1.4x RTF |
| **Stems** | 2/4/5 | 4 only |
| **Piano** | ✅ Yes (5-stem) | ❌ No |
| **Quality** | Good | Good |
| **Size** | 630 MB | 150 MB |
| **Dependencies** | Bundled Python | None (system frameworks) |
| **Formats** | WAV, MP3, FLAC, OGG | WAV, MP3, FLAC |
| **Progress** | ✅ Detailed | ❌ Basic |
| **Memory** | ~800 MB | ~600 MB |

### When to Use Each

**Use Spleeter-CX if you need:**
- Piano separation (5-stem mode)
- 2-stem mode (vocals + accompaniment)
- Detailed progress reporting
- Flexible output formats

**Use SpleeterRT if you need:**
- Maximum speed (1.4x faster than Spleeter-CX)
- Minimal disk space
- Real-time preview workflows
- Batch processing multiple files

---

## 💾 Installation Details

### Spleeter-CX (Python Bundle)

**Directory Structure:**
```
spleeter-cx/
├── spleeter                 # Wrapper script
├── spleeter_with_progress.py
├── python_env/              # Bundled Python 3.10 + dependencies
│   ├── bin/python
│   └── lib/python3.10/
│       └── site-packages/
│           ├── spleeter/
│           ├── tensorflow/
│           └── ...
└── pretrained_models/       # Auto-downloaded on first run
    ├── 2stems/
    ├── 4stems/
    └── 5stems/
```

**System Requirements:**
- macOS 10.13+
- ~800 MB RAM
- 630 MB disk space
- FFmpeg (bundled or system)

### SpleeterRT (C Implementation)

**Directory Structure:**
```
spleeterrt/
├── spleeterrt-offline       # Executable (163 KB)
├── README.txt
└── VERSION.txt

Models/ (separate download)
└── spleeterrt/
    ├── drum4stems.dat       (37.5 MB)
    ├── bass4stems.dat       (37.5 MB)
    ├── accompaniment4stems.dat (37.5 MB)
    └── vocal4stems.dat      (37.5 MB)
```

**System Requirements:**
- macOS (Intel x86_64 or Apple Silicon via Rosetta)
- ~600 MB RAM
- 150 MB disk space
- No dependencies (uses Apple Accelerate)

---

## 🔧 Usage Examples

### Spleeter-CX: Piano Extraction

```bash
cd spleeter-cx

# Extract piano from a jazz recording
./spleeter separate -p spleeter:5stems -o output/ jazz_song.mp3

# Output files:
# output/jazz_song/vocals.wav
# output/jazz_song/drums.wav
# output/jazz_song/bass.wav
# output/jazz_song/piano.wav  ← 🎹 Piano stem!
# output/jazz_song/other.wav
```

### Spleeter-CX: Progress Reporting

```bash
# The bundled version provides detailed progress:
./spleeter separate -p spleeter:4stems -o output/ song.mp3

# Output:
# SPLEETER_PROGRESS: 5   - Initializing...
# SPLEETER_PROGRESS: 15  - Loading model...
# SPLEETER_PROGRESS: 25  - Model loaded
# SPLEETER_PROGRESS: 35  - Loading audio...
# SPLEETER_PROGRESS: 50  - Starting separation...
# SPLEETER_PROGRESS: 55  - Processing neural network...
# SPLEETER_PROGRESS: 75  - Writing output files...
# SPLEETER_PROGRESS: 100 - Complete!
```

### SpleeterRT: Batch Processing

```bash
cd ~/Documents/Max\ 9/SplitWizard/ThirdPartyApps/spleeterrt

# Process multiple files (ultra-fast)
for file in ~/Music/*.wav; do
  ./spleeterrt-offline "$file"
done

# Each 6-minute song processes in ~4.5 minutes
# 10x faster than Python Spleeter
```

---

## 🏗️ Building from Source

### Spleeter-CX (Python Bundle)

```bash
cd spleeter_freeze
./build_bundle.sh
```

This creates a self-contained bundle with:
- Python 3.10 virtual environment
- Spleeter 2.4.2
- TensorFlow 2.12.1
- All dependencies
- Progress reporting wrapper

### SpleeterRT (C Implementation)

SpleeterRT is pre-compiled from the [SpleeterRT project](https://github.com/james34602/SpleeterRT).

Models are extracted from official Spleeter TensorFlow checkpoints (August 6, 2024) and verified with MD5 checksums.

---

## 🧪 Model Verification

### Spleeter-CX Models

Models download automatically on first use to `~/.spleeter/pretrained_models/`

### SpleeterRT Models

**MD5 Checksums:**
```
drum4stems.dat:          7fbdd7d698248dd714c8b33b3280c923
bass4stems.dat:          8fd9d251aea1d0fee5a49169194c5edf
accompaniment4stems.dat: d5235976883fdb6b72e92c346118e9e0
vocal4stems.dat:         19daaa09fa3fdaf8bd59f848c8f8878d
```

Verify checksums:
```bash
cd ~/Documents/Max\ 9/SplitWizard/ThirdPartyApps/Models/spleeterrt
md5 *.dat
```

---

## 🚨 Why Not cx_Freeze?

### The Problem

cx_Freeze and PyInstaller are **incompatible with TensorFlow 2.x** due to C++ protobuf conflicts:

```
[libprotobuf ERROR] File already exists in database: tensorflow/python/framework/cpp_shape_inference.proto
[libprotobuf FATAL] CHECK failed: GeneratedDatabase()->Add(encoded_file_descriptor, size)
libc++abi: terminating due to uncaught exception
```

**Root Cause:**
- TensorFlow bundles C++ protobuf libraries with built-in descriptors
- Freezing tools also bundle C++ libraries, creating duplicate registrations
- C++ protobuf descriptors are embedded in shared libraries (`.so`/`.dylib`), not deletable files
- Environment variables don't help - crash happens in C++ before Python initializes

### Attempted Fixes (All Failed)

1. ❌ Downgrade TensorFlow (2.10.1, 2.8.0)
2. ❌ Pin Protobuf (3.20.3, 3.19.6)
3. ❌ Exclude C++ libraries
4. ❌ Delete post-build
5. ❌ Environment variables
6. ❌ PyInstaller (different errors)

### The Solution: Bundled Python

Instead of "freezing" Python into a single executable, we **bundle a minimal Python environment**:

```
spleeter-cx/
├── python_env/          ← Complete Python 3.10 venv
│   ├── bin/python       ← Python interpreter
│   └── lib/...          ← All dependencies
└── spleeter             ← Wrapper script
```

The wrapper simply activates the venv and runs Spleeter - **no freezing, no conflicts**.

### Advantages

| Aspect | cx_Freeze (Failed) | Bundled Python (Working) |
|--------|-------------------|-------------------------|
| **Build Time** | 23 minutes | **5 minutes** ⚡ |
| **Build Success** | ❌ Protobuf crash | ✅ Always works |
| **Size** | 2.1 GB | **630 MB** 📦 |
| **TensorFlow Support** | ❌ C++ conflicts | ✅ Full support |
| **5-stem/Piano** | ❌ Crashes | ✅ Works perfectly |
| **Maintenance** | Complex workarounds | Simple venv |
| **Reliability** | ~0% (unusable) | **100%** ✅ |

This is the **industry-standard approach** for distributing applications with complex dependencies (Blender, Anaconda, VSCode, Docker all use bundled interpreters).

---

## 🎯 Performance Benchmarks

### Test: 6-minute stereo audio (44.1kHz)

| Tool | Time | Speed | Memory |
|------|------|-------|--------|
| **SpleeterRT (C)** | **4.5 min** | 1.4x RTF | 600 MB |
| **Spleeter-CX (Python)** | **10 min** | 3.6x RTF | 800 MB |
| Python Spleeter (official) | 45 min | 16x RTF | 1.2 GB |
| Demucs-CX | 12 min | 2x RTF | 1.5 GB |

**RTF = Real-Time Factor** (lower is faster)

### Performance Notes

**Spleeter-CX:**
- Initial TensorFlow import: ~5.4 seconds (one-time overhead)
- Actual separation: Very fast (3.6x RTF)
- Progress reporting adds minimal overhead
- Supports GPU acceleration (if available)

**SpleeterRT:**
- No initialization overhead (C binary)
- Multi-threaded (uses all cores)
- Optimized for Apple Accelerate framework
- Fixed 4-stem output (no model switching)

---

## 🔗 Integration with Max/MSP

Both variants integrate seamlessly with the SplitWizard Max/MSP environment:

```javascript
// In Max/MSP Node.js script
mode cx    // Use Spleeter-CX (Python bundle)
stems 5    // 5 stems with piano
separate /path/to/audio.wav

// Or use SpleeterRT for speed
mode rt    // Use SpleeterRT (C implementation)
stems 4    // Always 4 stems
separate /path/to/audio.wav
```

### Auto-Installation

The `setupSW+.js` script automatically downloads and installs both variants:

```javascript
// Automatically downloads:
// - Spleeter-CX: 456 MB
// - SpleeterRT: 72 KB + 139 MB models
// - FFmpeg (if needed)
// - Models (on first use)
```

---

## 📚 Documentation

### Spleeter-CX
- [Build Guide](build_bundle.sh)
- [Progress Wrapper](spleeter_with_progress.py)
- [Release Notes](RELEASE_NOTES_v1.2.md)

### SpleeterRT
- [README](../SplitWizard/ThirdPartyApps/spleeterrt/README.txt)
- [Model Info](../SplitWizard/ThirdPartyApps/spleeterrt/MODELS_INFO.txt)
- [Version Info](../SplitWizard/ThirdPartyApps/spleeterrt/VERSION.txt)

---

## 🙏 Credits

### Spleeter-CX (Python Bundle)
- **Spleeter**: [Deezer Research](https://github.com/deezer/spleeter) - Original neural network
- **Inspiration**: [demucs-cxfreeze](https://github.com/stemrollerapp/demucs-cxfreeze) - Bundling approach

### SpleeterRT (C Implementation)
- **SpleeterRT**: [james34602](https://github.com/james34602/SpleeterRT) - C implementation
- **Spleeter**: [Deezer Research](https://github.com/deezer/spleeter) - Original neural network
- **Models**: Official Spleeter 4-stem TensorFlow checkpoints (August 6, 2024)

---

## 📄 License

MIT License (same as Spleeter)

Spleeter itself is MIT Licensed by Deezer Research.

---

## 🔗 Links

- [Spleeter Repository](https://github.com/deezer/spleeter)
- [Spleeter Documentation](https://github.com/deezer/spleeter/wiki)
- [SpleeterRT Repository](https://github.com/james34602/SpleeterRT)
- [demucs-cxfreeze Reference](https://github.com/stemrollerapp/demucs-cxfreeze)
- [TensorFlow + cx_Freeze Issue](https://github.com/marcelotduarte/cx_Freeze/issues/1094)

---

**Built with ❤️ for the SplitWizard ecosystem**

**Repository**: https://github.com/ostinsolo/spleeter-cxfreeze  
**Latest Release**: v1.3-unified  
**Platforms**: macOS (Intel x86_64, Apple Silicon via Rosetta)
