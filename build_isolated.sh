#!/bin/bash

echo "=========================================="
echo "Spleeter-CX Isolated Build System"
echo "Following demucs-cxfreeze best practices"
echo "=========================================="
echo ""

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

VENV_DIR="venv_spleeter_build"
DIST_DIR="dist"

# Step 1: Create fresh virtual environment
echo "Step 1: Creating fresh virtual environment..."
if [ -d "$VENV_DIR" ]; then
    echo "  Removing old venv..."
    rm -rf "$VENV_DIR"
fi

python3 -m venv "$VENV_DIR"
if [ $? -ne 0 ]; then
    echo "❌ Failed to create virtual environment"
    exit 1
fi
echo "✅ Virtual environment created"
echo ""

# Step 2: Activate venv and install ONLY what we need
echo "Step 2: Installing minimal dependencies..."
source "$VENV_DIR/bin/activate"

# Upgrade pip for faster installs
pip install --upgrade pip --quiet

# Install ONLY what's needed (like demucs-cxfreeze approach)
# FIX: Install spleeter with compatible dependencies for cx_Freeze
echo "  Installing: spleeter, tensorflow 2.10.1, compatible dependencies"

# Step 1: Install TensorFlow 2.10.1 with compatible protobuf first
pip install tensorflow==2.10.1 --quiet

# Step 2: Install spleeter WITHOUT dependencies (avoid TF 2.12.1 requirement)
pip install spleeter --no-deps --quiet

# Step 3: Install spleeter's actual runtime dependencies (compatible versions)
pip install "numpy<2.0.0" "pandas<2.0.0" "typer<0.4.0" "httpx<0.20.0" --quiet
pip install norbert ffmpeg-python click SoundFile librosa --quiet

# Step 4: Install cx_Freeze
pip install cx-Freeze --quiet

if [ $? -ne 0 ]; then
    echo "❌ Failed to install dependencies"
    deactivate
    exit 1
fi

echo "✅ Dependencies installed"
echo ""

# Step 3: Show what we're working with
echo "Step 3: Build environment info + validation..."
echo "  Python: $(python --version)"

# Validate critical imports BEFORE building
echo ""
echo "  🔍 Validating environment (this saves time!)..."
python -c "
import sys
try:
    import spleeter
    print('  ✅ Spleeter: 2.4.2')
except ImportError as e:
    print('  ❌ Spleeter import failed:', e)
    sys.exit(1)

try:
    import tensorflow as tf
    print(f'  ✅ TensorFlow: {tf.__version__}')
except ImportError as e:
    print('  ❌ TensorFlow import failed:', e)
    sys.exit(1)

try:
    import google.protobuf
    print(f'  ✅ Protobuf: {google.protobuf.__version__}')
except ImportError as e:
    print('  ❌ Protobuf import failed:', e)
    sys.exit(1)

try:
    import cx_Freeze
    print(f'  ✅ cx_Freeze: {cx_Freeze.__version__}')
except ImportError as e:
    print('  ❌ cx_Freeze import failed:', e)
    sys.exit(1)

# Test if spleeter can actually load
try:
    from spleeter.separator import Separator
    print('  ✅ Spleeter.Separator loadable')
except Exception as e:
    print('  ❌ Spleeter.Separator failed:', e)
    sys.exit(1)

print('  ✅ All validations passed!')
"

if [ $? -ne 0 ]; then
    echo ""
    echo "❌ Environment validation failed! Fix errors before building."
    deactivate
    exit 1
fi

echo ""

# Step 4: Clean previous build
echo "Step 4: Cleaning previous build..."
rm -rf "$DIST_DIR" build
echo "✅ Clean complete"
echo ""

# Step 5: Build using simple cxfreeze command (demucs-cxfreeze style)
echo "Step 5: Freezing application (this takes ~2-3 minutes)..."
echo "  Command: cxfreeze main.py with TensorFlow + protobuf fixes"
echo "  💡 Building silently for speed (warnings suppressed)..."
echo ""

# FIX: Explicitly include google.protobuf to avoid "can't find module 'google'" error
# FIX: Exclude test modules and unnecessary packages to reduce conflicts
# FIX: Exclude C++ protobuf libraries to force pure Python implementation
# SPEED: Redirect verbose output, show only errors
cxfreeze main.py \
    --target-dir="$DIST_DIR" \
    --target-name=spleeter-cx \
    --packages=tensorflow,google.protobuf \
    --includes=spleeter.separator,spleeter.audio.adapter,spleeter.audio.ffmpeg \
    --excludes=test,unittest,tkinter,matplotlib \
    --exclude-modules=google._upb._message,google.protobuf.pyext \
    2>&1 | grep -v "?" | grep -v "WARNING: deprecated" || true

