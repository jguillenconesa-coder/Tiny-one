#!/usr/bin/env bash
# SPDX-License-Identifier: AGPL-3.0-only
set -euo pipefail
[[ $(uname -s) == Darwin ]] || { echo 'This package must be built on macOS.' >&2; exit 1; }
repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
build_dir=${1:-"$repo_root/build"}
output_dir=${2:-"$repo_root/dist"}
version=$(awk -F= '/^CMAKE_PROJECT_VERSION:STATIC=/{print $2}' "$build_dir/CMakeCache.txt")
[[ $version =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || { echo 'Missing CMake version.' >&2; exit 1; }
app="$build_dir/PocketOne_artefacts/Release/Standalone/Pocket One.app"
vst="$build_dir/PocketOne_artefacts/Release/VST3/Pocket One.vst3"
[[ -x "$app/Contents/MacOS/Pocket One" && -f "$vst/Contents/MacOS/Pocket One" ]] || { echo 'Build standalone and VST3 first.' >&2; exit 1; }
# Fail if either deliverable is missing a promised universal architecture.
for binary in "$app/Contents/MacOS/Pocket One" "$vst/Contents/MacOS/Pocket One"; do
    lipo -verify_arch arm64 x86_64 "$binary"
done
stage_dir=$(mktemp -d "${TMPDIR:-/tmp}/pocket-one-package.XXXXXX")
trap 'rm -rf "$stage_dir"' EXIT
mkdir -p "$stage_dir/root/Applications" "$stage_dir/root/Library/Audio/Plug-Ins/VST3" "$output_dir"
ditto "$app" "$stage_dir/root/Applications/Pocket One.app"
ditto "$vst" "$stage_dir/root/Library/Audio/Plug-Ins/VST3/Pocket One.vst3"
# Local ad-hoc signatures; these are not Developer ID signatures/notarization.
codesign --force --deep --sign - "$stage_dir/root/Applications/Pocket One.app"
codesign --force --deep --sign - "$stage_dir/root/Library/Audio/Plug-Ins/VST3/Pocket One.vst3"
codesign --verify --deep --strict "$stage_dir/root/Applications/Pocket One.app"
codesign --verify --deep --strict "$stage_dir/root/Library/Audio/Plug-Ins/VST3/Pocket One.vst3"
package="$output_dir/Pocket-One-$version-macOS-universal.pkg"
pkgbuild --root "$stage_dir/root" --identifier studio.pocket.one --version "$version" --install-location / "$package"
pkgutil --payload-files "$package" | sed 's|^\./||' > "$stage_dir/payload.txt"
for path in 'Applications/Pocket One.app/Contents/MacOS/Pocket One' 'Library/Audio/Plug-Ins/VST3/Pocket One.vst3/Contents/MacOS/Pocket One'; do
    grep -Fqx "$path" "$stage_dir/payload.txt" || { echo "Package payload missing: $path" >&2; exit 1; }
done
# Portable alternative, preserving bundle metadata and symbolic links.
mkdir -p "$stage_dir/portable"
ditto "$stage_dir/root/Applications/Pocket One.app" "$stage_dir/portable/Pocket One.app"
ditto "$stage_dir/root/Library/Audio/Plug-Ins/VST3/Pocket One.vst3" "$stage_dir/portable/Pocket One.vst3"
cp "$repo_root/packaging/macos/README.txt" "$repo_root/LICENSE" "$repo_root/THIRD_PARTY_NOTICES.md" "$stage_dir/portable/"
ditto -c -k --sequesterRsrc --keepParent "$stage_dir/portable" "$output_dir/Pocket-One-$version-macOS-universal-portable.zip"
echo "Verified universal bundles, signatures and package payload: $package"
