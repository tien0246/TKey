import Cocoa
import ApplicationServices
import Combine

class AppDelegate: NSObject, NSApplicationDelegate {
    private var retryTimer: Timer?
    private var mouseMonitor: Any?
    private var localMouseMonitor: Any?
    private var activateObserver: NSObjectProtocol?
    private var exclusionCancellable: AnyCancellable?

    func applicationDidFinishLaunching(_ notification: Notification) {
        tk_setup()

        let ourBundleID = Bundle.main.bundleIdentifier ?? "com.tien0246.tkey"
        if let front = NSWorkspace.shared.frontmostApplication,
           front.bundleIdentifier != ourBundleID {
            TKeyTapCallback.frontBundleID = front.bundleIdentifier ?? ""
            AppState.shared.activeAppBundleID = TKeyTapCallback.frontBundleID
            AppState.shared.activeAppName = front.localizedName ?? ""
        }

        EngineConfig.applyFromDefaults()
        MacroPersistence.pushToEngine()

        TKeyTapCallback.engineSource = CGEventSource(stateID: .privateState)
        if let src = TKeyTapCallback.engineSource {
            TKeyTapCallback.sourceStateID = TKeyTapCallback.getSourceStateID(src)
        }

        if !AXIsProcessTrusted() {
            let key = kAXTrustedCheckOptionPrompt.takeUnretainedValue() as String
            AXIsProcessTrustedWithOptions([key: true] as CFDictionary)
        }
        installTapOrRetry()

        mouseMonitor = NSEvent.addGlobalMonitorForEvents(
            matching: [.leftMouseDown, .rightMouseDown, .otherMouseDown]) { _ in
            _ = tkResetBuff(0)
        }
        localMouseMonitor = NSEvent.addLocalMonitorForEvents(
            matching: [.leftMouseDown, .rightMouseDown, .otherMouseDown]) { event in
            _ = tkResetBuff(0)
            return event
        }
        activateObserver = NSWorkspace.shared.notificationCenter.addObserver(
            forName: NSWorkspace.didActivateApplicationNotification,
            object: nil, queue: .main) { note in
            _ = tkResetBuff(0)
            if let app = note.userInfo?[NSWorkspace.applicationUserInfoKey] as? NSRunningApplication,
               let bid = app.bundleIdentifier {
                TKeyTapCallback.isTKeyFrontmost = (bid == ourBundleID)
                if bid != ourBundleID {
                    TKeyTapCallback.frontBundleID = bid
                    AppState.shared.activeAppBundleID = bid
                    AppState.shared.activeAppName = app.localizedName ?? bid
                }
                TKeyTapCallback.updateVietnameseMode()
            }
        }
        exclusionCancellable = ExclusionList.shared.$apps
            .receive(on: RunLoop.main)
            .sink { _ in TKeyTapCallback.updateVietnameseMode() }
        TKeyTapCallback.updateVietnameseMode()

        let args = CommandLine.arguments
        if let idx = args.firstIndex(of: "--page"), idx + 1 < args.count,
           let page = SettingsPage(rawValue: args[idx + 1]) {
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.15) {
                MainWindowController.shared.show(page)
            }
        }
    }

    func applicationShouldHandleReopen(_ sender: NSApplication, hasVisibleWindows flag: Bool) -> Bool {
        MainWindowController.shared.show()
        return true
    }

    func applicationWillTerminate(_ notification: Notification) {
        retryTimer?.invalidate()
        if let m = mouseMonitor { NSEvent.removeMonitor(m) }
        if let m = localMouseMonitor { NSEvent.removeMonitor(m) }
        if let o = activateObserver { NSWorkspace.shared.notificationCenter.removeObserver(o) }
        TKeyTapCallback.uninstall()
        tk_cleanup()
    }

    private func installTapOrRetry() {
        if TKeyTapCallback.install() {
            AppState.shared.tapActive = true
            return
        }
        NSLog("TKey: cannot create event tap — grant Accessibility")
        AppState.shared.tapActive = false
        retryTimer = Timer.scheduledTimer(withTimeInterval: 2, repeats: true) { [weak self] _ in
            if TKeyTapCallback.install() {
                AppState.shared.tapActive = true
                self?.retryTimer?.invalidate()
                self?.retryTimer = nil
            }
        }
    }
}

