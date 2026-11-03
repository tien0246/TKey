import SwiftUI
import ServiceManagement

final class SettingsStore: ObservableObject {
    static let shared = SettingsStore()

    struct Method: Identifiable { let id: Int; let name: String; let summary: String }
    struct Charset: Identifiable { let id: Int; let name: String; let note: String }

    static let commonMethods: [Method] = [
        Method(id: 0, name: "Telex", summary: "Phổ biến nhất — dùng chữ cái s, f, r, x, j, w để bỏ dấu"),
        Method(id: 1, name: "VNI", summary: "Dùng hàng phím số 1–9 để bỏ dấu thanh và dấu mũ"),
        Method(id: 2, name: "VIQR", summary: "Dùng ký hiệu dấu câu ', `, ?, ~, ., ^, +, ( để bỏ dấu"),
    ]
    static let otherMethods: [Method] = [
        Method(id: 4, name: "Telex đơn giản", summary: "Giống Telex nhưng gõ chữ w rời không biến thành ư"),
        Method(id: 5, name: "Telex đơn giản 2", summary: "Biến thể Telex đơn giản cho người gõ song ngữ Anh–Việt"),
        Method(id: 6, name: "Telex + VNI", summary: "Kết hợp cả phím chữ Telex và phím số VNI cùng lúc"),
        Method(id: 3, name: "MS Vietnamese", summary: "Bố cục bàn phím tiếng Việt mặc định của Microsoft"),
    ]
    static var methods: [Method] { commonMethods + otherMethods }

    static let charsets: [Charset] = [
        Charset(id: 0,  name: "Unicode (Dựng sẵn)", note: "Chuẩn quốc tế hiện đại — dùng cho mọi ứng dụng, trình duyệt, văn bản."),
        Charset(id: 4,  name: "Unicode tổ hợp", note: "Tách chữ cái và dấu thanh thành 2 ký tự (NFD)."),
        Charset(id: 20, name: "TCVN3 (ABC)", note: "Bảng mã cũ 1-byte dùng cho các font .VnTime, .VnArial."),
        Charset(id: 40, name: "VNI Windows", note: "Bảng mã cũ 2-byte dùng cho các font VNI-Times, VNI-Helve."),
        Charset(id: 5,  name: "Windows CP1258", note: "Bảng mã tiếng Việt của hệ điều hành Windows cũ."),
        Charset(id: 10, name: "VIQR", note: "Biểu diễn dấu bằng ký tự ASCII thuần (ví dụ: Tie^'ng Vie^.t)."),
    ]

    @AppStorage("enabled") var vietnameseEnabled = true {
        willSet { objectWillChange.send() } didSet { EngineConfig.applyFromDefaults() }
    }
    @AppStorage("inputMethod") var inputMethod = 0 {
        willSet { objectWillChange.send() } didSet { EngineConfig.applyFromDefaults() }
    }
    @AppStorage("outputCharset") var outputCharset = 0 {
        willSet { objectWillChange.send() } didSet { EngineConfig.applyFromDefaults() }
    }
    @AppStorage("spellCheck") var spellCheck = true {
        willSet { objectWillChange.send() } didSet { EngineConfig.applyFromDefaults() }
    }
    @AppStorage("autoRestore") var autoRestore = true {
        willSet { objectWillChange.send() } didSet { EngineConfig.applyFromDefaults() }
    }
    @AppStorage("freeMark") var freeMark = true {
        willSet { objectWillChange.send() } didSet { EngineConfig.applyFromDefaults() }
    }
    @AppStorage("newMark") var newMark = true {
        willSet { objectWillChange.send() } didSet { EngineConfig.applyFromDefaults() }
    }
    @AppStorage("autoUpper") var autoUpper = false {
        willSet { objectWillChange.send() } didSet { EngineConfig.applyFromDefaults() }
    }
    @AppStorage("enableMacro") var enableMacro = false {
        willSet { objectWillChange.send() } didSet { EngineConfig.applyFromDefaults() }
    }
    @AppStorage("toggleHotkey") var toggleHotkey = ToggleHotkey.controlShift.rawValue {
        willSet { objectWillChange.send() }
    }

    var inputMethodName: String {
        Self.methods.first(where: { $0.id == inputMethod })?.name ?? "Telex"
    }

    var inputMethodSummary: String {
        Self.methods.first(where: { $0.id == inputMethod })?.summary ?? ""
    }

    var charsetName: String {
        Self.charsets.first(where: { $0.id == outputCharset })?.name ?? "Unicode (Dựng sẵn)"
    }

    var charsetNote: String {
        Self.charsets.first(where: { $0.id == outputCharset })?.note ?? ""
    }

    var hotkey: ToggleHotkey {
        ToggleHotkey(rawValue: toggleHotkey) ?? .controlShift
    }

    func setLaunchAtLogin(_ on: Bool) throws {
        objectWillChange.send()
        if on {
            try SMAppService.mainApp.register()
        } else {
            try SMAppService.mainApp.unregister()
        }
    }

    func resetToDefaults() {
        vietnameseEnabled = true
        inputMethod = 0
        outputCharset = 0
        spellCheck = true
        autoRestore = true
        freeMark = true
        newMark = true
        autoUpper = false
        enableMacro = false
        toggleHotkey = ToggleHotkey.controlShift.rawValue
        EngineConfig.applyFromDefaults()
    }
}
