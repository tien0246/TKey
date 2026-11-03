import SwiftUI
import AppKit

enum AboutTKey {
    static var version: String {
        Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "1.0"
    }

    static func show() {
        NSApp.activate(ignoringOtherApps: true)
        NSApp.orderFrontStandardAboutPanel(
            options: [
                .applicationName: "TKey",
                .applicationVersion: version,
                .credits: NSAttributedString(
                    string: "Bộ gõ tiếng Việt cho macOS — Telex · VNI · VIQR\n© 2026 tien0246",
                    attributes: [.font: NSFont.systemFont(ofSize: 11)]
                )
            ]
        )
    }
}
