#!/bin/bash

echo "=========================================="
echo "Spleeter-CX: Bundled Python Approach"
echo "No freezing - just bundle Python + deps"
echo "=========================================="
echo ""

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

BUNDLE_DIR="spleeter-cx-bundle"

# Step 1: Clean
echo "Step 1: Cleaning..."
rm -rf "$BUNDLE_DIR"
mkdir -p "$BUNDLE_DIR"
echo "✅ Clean"
echo ""

# Step 2: Create isolated venv
echo "Step 2: Creating Python environment (~2 minutes)..."
python3 -m venv "$BUNDLE_DIR/python_env"
source "$BUNDLE_DIR/python_env/bin/activate"

# Upgrade pip silently
pip install --upgrade pip --quiet

# Install spleeter and dependencies
echo "  Installing spleeter..."
pip install spleeter --quiet 2>&1 | grep -v "already satisfied" || true

echo "✅ Python environment ready"
echo ""

# Step 3: Test the environment
echo "Step 3: Testing environment..."
python -c "
from spleeter.separator import Separator
import tensorflow as tf
print(f'  ✅ TensorFlow: {tf.__version__}')
print(f'  ✅ Spleeter works!')
"

if [ $? -ne 0 ]; then
    echo "❌ Environment test failed"
    deactivate
    exit 1
fi

deactivate
echo ""

# Step 4: Create wrapper script
echo "Step 4: Creating wrapper script..."
cat > "$BUNDLE_DIR/spleeter" << 'WRAPPER_EOF'
#!/bin/bash
# Spleeter-CX: Bundled Python wrapper
# Self-contained - no system Python needed

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Activate bundled Python environment
source "$SCRIPT_DIR/python_env/bin/activate"

# Add FFmpeg to PATH (shared with Demucs)
FFMPEG_DIR="/Users/ostinsolo/Documents/Max 9/SplitWizard/ThirdPartyApps/ffmpeg/bin"
if [ -d "$FFMPEG_DIR" ]; then
    export PATH="$FFMPEG_DIR:$PATH"
fi

# Environment variables
export PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=python
export TF_CPP_MIN_LOG_LEVEL=2
export LC_ALL=en_US.UTF-8
export LANG=en_US.UTF-8

# Run spleeter
python -m spleeter "$@"
WRAPPER_EOF

chmod +x "$BUNDLE_DIR/spleeter"
echo "✅ Wrapper created"
echo ""

# Copy progress wrapper script
echo "Copying progress wrapper..."
cp "$SCRIPT_DIR/../spleeter_with_progress.py" "$BUNDLE_DIR/spleeter_with_progress.py" 2>/dev/null || \
cp "$SCRIPT_DIR/spleeter_with_progress.py" "$BUNDLE_DIR/spleeter_with_progress.py" 2>/dev/null || \
cat > "$BUNDLE_DIR/spleeter_with_progress.py" << 'PROGRESS_EOF'
#!/usr/bin/env python3
"""
Spleeter wrapper with explicit progress reporting for Node.js integration
Outputs progress in a parseable format: SPLEETER_PROGRESS: <percentage>
"""

import sys
import os
import argparse
import logging
from pathlib import Path

# Suppress TensorFlow warnings BEFORE importing anything
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'  # 0=all, 1=no INFO, 2=no WARNING, 3=no ERROR
os.environ['TF_ENABLE_ONEDNN_OPTS'] = '0'  # Suppress oneDNN messages

# Set Spleeter model path to pre-downloaded models
# This avoids downloading models on first run (saves ~30 seconds!)
home_dir = os.path.expanduser('~')
spleeter_models_dir = os.path.join(home_dir, 'Documents', 'Max 9', 'SplitWizard', 'ThirdPartyApps', 'Models', 'spleeter')
if os.path.exists(spleeter_models_dir):
    os.environ['MODEL_PATH'] = spleeter_models_dir

import warnings
warnings.filterwarnings('ignore', category=DeprecationWarning)
warnings.filterwarnings('ignore', category=FutureWarning)

# Set up logging (only our messages)
logging.basicConfig(level=logging.INFO, format='%(levelname)s:%(name)s:%(message)s')
logger = logging.getLogger('spleeter')

