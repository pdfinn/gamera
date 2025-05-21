#!/bin/sh
# Install dependencies and configure plan9port for Debian/Ubuntu

set -e

# Install required system packages
sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
    gcc make git x11-utils libx11-dev libxt-dev libxext-dev \
    libfontconfig1-dev libfreetype6-dev

# Set install location
P9DIR="/usr/local/src/plan9port"

# Clone plan9port if it doesn't already exist
if [ ! -d "$P9DIR" ]; then
    sudo git clone --depth 1 https://github.com/9fans/plan9port.git "$P9DIR"
fi

# Build and install plan9port (non-interactively)
cd "$P9DIR"
sudo ./INSTALL </dev/null

# Determine which profile file to modify
PROFILE_FILE="$HOME/.profile"
[ -n "$BASH_VERSION" ] && PROFILE_FILE="$HOME/.bashrc"
[ -n "$ZSH_VERSION" ] && PROFILE_FILE="$HOME/.zshrc"

# Add Plan 9 environment configuration if missing
if ! grep -q 'plan9port' "$PROFILE_FILE" 2>/dev/null; then
    echo "\n# Plan 9 from User Space environment" >> "$PROFILE_FILE"
    echo "export PLAN9=$P9DIR" >> "$PROFILE_FILE"
    echo 'export PATH=$PLAN9/bin:$PATH' >> "$PROFILE_FILE"
fi

# Immediately apply changes for current session
export PLAN9=$P9DIR
export PATH=$PLAN9/bin:$PATH

# Confirm mk is available
if command -v mk >/dev/null; then
    echo "Plan 9 'mk' is now available."
else
    echo "Error: 'mk' not found in PATH." >&2
fi
