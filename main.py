"""
Spleeter frozen application entry point
Fixed for TensorFlow 2.10.1 + Protobuf 3.20.3 compatibility
"""

# CRITICAL: Set environment BEFORE any imports
import os
import sys

# FIX 1: Use pure Python protobuf (no C++ conflicts)
os.environ['PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION'] = 'python'

# FIX 2: Suppress TensorFlow warnings
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'

# Now we can safely import
import warnings
warnings.filterwarnings('ignore', category=DeprecationWarning)
warnings.filterwarnings('ignore', message='.*already in the registry.*')
warnings.filterwarnings('ignore', message='.*Deprecated.*')

from spleeter.__main__ import entrypoint

if __name__ == '__main__':
    entrypoint()