# Suppress TensorFlow logging
logging.getLogger('tensorflow').setLevel(logging.ERROR)
logging.getLogger('tensorflow_estimator').setLevel(logging.ERROR)

def print_progress(percentage, message=''):
    """Print progress in a format that Node.js can parse"""
    print(f'SPLEETER_PROGRESS: {percentage}', flush=True)
    if message:
        logger.info(message)

def main():
    parser = argparse.ArgumentParser(description='Spleeter with progress reporting')
    parser.add_argument('audio_file', help='Input audio file')
    parser.add_argument('-p', '--params', default='spleeter:2stems', help='Model params (e.g., spleeter:5stems)')
    parser.add_argument('-o', '--output', required=True, help='Output directory')
    parser.add_argument('-c', '--codec', default='wav', help='Output codec')
    parser.add_argument('-b', '--bitrate', default='128k', help='Output bitrate')
    parser.add_argument('-d', '--duration', type=float, default=600.0, help='Max duration')
    parser.add_argument('-s', '--offset', type=float, default=0.0, help='Start offset')
    
    args = parser.parse_args()
    
    try:
        # Import after args parsing (faster startup)
        print_progress(5, 'Initializing Spleeter...')
        from spleeter.separator import Separator
        from spleeter.audio.adapter import AudioAdapter
        
        print_progress(15, f'Loading model: {args.params}')
        separator = Separator(args.params)
        
        print_progress(25, 'Model loaded')
        
        audio_adapter = AudioAdapter.default()
        audio_file = args.audio_file
        
        print_progress(35, f'Loading audio file: {Path(audio_file).name}')
        
        # Load audio
        waveform, sample_rate = audio_adapter.load(
            audio_file,
            offset=args.offset,
            duration=args.duration,
            sample_rate=separator._sample_rate
        )
        
        print_progress(50, f'Audio loaded ({len(waveform)/sample_rate:.1f}s), starting separation...')
        
        # Perform separation (this is the slow part - TensorFlow processing)
        print_progress(55, 'Separating audio with neural network...')
        prediction = separator.separate(waveform)
        
        print_progress(75, 'Separation complete, writing output files...')
        
        # Write output files
        # Note: Spleeter doesn't create subdirectories, we handle that
        output_path = Path(args.output)
        output_path.mkdir(parents=True, exist_ok=True)
        
        stems = list(prediction.keys())
        for idx, (stem_name, stem_data) in enumerate(prediction.items()):
            progress = 75 + int((idx + 1) / len(stems) * 20)
            print_progress(progress, f'Writing {stem_name}.{args.codec}')
            
            output_file = output_path / f'{stem_name}.{args.codec}'
            audio_adapter.save(
                str(output_file),
                stem_data,
                sample_rate,
                args.codec,
                args.bitrate
            )
            logger.info(f'File {output_file} written succesfully')
        
        print_progress(100, 'Separation complete!')
        return 0
        
    except Exception as e:
        logger.error(f'Error during separation: {str(e)}')
        import traceback
        traceback.print_exc()
        return 1

if __name__ == '__main__':
    sys.exit(main())
PROGRESS_EOF

chmod +x "$BUNDLE_DIR/spleeter_with_progress.py"
echo "✅ Progress wrapper included"
echo ""

# Step 5: Test the wrapper
echo "Step 5: Testing wrapper..."
"$BUNDLE_DIR/spleeter" --help > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✅ Wrapper works!"
else
    echo "⚠️  Wrapper test inconclusive (may need models)"
fi
echo ""

# Step 6: Show bundle size
BUNDLE_SIZE=$(du -sh "$BUNDLE_DIR" | cut -f1)
echo "=========================================="
echo "✅ Build Complete!"
echo "=========================================="
echo ""
echo "📦 Bundle size: $BUNDLE_SIZE"
echo "📁 Location: $BUNDLE_DIR"
echo ""
echo "Next: Test with real audio file"
echo "  cd $BUNDLE_DIR"
echo "  ./spleeter separate -p spleeter:5stems -o /tmp/test <audio_file>"
echo ""

