# Spleeter-CX v1.2 - Progress Reporting Update

## What's New in v1.2

✅ **Professional progress reporting** for Max/MSP and Node.js integration  
✅ **Explicit progress messages** at every stage (5%, 15%, 25%, 35%, 50%, 55%, 75-100%)  
✅ **FFmpeg path auto-detection** - no manual PATH setup needed  
✅ **Cleaner console output** - TensorFlow warnings suppressed  
✅ **Pre-downloaded models support** - faster startup when models are already available

## Installation

```bash
# Download and extract
tar -xzf spleeter-cx-macos-v1.2.tar.gz
cd spleeter-cx

# Test it (5 stems with piano)
./spleeter separate -p spleeter:5stems -o output input.wav

# Or use the progress wrapper
./python_env/bin/python spleeter_with_progress.py input.wav -p spleeter:5stems -o output
```

## Progress Reporting

The bundled `spleeter_with_progress.py` outputs parseable progress messages:

```
SPLEETER_PROGRESS: 5     # Initializing
SPLEETER_PROGRESS: 15    # Loading model
SPLEETER_PROGRESS: 25    # Model loaded
SPLEETER_PROGRESS: 35    # Loading audio
SPLEETER_PROGRESS: 50    # Audio loaded
SPLEETER_PROGRESS: 55    # Separating (neural network processing)
SPLEETER_PROGRESS: 75    # Separation complete
SPLEETER_PROGRESS: 79-95 # Writing stems
SPLEETER_PROGRESS: 100   # Done!
```

## Performance

Test file: 2 minutes (120 seconds)

- **Processing time**: ~33 seconds
- **Real-time factor**: 3.6x
- **Model load time**: ~5 seconds (if models already downloaded)

## What's Included

- ✅ Self-contained Python environment (no system Python needed)
- ✅ TensorFlow 2.12.1 (fully compatible)
- ✅ Spleeter 2.4.2 with all dependencies
- ✅ Progress reporting wrapper
- ✅ FFmpeg integration
- ✅ 2/4/5 stem support (including piano!)

## Models

Models download automatically on first use to:
- `~/.spleeter/pretrained_models/` (default)
- Or use pre-downloaded models from: `~/Documents/Max 9/SplitWizard/ThirdPartyApps/Models/spleeter/`

## Changes from v1.1

### Added
- ✅ `spleeter_with_progress.py` - explicit progress reporting wrapper
- ✅ Automatic FFmpeg PATH detection
- ✅ TensorFlow warning suppression (cleaner output)
- ✅ Pre-downloaded model path support
- ✅ Detailed progress messages at each stage

### Fixed
- ✅ FFmpeg not found errors
- ✅ Excessive TensorFlow warning messages
- ✅ Progress reporting during neural network separation

### Improved
- ✅ Cleaner console output
- ✅ Better Max/MSP and Node.js integration
- ✅ More accurate progress tracking

## System Requirements

- **macOS**: 10.15+ (Catalina or later)
- **RAM**: 4GB minimum (8GB recommended)
- **Disk**: 2GB for bundle + models
- **Python**: Not required (bundled)

## Integration

### Node.js / Max/MSP

```javascript
const { spawn } = require('child_process');

const process = spawn('python', [
  '/path/to/spleeter-cx/spleeter_with_progress.py',
  'input.wav',
  '-p', 'spleeter:5stems',
  '-o', 'output'
]);

process.stdout.on('data', (data) => {
  const match = data.toString().match(/SPLEETER_PROGRESS:\s*(\d+)/);
  if (match) {
    console.log(`Progress: ${match[1]}%`);
  }
});
```

## Bundle Contents

```
spleeter-cx/
├── python_env/           # Complete Python environment (1.9GB)
│   ├── bin/
│   │   └── python3
│   └── lib/
│       └── python3.10/
│           └── site-packages/
│               ├── spleeter/
│               └── tensorflow/
├── spleeter              # Wrapper script (for standard Spleeter CLI)
└── spleeter_with_progress.py  # Progress reporting wrapper
```

## Testing

```bash
# Quick test (2 stems, fastest)
./spleeter separate -p spleeter:2stems -o /tmp/test input.wav

# Full test (5 stems with piano)
./python_env/bin/python spleeter_with_progress.py input.wav \
  -p spleeter:5stems \
  -o /tmp/test
```

Expected output:
- `vocals.wav`
- `piano.wav` (5-stem only)
- `drums.wav`
- `bass.wav`
- `other.wav`

## Known Issues

None! This is a stable release.

## License

Same as Spleeter: MIT License

## Credits

- **Spleeter**: Deezer Research (https://github.com/deezer/spleeter)
- **Bundled approach**: Avoids TensorFlow freezing issues
- **Progress wrapper**: Custom integration for Max/MSP and Node.js

---

**Previous version**: v1.1 (bundled Python without progress reporting)  
**GitHub**: https://github.com/ostinsolo/spleeter-cxfreeze  
**Released**: November 15, 2025

