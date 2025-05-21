#!/bin/sh
# Install dependencies for building Gammera on Debian/Ubuntu systems
set -e
sudo apt-get update
sudo apt-get install -y plan9port gcc make
