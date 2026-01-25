#!/bin/bash

# Check if tag is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <tag>"
  exit 1
fi
tag=$1
echo "$tag"
from="esp-ct4/src"
to="esp-ct4/srctags/$tag"
# Enable matching hidden files (dotfiles)
shopt -s dotglob
# Create destination including parents
mkdir -p "$to"
# Copy files (cp -r) from source to destination
# Note: We copy everything first, then remove .pio to simulate "exclude"
cp -r "$from"/* "$to/"
# Remove the excluded directory
rm -rf "$to/.pio"
# Restore shell options
shopt -u dotglob
# Git operations
git add -A
git commit -m "$tag"
git tag "$tag"
git push
git push --tags