enum EngineConfig {
    static func applyFromDefaults() {
        let d = UserDefaults.standard
        func flag(_ key: String, _ def: Bool) -> Int32 {
            (d.object(forKey: key) == nil ? def : d.bool(forKey: key)) ? 1 : 0
        }
        tk_enable_vietnamese(flag("enabled", true))
        tk_set_input_method(Int32(d.integer(forKey: "inputMethod")))
        tk_set_output_charset(Int32(d.integer(forKey: "outputCharset")))
        tk_set_option("spell_check", flag("spellCheck", true))
        tk_set_option("auto_restore", flag("autoRestore", true))
        tk_set_option("auto_upper", flag("autoUpper", false))
        tk_set_option("free_mark", flag("freeMark", true))
        tk_set_option("new_mark", flag("newMark", true))
        tk_set_option("enable_macro", flag("enableMacro", false))
        _ = tkResetBuff(0)
        TKeyTapCallback.updateVietnameseMode()
    }
}

enum TKeyTapCallback {
    static var engineSource: CGEventSource?
    static var sourceStateID: Int64 = -1
    static var frontBundleID = ""
    static var isTKeyFrontmost = false

    static let autocompleteApps = [
        "com.google.Chrome", "com.microsoft.edgemac", "com.brave.Browser",
        "com.operasoftware.Opera", "com.vivaldi.Vivaldi", "org.chromium.Chromium",
        "company.thebrowser.Browser",
        "com.coccoc.",
        "com.apple.Safari",
        "org.mozilla.firefox", "app.zen-browser.zen",
        "com.kagi.kagimacOS", "com.duckduckgo.macos.browser",
    ]
    static var needsAutocompleteGuard: Bool {
        let b = frontBundleID
        return autocompleteApps.contains { b.hasPrefix($0) }
    }
    private static let throwawayChar: UniChar = 0x202F

    static func updateVietnameseMode() {
        let d = UserDefaults.standard
        let on = d.object(forKey: "enabled") == nil ? true : d.bool(forKey: "enabled")
        let excluded = !isTKeyFrontmost && ExclusionList.shared.isExcluded(frontBundleID)
        if AppState.shared.excludedNow != excluded {
            AppState.shared.excludedNow = excluded
        }
        tk_enable_vietnamese((on && !excluded) ? 1 : 0)
        _ = tkResetBuff(0)
    }

    @_silgen_name("CGEventSourceGetSourceStateID")
    static func getSourceStateID(_ source: CGEventSource) -> Int64
    private static var tap: CFMachPort?
    private static var runLoopSource: CFRunLoopSource?

