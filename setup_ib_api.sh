#!/bin/bash

# Setup script for IB API on macOS
echo "FattyTraders - Interactive Brokers API Setup"
echo "============================================="

# Check if we're on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "This script is designed for macOS"
    exit 1
fi

# Create directories
mkdir -p IBApi
cd IBApi

echo "Downloading IB API..."

# Download the latest IB API for C++
IB_API_URL="https://download2.interactivebrokers.com/installers/ibapi/latest-stable/IBApi_macos_x64.zip"

# Try to download using curl
if command -v curl &> /dev/null; then
    curl -L -o IBApi_macos.zip "$IB_API_URL"
elif command -v wget &> /dev/null; then
    wget -O IBApi_macos.zip "$IB_API_URL"
else
    echo "Error: Neither curl nor wget found. Please download manually from:"
    echo "$IB_API_URL"
    echo "And extract to $(pwd)/IBApi_macos.zip"
    exit 1
fi

# Check if download was successful
if [ ! -f "IBApi_macos.zip" ]; then
    echo "Download failed. Please download manually from:"
    echo "https://www.interactivebrokers.com/en/trading/ib-api.php"
    echo "Look for 'API Latest' and download the Mac version"
    exit 1
fi

echo "Extracting IB API..."
unzip -q IBApi_macos.zip

# Find the extracted directory (name may vary)
IB_DIR=$(find . -name "IBJts" -type d | head -1)
if [ -z "$IB_DIR" ]; then
    echo "Could not find IBJts directory. Please check the extraction."
    ls -la
    exit 1
fi

echo "Found IB API directory: $IB_DIR"

# Move to standard location
if [ "$IB_DIR" != "./source" ]; then
    mv "$IB_DIR" source
fi

# Build the C++ client library
echo "Building IB API library..."
cd source/cppclient

# Check if we have the right build tools
if ! command -v make &> /dev/null; then
    echo "Error: 'make' not found. Please install Xcode Command Line Tools:"
    echo "xcode-select --install"
    exit 1
fi

# Build the library
make

# Check if library was built successfully
if [ -f "lib/libTwsApi.a" ]; then
    echo "✅ IB API library built successfully!"
else
    echo "❌ Failed to build IB API library"
    echo "Manual build may be required. Check the Makefile in:"
    echo "$(pwd)"
    exit 1
fi

cd ../../..

echo ""
echo "✅ IB API setup complete!"
echo ""
echo "Next steps:"
echo "1. Build the FattyTraders connector:"
echo "   mkdir build && cd build"
echo "   cmake .. && make"
echo ""
echo "2. Make sure TWS or IB Gateway is running with API enabled"
echo ""
echo "3. Run the connector:"
echo "   ./fatty_traders"
echo ""
echo "Note: If you get compilation errors, you may need to:"
echo "- Install Xcode Command Line Tools: xcode-select --install"
echo "- Install CMake: brew install cmake"
echo ""