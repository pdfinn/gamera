#!/bin/sh
# Install dependencies for building Gammera on Debian/Ubuntu systems

set -e

# Install system dependencies
sudo apt-get update
sudo apt-get install -y gcc make x11-utils libx11-dev libxt-dev libxext-dev

# Clone and install plan9port
cd /usr/local/src
sudo git clone https://github.com/9fans/plan9port.git
cd plan9port
sudo ./INSTALL
