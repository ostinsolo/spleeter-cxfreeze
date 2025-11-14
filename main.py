"""
Spleeter frozen application entry point
Simple wrapper that calls spleeter's entrypoint
"""
from spleeter.__main__ import entrypoint

if __name__ == '__main__':
    entrypoint()

