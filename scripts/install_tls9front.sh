#!/usr/bin/env bash
# Fetch and build the 9front TLS tools.
# Installs into $PLAN9 if set, otherwise /usr/local to preserve Plan 9 style.
set -e

PREFIX="${PLAN9:-/usr/local}"
SRCDIR="$PREFIX/src/9front_tls"
REPO="https://git.9front.org/plan9front/tls"

# Clone or update the repository
if [ ! -d "$SRCDIR" ]; then
    echo "Cloning TLS tools from $REPO"
    git clone "$REPO" "$SRCDIR"
else
    echo "Updating TLS tools in $SRCDIR"
    git -C "$SRCDIR" pull
fi

cd "$SRCDIR"

echo "Building TLS tools..."
PLAN9="$PREFIX" mk clean || true
PLAN9="$PREFIX" mk

# Install with sudo if required
if [ -w "$PREFIX" ]; then
    PLAN9="$PREFIX" mk install
else
    sudo PLAN9="$PREFIX" mk install
fi
