import SwiftUI
import AppKit
import ApplicationServices

final class AppState: ObservableObject {
    static let shared = AppState()

    @Published var tapActive = false

    @Published var page: SettingsPage = .typing

    @Published var excludedNow = false

    @Published var activeAppName: String = ""

    @Published var activeAppBundleID: String = ""
}

enum SettingsPage: String, CaseIterable, Identifiable {
    case typing, macros, convert, exclusion, general
    var id: String { rawValue }

    var title: String {
        switch self {
        case .typing:    return "Gõ chữ"
        case .macros:    return "Gõ tắt"
        case .convert:   return "Chuyển mã"
        case .exclusion: return "Loại trừ"
        case .general:   return "Hệ thống"
        }
    }

    var subtitle: String {
        switch self {
        case .typing:    return "Kiểu gõ, phím tắt & chính tả"
        case .macros:    return "Từ viết tắt & cụm từ nhanh"
        case .convert:   return "Unicode, TCVN3, VNI, VIQR"
        case .exclusion: return "Tự tắt khi dùng Terminal, IDE"
        case .general:   return "Quyền Trợ năng & khởi động"
        }
    }

    var icon: String {
        switch self {
        case .typing:    return "keyboard.fill"
        case .macros:    return "text.badge.plus"
        case .convert:   return "arrow.left.arrow.right.circle.fill"
        case .exclusion: return "macwindow.badge.plus"
        case .general:   return "gearshape.fill"
        }
    }
}

@MainActor
final class MainWindowController: NSObject, NSWindowDelegate {
    static let shared = MainWindowController()
    private var window: NSWindow?

    func show(_ page: SettingsPage? = nil) {
        if let page {
            AppState.shared.page = page
        }

        NSApp.setActivationPolicy(.regular)

        if let w = window {
            w.title = "TKey — \(AppState.shared.page.title)"
            w.makeKeyAndOrderFront(nil)
            w.orderFrontRegardless()
            NSApp.activate(ignoringOtherApps: true)
            return
        }

        let rootView = MainWindow()
            .environmentObject(SettingsStore.shared)
            .environmentObject(AppState.shared)
            .environmentObject(ExclusionList.shared)

        let hosting = NSHostingController(rootView: rootView)
        let w = NSWindow(contentViewController: hosting)
        w.title = "TKey"
        w.styleMask = [.titled, .closable, .miniaturizable, .resizable, .fullSizeContentView]
        w.titlebarAppearsTransparent = true
        w.titleVisibility = .hidden
        w.toolbarStyle = .unified
        w.isMovableByWindowBackground = false
        w.setContentSize(NSSize(width: 880, height: 620))
        w.minSize = NSSize(width: 780, height: 540)
        w.maxSize = NSSize(width: 1120, height: 860)
        w.center()
        w.delegate = self
        w.isReleasedWhenClosed = false
        self.window = w

        w.makeKeyAndOrderFront(nil)
        w.orderFrontRegardless()
        NSApp.activate(ignoringOtherApps: true)
    }

    func windowWillClose(_ notification: Notification) {

        DispatchQueue.main.async {
            NSApp.setActivationPolicy(.accessory)
        }
    }
}

enum ToggleHotkey: Int, CaseIterable, Identifiable {
    case controlShift = 0, optionShift = 1, commandShift = 2, off = 3
    var id: Int { rawValue }

    var title: String {
        switch self {
        case .controlShift: return "⌃⇧  Control + Shift"
        case .optionShift:  return "⌥⇧  Option + Shift"
        case .commandShift: return "⌘⇧  Command + Shift"
        case .off:          return "Không dùng"
        }
    }

    var symbols: String? {
        switch self {
        case .controlShift: return "⌃⇧"
        case .optionShift:  return "⌥⇧"
        case .commandShift: return "⌘⇧"
        case .off:          return nil
        }
    }

    var flags: CGEventFlags? {
        switch self {
        case .controlShift: return [.maskControl, .maskShift]
        case .optionShift:  return [.maskAlternate, .maskShift]
        case .commandShift: return [.maskCommand, .maskShift]
        case .off:          return nil
        }
    }

    static var current: ToggleHotkey {
        ToggleHotkey(rawValue: UserDefaults.standard.integer(forKey: "toggleHotkey")) ?? .controlShift
    }
}

enum HotkeyMonitor {
    private static var armed = false
    private static let watched: CGEventFlags = [.maskControl, .maskShift, .maskAlternate, .maskCommand]

    static func flagsChanged(_ flags: CGEventFlags) {
        guard let combo = ToggleHotkey.current.flags else { armed = false; return }
        let held = flags.intersection(watched)
        if held == combo {
            armed = true
        } else if held.isEmpty {
            if armed {
                armed = false
                SettingsStore.shared.vietnameseEnabled.toggle()
            }
        } else if !combo.isSuperset(of: held) {
            armed = false
        }
    }

    static func keyPressed() { armed = false }
}

enum MenuBarIcon {
    private static var cache: [String: NSImage] = [:]

    static func image(vietnamese: Bool, ready: Bool) -> NSImage {
        let key = "\(vietnamese)-\(ready)"
        if let img = cache[key] { return img }
        let img = makeImage(vietnamese: vietnamese, ready: ready)
        cache[key] = img
        return img
    }

    private static func makeImage(vietnamese: Bool, ready: Bool) -> NSImage {
        let letter = !ready ? "!" : (vietnamese ? "V" : "E")
        let filled = ready && vietnamese
        let size = NSSize(width: 20, height: 15)
        let image = NSImage(size: size, flipped: false) { _ in
            let box = NSRect(x: 1, y: 1, width: 18, height: 13)
            let path = NSBezierPath(roundedRect: box, xRadius: 3.5, yRadius: 3.5)
            NSColor.black.setFill()
            NSColor.black.setStroke()
            if filled {
                path.fill()
            } else {
                path.lineWidth = 1.25
                if !ready { path.setLineDash([2, 1.5], count: 2, phase: 0) }
                path.stroke()
            }
            let font = NSFont.systemFont(ofSize: 10.5, weight: .bold)
            let attrs: [NSAttributedString.Key: Any] = [.font: font, .foregroundColor: NSColor.black]
            let text = NSAttributedString(string: letter, attributes: attrs)
            let t = text.size()
            let origin = NSPoint(x: box.midX - t.width / 2, y: box.midY - t.height / 2 + 0.5)
            if filled {
                NSGraphicsContext.current?.compositingOperation = .destinationOut
            }
            text.draw(at: origin)
            NSGraphicsContext.current?.compositingOperation = .sourceOver
            return true
        }
        image.isTemplate = true
        image.accessibilityDescription = !ready
            ? "TKey chưa được cấp quyền"
            : (vietnamese ? "TKey: tiếng Việt" : "TKey: tiếng Anh")
        return image
    }
}
