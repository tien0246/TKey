#!/bin/zsh
set -e
cd "$(dirname "$0")"

export DEVELOPER_DIR=${DEVELOPER_DIR:-/Applications/Xcode.app/Contents/Developer}
CXX=${CXX:-$(xcrun -f clang++)}
SWIFTC=${SWIFTC:-$(xcrun -f swiftc)}
SDK=$(xcrun --show-sdk-path)
APP=TKey.app

echo "── 1/3 Engine ──"
mkdir -p build
for f in Engine/*.cpp; do
    $CXX -std=c++17 -arch arm64 -mmacosx-version-min=26.0 -isysroot "$SDK" -O2 -w -c "$f" -o "build/$(basename ${f%.cpp}).o"
done
rm -f build/libtkey.a
ar rcs build/libtkey.a build/*.o
echo "   ✓ $(ls build/*.o | wc -l | tr -d ' ') objects → libtkey.a"

echo "── 2/3 Swift (macOS 26 Liquid Glass SDK) ──"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources"
rm -f "$APP/Contents/MacOS/TKey"
$SWIFTC \
    -sdk "$SDK" \
    -target arm64-apple-macosx26.0 \
    -O \
    -whole-module-optimization \
    App/*.swift \
    -import-objc-header Engine/tkey_bridge.h \
    -Xcc -fobjc-arc \
    build/libtkey.a \
    -lc++ \
    -framework Cocoa \
    -framework Carbon \
    -framework CoreGraphics \
    -framework ApplicationServices \
    -framework ServiceManagement \
    -framework UniformTypeIdentifiers \
    -o "$APP/Contents/MacOS/TKey"
echo "   ✓ TKey binary"

echo "── 3/3 Bundle ──"
if [ -f AppIcon.icns ]; then
    cp AppIcon.icns "$APP/Contents/Resources/AppIcon.icns"
fi
cat > "$APP/Contents/Info.plist" << 'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key><string>TKey</string>
    <key>CFBundleIdentifier</key><string>com.tien0246.tkey</string>
    <key>CFBundleName</key><string>TKey</string>
    <key>CFBundleDisplayName</key><string>TKey</string>
    <key>CFBundleIconFile</key><string>AppIcon</string>
    <key>CFBundlePackageType</key><string>APPL</string>
    <key>CFBundleShortVersionString</key><string>2.0</string>
    <key>CFBundleVersion</key><string>2</string>
    <key>LSMinimumSystemVersion</key><string>26.0</string>
    <key>LSUIElement</key><true/>
    <key>NSPrincipalClass</key><string>NSApplication</string>
</dict>
</plist>
PLIST
codesign --force --sign - "$APP" 2>/dev/null
echo "── ✓ $APP ready ──"
