#!/bin/zsh
set -e
cd "$(dirname "$0")/.."
export DEVELOPER_DIR=${DEVELOPER_DIR:-/Applications/Xcode.app/Contents/Developer}
CXX=${CXX:-$(xcrun -f clang++)}
SDK=$(xcrun --show-sdk-path)
OUT=build/test
mkdir -p $OUT

echo "── 1/3 Engine (C++)"
$CXX -std=c++17 -isysroot "$SDK" -O1 -g -w -I. Tests/engine_tests.cpp Engine/*.cpp -o $OUT/engine_tests
$OUT/engine_tests

echo "── 2/3 Swift bridge + output"
xcrun swiftc -sdk "$SDK" -O App/TKeyBridge.swift App/TKeyOutput.swift App/CharsetCodec.swift App/FlowArrange.swift Tests/main.swift \
     -Lbuild -ltkey -lc++ -o $OUT/swift_tests
$OUT/swift_tests

echo "── 3/3 Thanh địa chỉ trình duyệt (mô phỏng gợi ý tự hoàn thành)"
$CXX -std=c++17 -isysroot "$SDK" -O1 -w -I. Tests/omnibox_sim.cpp Engine/*.cpp -o $OUT/omnibox_sim
$OUT/omnibox_sim
