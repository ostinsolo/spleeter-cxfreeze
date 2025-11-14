#!/bin/bash
# Download SpleeterRT models
# Similar to demucs model download in setupSW

set -e

MODELS_URL="https://github.com/ostinsolo/spleeter-cxfreeze/releases/download/models-v1.0.0/spleeterrt-models-v1.0.0.tar.gz"
MODELS_DIR="model_spleeter"

echo "=========================================="
echo "Downloading SpleeterRT Models"
echo "=========================================="
echo ""

# Check if models already exist
if [ -d "$MODELS_DIR" ] && [ "$(ls -A $MODELS_DIR/*.dat 2>/dev/null | wc -l)" -eq 4 ]; then
    echo "✅ Models already exist in $MODELS_DIR"
    echo ""
    ls -lh "$MODELS_DIR"/*.dat
    echo ""
    echo "Skip download. Delete $MODELS_DIR to re-download."
    exit 0
fi

echo "📥 Downloading models (150 MB)..."
if command -v curl &> /dev/null; then
    curl -L "$MODELS_URL" -o spleeterrt-models.tar.gz
elif command -v wget &> /dev/null; then
    wget "$MODELS_URL" -O spleeterrt-models.tar.gz
else
    echo "❌ Error: curl or wget required"
    exit 1
fi

echo "✅ Downloaded"
echo ""

echo "📦 Extracting models..."
mkdir -p "$MODELS_DIR"
tar -xzf spleeterrt-models.tar.gz -C "$MODELS_DIR"
rm spleeterrt-models.tar.gz

echo "✅ Extracted to $MODELS_DIR/"
echo ""

# Verify models
REQUIRED_MODELS=("drum4stems.dat" "bass4stems.dat" "accompaniment4stems.dat" "vocal4stems.dat")
ALL_PRESENT=true

for model in "${REQUIRED_MODELS[@]}"; do
    if [ -f "$MODELS_DIR/$model" ]; then
        SIZE=$(du -h "$MODELS_DIR/$model" | cut -f1)
        echo "  ✅ $model ($SIZE)"
    else
        echo "  ❌ $model MISSING"
        ALL_PRESENT=false
    fi
done

echo ""

if [ "$ALL_PRESENT" = true ]; then
    echo "=========================================="
    echo "✅ All SpleeterRT models installed!"
    echo "=========================================="
    exit 0
else
    echo "=========================================="
    echo "❌ Some models are missing"
    echo "=========================================="
    exit 1
fi