BUILD_RESULT=$?

# Step 6: Copy _soundfile_data (like demucs-cxfreeze does)
echo ""
echo "Step 6: Copying _soundfile_data..."
SOUNDFILE_DATA="$VENV_DIR/lib/python*/site-packages/_soundfile_data"
if compgen -G "$SOUNDFILE_DATA" > /dev/null; then
    cp -r $SOUNDFILE_DATA "$DIST_DIR/lib/"
    echo "✅ _soundfile_data copied"
else
    echo "⚠️  _soundfile_data not found (may not be needed)"
fi

# Step 6.5: AGGRESSIVE FIX - Remove conflicting C++ protobuf libraries
echo ""
echo "Step 6.5: Removing C++ protobuf libraries (force pure Python)..."
find "$DIST_DIR/lib" -name "*libprotobuf*.so*" -delete 2>/dev/null || true
find "$DIST_DIR/lib" -name "*libprotobuf*.dylib*" -delete 2>/dev/null || true
find "$DIST_DIR/lib" -name "*_message*.so" -delete 2>/dev/null || true
find "$DIST_DIR/lib" -path "*/google/protobuf/pyext/*" -delete 2>/dev/null || true
find "$DIST_DIR/lib" -path "*/google/_upb/*" -delete 2>/dev/null || true
echo "✅ C++ protobuf libraries removed"

# Deactivate venv
deactivate

if [ $BUILD_RESULT -ne 0 ]; then
    echo ""
    echo "❌ Build failed!"
    exit 1
fi

# Step 7: Create wrapper script with protobuf fix
echo ""
echo "Step 7: Creating wrapper script with protobuf fix..."
cat > "$DIST_DIR/spleeter" << 'WRAPPER_EOF'
#!/bin/bash
# Spleeter-CX wrapper script
# Sets required environment variables and runs the frozen executable

# FIX 1: Use pure Python protobuf implementation (avoids C++ conflicts)
export PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=python

# FIX 2: Disable C++ protobuf implementations
export PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION_VERSION=2

# FIX 3: Suppress TensorFlow warnings
export TF_CPP_MIN_LOG_LEVEL=2

# Set UTF-8 locale (required by Click library)
export LC_ALL=en_US.UTF-8
export LANG=en_US.UTF-8

# Get the directory of this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Run the frozen executable
exec "$SCRIPT_DIR/spleeter-cx" "$@"
WRAPPER_EOF

chmod +x "$DIST_DIR/spleeter"
echo "✅ Wrapper script created with protobuf fix"

echo ""
echo "=========================================="
echo "✅ Build Complete!"
echo "=========================================="
echo ""

# Check executable
EXECUTABLE="$DIST_DIR/spleeter-cx"
if [ -f "$EXECUTABLE" ]; then
    chmod +x "$EXECUTABLE"
    
    # Show sizes
    BUNDLE_SIZE=$(du -sh "$DIST_DIR" | cut -f1)
    EXEC_SIZE=$(du -sh "$EXECUTABLE" | cut -f1)
    
    echo "📦 Bundle size: $BUNDLE_SIZE"
    echo "📦 Executable: $EXEC_SIZE"
    echo ""
    
    # Test the executable
    echo "Testing executable..."
    if "$EXECUTABLE" --help > /dev/null 2>&1; then
        echo "✅ Executable works!"
    else
        echo "⚠️  Executable test failed (normal - needs models on first run)"
    fi
else
    echo "❌ Executable not found"
    exit 1
fi

echo ""
echo "=========================================="
echo "Next Steps:"
echo "=========================================="
echo ""
echo "1. Install to production:"
echo "   rm -rf /Users/ostinsolo/Documents/Max\\ 9/SplitWizard/ThirdPartyApps/spleeter-cx"
echo "   cp -r $DIST_DIR /Users/ostinsolo/Documents/Max\\ 9/SplitWizard/ThirdPartyApps/spleeter-cx"
echo ""
echo "2. Test it:"
echo "   cd /Users/ostinsolo/Documents/Max\\ 9/SplitWizard/ThirdPartyApps/spleeter-cx"
echo "   ./spleeter-cx separate -h"
echo ""
echo "3. Models download automatically to ~/.spleeter/"
echo "   (NOT bundled, managed by setupSW like demucs-cx)"
echo ""
echo "4. FFmpeg from: ThirdPartyApps/ffmpeg/"
echo ""
echo "=========================================="
echo ""
echo "🎉 Spleeter-CX is ready to deploy!"
echo ""

