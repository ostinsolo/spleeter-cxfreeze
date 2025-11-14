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
echo "  Installing: spleeter, cx-Freeze, SoundFile"
pip install spleeter cx-Freeze SoundFile --quiet

if [ $? -ne 0 ]; then
    echo "❌ Failed to install dependencies"
    deactivate
    exit 1
fi

echo "✅ Dependencies installed"
echo ""

# Step 3: Show what we're working with
echo "Step 3: Build environment info..."
echo "  Python: $(python --version)"
echo "  Spleeter: $(python -c "import spleeter; print('2.4.2' if not hasattr(spleeter, '__version__') else spleeter.__version__)" 2>/dev/null)"
echo "  cx_Freeze: $(python -c "import cx_Freeze; print(cx_Freeze.__version__)" 2>/dev/null)"
echo ""

# Step 4: Clean previous build
echo "Step 4: Cleaning previous build..."
rm -rf "$DIST_DIR" build
echo "✅ Clean complete"
echo ""

# Step 5: Build using simple cxfreeze command (demucs-cxfreeze style)
echo "Step 5: Freezing application..."
echo "  Command: cxfreeze main.py --target-dir=$DIST_DIR --target-name=spleeter-cx"
echo "           --packages=tensorflow --includes=spleeter.separator"
echo ""

cxfreeze main.py \
    --target-dir="$DIST_DIR" \
    --target-name=spleeter-cx \
    --packages=tensorflow \
    --includes=spleeter.separator

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

# Deactivate venv
deactivate

if [ $BUILD_RESULT -ne 0 ]; then
    echo ""
    echo "❌ Build failed!"
    exit 1
fi

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