    static func install() -> Bool {
        if tap != nil { return true }
        let mask = CGEventMask(1 << CGEventType.keyDown.rawValue)
                 | CGEventMask(1 << CGEventType.keyUp.rawValue)
                 | CGEventMask(1 << CGEventType.flagsChanged.rawValue)
        guard let t = CGEvent.tapCreate(
            tap: .cgSessionEventTap, place: .headInsertEventTap,
            options: .defaultTap, eventsOfInterest: mask,
            callback: { proxy, type, event, refcon in
                TKeyTapCallback.process(proxy, type, event, refcon)
            }, userInfo: nil
        ) else { return false }
        tap = t
        let src = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, t, 0)
        runLoopSource = src
        CFRunLoopAddSource(CFRunLoopGetMain(), src, .commonModes)
        CGEvent.tapEnable(tap: t, enable: true)
        return true
    }

    static func uninstall() {
        if let t = tap { CGEvent.tapEnable(tap: t, enable: false) }
        if let s = runLoopSource { CFRunLoopRemoveSource(CFRunLoopGetMain(), s, .commonModes) }
        tap = nil; runLoopSource = nil
    }

    private static func pass(_ event: CGEvent) -> Unmanaged<CGEvent>? {
        Unmanaged.passUnretained(event)
    }

    static func process(_ proxy: CGEventTapProxy, _ type: CGEventType,
                        _ event: CGEvent, _ refcon: UnsafeMutableRawPointer?) -> Unmanaged<CGEvent>? {
        if type == .tapDisabledByTimeout || type == .tapDisabledByUserInput {
            if let t = tap { CGEvent.tapEnable(tap: t, enable: true) }
            _ = tkResetBuff(0)
            return pass(event)
        }

        let srcID = event.getIntegerValueField(.eventSourceStateID)
        if srcID == sourceStateID {
            return pass(event)
        }

        if type == .flagsChanged {
            HotkeyMonitor.flagsChanged(event.flags)
            return pass(event)
        }

        guard type == .keyDown else { return pass(event) }
        HotkeyMonitor.keyPressed()

        let blocking: CGEventFlags = [.maskCommand, .maskControl, .maskAlternate, .maskSecondaryFn]
        if !event.flags.intersection(blocking).isEmpty {
            _ = tkResetBuff(0)
            return pass(event)
        }

        let keyCode = event.getIntegerValueField(.keyboardEventKeycode)

        if keyCode == 36 || keyCode == 76 {
            _ = tkProcessChar(13)
            if pushOutput(proxy) {
                if let src = engineSource,
                   let ev = CGEvent(keyboardEventSource: src, virtualKey: 36, keyDown: true) {
                    ev.flags = event.flags
                    ev.tapPostEvent(proxy)
                }
                return nil
            }
            return pass(event)
        }

        if keyCode == 51 {
            _ = tkPutBackspace()
            return pushOutput(proxy) ? nil : pass(event)
        }

        var length = 0
        var chars = [UniChar](repeating: 0, count: 4)
        event.keyboardGetUnicodeString(maxStringLength: chars.count,
                                       actualStringLength: &length,
                                       unicodeString: &chars)
        guard length == 1, chars[0] >= 0x20, chars[0] < 0x7F else {
            _ = tkResetBuff(0)
            return pass(event)
        }

        _ = tkProcessChar(UInt32(chars[0]))
        return pushOutput(proxy) ? nil : pass(event)
    }

    private static func postText(_ units: [UniChar], _ src: CGEventSource, _ proxy: CGEventTapProxy) {
        for chunk in TKeyOutput.chunks(units) {
            guard let ev = CGEvent(keyboardEventSource: src, virtualKey: 0, keyDown: true) else { continue }
            chunk.withUnsafeBufferPointer {
                ev.keyboardSetUnicodeString(stringLength: chunk.count, unicodeString: $0.baseAddress)
            }
            ev.tapPostEvent(proxy)
        }
    }

    @discardableResult
    private static func pushOutput(_ proxy: CGEventTapProxy) -> Bool {
        let backspaces = Int(tk_get_backspace_len())
        var buf = [UInt8](repeating: 0, count: 4104)
        var len: Int32 = 0
        if tk_get_buff_len() > 0 { tk_get_buff(&buf, &len) }
        guard backspaces > 0 || len > 0, let src = engineSource else { return false }

        var backspaceEvents = backspaces
        if backspaceEvents > 0, needsAutocompleteGuard {
            postText([throwawayChar], src, proxy)
            backspaceEvents += 1
        }
        for _ in 0..<backspaceEvents {
            for down in [true, false] {
                if let ev = CGEvent(keyboardEventSource: src, virtualKey: 0x33, keyDown: down) {
                    ev.tapPostEvent(proxy)
                }
            }
        }

        let units = TKeyOutput.decode(buf, length: Int(len),
                                      outType: tk_get_out_type(),
                                      charset: UserDefaults.standard.integer(forKey: "outputCharset"))
        postText(units, src, proxy)
        return true
    }
}
