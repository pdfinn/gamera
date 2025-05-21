#!/bin/sh
# Install dependencies and configure plan9port for Debian/Ubuntu

set -e

# Install required system packages
sudo apt-get update
sudo apt-get install -y gcc make git x11-utils libx11-dev libxt-dev libxext-dev

# Set install location
P9DIR="/usr/local/src/plan9port"

# Clone plan9port if it doesn’t already exist
if [ ! -d "$P9DIR" ]; then
    sudo git clone https://github.com/9fans/plan9port.git "$P9DIR"
fi

# Build and install plan9port
cd "$P9DIR"
sudo ./INSTALL

# Determine which profile file to modify
PROFILE_FILE="$HOME/.profile"
[ -n "$ZSH_VERSION" ] && PROFILE_FILE="$HOME/.zshrc"
[ -n "$BASH_VERSION" ] && PROFILE_FILE="$HOME/.bashrc"

# Add Plan 9 environment configuration
if ! grep -q 'plan9port' "$PROFILE_FILE"; then
    echo "\n# Plan 9 from User Space environment" >> "$PROFILE_FILE"
    echo "export PLAN9=$P9DIR" >> "$PROFILE_FILE"
    echo 'export PATH=$PLAN9/bin:$PATH' >> "$PROFILE_FILE"
fi

# Immediately apply changes for current session
export PLAN9=$P9DIR
export PATH=$PLAN9/bin:$PATH

# Confirm mk is available
command -v mk >/dev/null && echo "Plan 9 'mk' is now available." || echo "Error: 'mk' not found in PATH."
