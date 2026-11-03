import SwiftUI

struct MenuView: View {
    @EnvironmentObject private var settings: SettingsStore
    @EnvironmentObject private var state: AppState

    var body: some View {
        if !state.tapActive {
            Button("⚠︎ Cần cấp quyền Trợ năng để gõ…") {
                MainWindowController.shared.show(.general)
            }
            Divider()
        }

        if state.excludedNow {
            Button("Đang tạm tắt cho \(state.activeAppName.isEmpty ? "ứng dụng này" : state.activeAppName)…") {
                MainWindowController.shared.show(.exclusion)
            }
            Divider()
        }

        Toggle(toggleTitle, isOn: $settings.vietnameseEnabled)
            .disabled(state.excludedNow)

        Picker("Kiểu gõ: \(settings.inputMethodName)", selection: $settings.inputMethod) {
            ForEach(SettingsStore.commonMethods) { m in Text(m.name).tag(m.id) }
            Divider()
            ForEach(SettingsStore.otherMethods) { m in Text(m.name).tag(m.id) }
        }

        Picker("Bảng mã: \(settings.charsetName)", selection: $settings.outputCharset) {
            ForEach(SettingsStore.charsets) { c in Text(c.name).tag(c.id) }
        }

        Divider()

        Toggle("Kiểm tra chính tả", isOn: $settings.spellCheck)
        Toggle("Khôi phục từ tiếng Anh", isOn: $settings.autoRestore)
        Toggle("Bỏ dấu tự do", isOn: $settings.freeMark)
        Toggle("Gõ tắt", isOn: $settings.enableMacro)

        Divider()

        Button("Cấu hình Gõ chữ…") {
            MainWindowController.shared.show(.typing)
        }
        .keyboardShortcut(",")

        Button("Bảng Gõ tắt…") {
            MainWindowController.shared.show(.macros)
        }

        Button("Chuyển mã Tiếng Việt…") {
            MainWindowController.shared.show(.convert)
        }

        Button("Loại trừ ứng dụng…") {
            MainWindowController.shared.show(.exclusion)
        }

        Divider()

        Button("Giới thiệu TKey") {
            AboutTKey.show()
        }

        Button("Thoát TKey") {
            NSApp.terminate(nil)
        }
        .keyboardShortcut("q")
    }

    private var toggleTitle: String {
        if state.excludedNow {
            return "Gõ Tiếng Việt (Đã loại trừ app)"
        }
        if let keys = settings.hotkey.symbols {
            return "Gõ Tiếng Việt  (\(keys))"
        }
        return "Gõ Tiếng Việt"
    }
}
