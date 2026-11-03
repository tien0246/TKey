// swift-tools-version: 6.0
import PackageDescription

let package = Package(
    name: "TKey",
    platforms: [.macOS(.v15)],
    targets: [
        .executableTarget(
            name: "TKey",
            path: "App",
            cSettings: [],
            linkerSettings: [
                .unsafeFlags(["-L./build", "-ltkey", "-lc++", "-lc++abi"])
            ]
        )
    ]
)
