#!/bin/bash
# Install dependencies and configure plan9port for Debian/Ubuntu systems

set -e

# Set install location
P9DIR="/usr/local/src/plan9port"
P9BIN="$P9DIR/bin"
P9SRC="$P9DIR/src"

# Check and install required system packages
echo "Checking system packages..."
REQUIRED_PKGS="gcc make git x11-utils libx11-dev libxt-dev libxext-dev libfontconfig1-dev libfreetype6-dev"
MISSING_PKGS=$(dpkg-query -W -f='${binary:Package}\n' $REQUIRED_PKGS 2>/dev/null | sort | uniq | comm -23 <(echo "$REQUIRED_PKGS" | tr ' ' '\n' | sort) -)

if [ -n "$MISSING_PKGS" ]; then
    echo "Installing missing packages: $MISSING_PKGS"
    sudo apt-get update
    sudo DEBIAN_FRONTEND=noninteractive apt-get install -y $MISSING_PKGS
else
    echo "All required system packages are already installed."
fi

# Clone plan9port if not already present
if [ ! -d "$P9DIR" ]; then
    echo "Cloning plan9port..."
    sudo git clone --depth 1 https://github.com/9fans/plan9port.git "$P9DIR"
    sudo chown -R "$USER":"$USER" "$P9DIR"
else
    echo "plan9port already cloned."
fi

# Build and install plan9port (non-interactive)
if [ ! -x "$P9BIN/mk" ]; then
    echo "Running plan9port INSTALL..."
    cd "$P9DIR"
    ./INSTALL </dev/null
else
    echo "Plan 9 already installed."
fi

# Build Plan 9 toolchain if missing
if [ ! -x "$P9BIN/9c" ]; then
    echo "Building Plan 9 toolchain..."
    cd "$P9SRC"
    mk all
else
    echo "Plan 9 toolchain already built."
fi

# Configure shell profile
PROFILE_FILE="$HOME/.profile"
[ -n "$BASH_VERSION" ] && PROFILE_FILE="$HOME/.bashrc"
[ -n "$ZSH_VERSION" ] && PROFILE_FILE="$HOME/.zshrc"

if ! grep -q 'plan9port' "$PROFILE_FILE" 2>/dev/null; then
    echo "Configuring shell environment in $PROFILE_FILE"
    {
        echo ""
        echo "# Plan 9 from User Space environment"
        echo "export PLAN9=$P9DIR"
        echo 'export PATH=$PLAN9/bin:$PATH'
    } >> "$PROFILE_FILE"
else
    echo "Shell profile already configured."
fi

# Export for current session
export PLAN9=$P9DIR
export PATH=$PLAN9/bin:$PATH

# Confirm availability
if command -v mk >/dev/null && command -v 9c >/dev/null; then
    echo "✅ Plan 9 environment successfully installed and configured."
else
    echo "❌ Error: Plan 9 tools not found in PATH." >&2
    exit 1
fi
