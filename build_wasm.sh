#!/bin/bash

# WebAssembly build script for Torpedo
# Requires Emscripten SDK to be installed

if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten not found. Please install emsdk:"
    echo "  git clone https://github.com/emscripten-core/emsdk.git"
    echo "  cd emsdk"
    echo "  ./emsdk install latest"
    echo "  ./emsdk activate latest"
    echo "  source ./emsdk_env.sh"
    exit 1
fi

echo "Building Torpedo WebAssembly..."

emcc -O2 -s WASM=1 \
    -s EXPORTED_FUNCTIONS='["_torpedo_encode_js", "_torpedo_decode_js", "_torpedo_get_capacity_js", "_malloc", "_free"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "getValue", "setValue", "stringToUTF8", "UTF8ToString", "HEAPU8"]' \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='TorpedoModule' \
    src/torpedo.c src/image.c src/lsb.c src/crypto.c src/canvas_image.c src/wasm_wrapper.c \
    -o web/torpedo.js

if [ $? -eq 0 ]; then
    echo "✓ WebAssembly build complete!"
    echo "Open web/index.html in your browser to test."
else
    echo "✗ Build failed"
    exit 1
fi