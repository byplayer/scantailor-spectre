#!/bin/bash
# Deploy Qt's offscreen platform plugin into the app bundle.
#
# macdeployqt only deploys the platform plugin the GUI uses (cocoa), but a
# command line conversion runs on the offscreen one. Copying it is not enough:
# the plugin references Qt through @rpath, which would resolve against the Qt
# installation and load a second copy of QtCore into the process, so its Qt
# references are rewritten to the frameworks macdeployqt already put in the
# bundle - exactly what macdeployqt does to the plugins it deploys itself.
#
# Usage: deploy-offscreen-plugin.sh <path-to-app-bundle> <path-to-libqoffscreen.dylib>
set -euo pipefail

APP_BUNDLE="${1:-}"
PLUGIN_SOURCE="${2:-}"
if [ -z "$APP_BUNDLE" ] || [ -z "$PLUGIN_SOURCE" ]; then
    echo "Usage: $0 <path-to-app-bundle> <path-to-libqoffscreen.dylib>"
    exit 1
fi

if [ ! -f "$PLUGIN_SOURCE" ]; then
    echo "Offscreen platform plugin not found at $PLUGIN_SOURCE - skipping."
    exit 0
fi

PLATFORMS_DIR="$APP_BUNDLE/Contents/PlugIns/platforms"
PLUGIN="$PLATFORMS_DIR/$(basename "$PLUGIN_SOURCE")"

mkdir -p "$PLATFORMS_DIR"
cp -f "$PLUGIN_SOURCE" "$PLUGIN"
chmod u+w "$PLUGIN"

while IFS= read -r dep; do
    framework_path="${dep#@rpath/}"
    install_name_tool -change "$dep" \
        "@executable_path/../Frameworks/$framework_path" "$PLUGIN"
done < <(otool -L "$PLUGIN" | awk '/@rpath\/Qt.*\.framework/ { print $1 }')

codesign --force --sign - "$PLUGIN"
echo "Deployed $(basename "$PLUGIN") to the app bundle."
