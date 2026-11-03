import SwiftUI
import AppKit
import UniformTypeIdentifiers

struct MacroItem: Identifiable, Hashable {
    var id: String { key }
    var key: String
    var value: String
}

struct MacroPage: View {
    @EnvironmentObject private var settings: SettingsStore
    @State private var macros: [MacroItem] = []
    @State private var search = ""
    @State private var draftKey = ""
    @State private var draftValue = ""
    @State private var editingKey: String?
    @State private var problem: String?
    @FocusState private var keyFocused: Bool

    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            VStack(alignment: .leading, spacing: 16) {
                pageHeader(
                    title: "Bảng Gõ tắt (Macro)",
                    subtitle: "Tự động mở rộng từ viết tắt thành cụm từ dài khi nhấn dấu cách hoặc Enter."
                )

                GlassSection {
                    GlassToggleRow(
                        icon: "text.badge.plus",
                        iconColor: .purple,
                        title: "Bật tính năng Gõ tắt",
                        subtitle: settings.enableMacro
                            ? "Đang bật — gõ từ khóa rồi nhấn Space hoặc Return để thay thế."
                            : "Đang tắt — các từ khóa bên dưới sẽ được giữ nguyên khi gõ.",
                        isOn: $settings.enableMacro,
                        showDivider: true
                    )

                    HStack(spacing: 10) {
                        Image(systemName: "magnifyingglass")
                            .foregroundStyle(.secondary)
                        TextField("Tìm từ khóa hoặc nội dung…", text: $search)
                            .textFieldStyle(.plain)
                            .font(.system(size: 13))
                        if !search.isEmpty {
                            Button {
                                search = ""
                            } label: {
                                Image(systemName: "xmark.circle.fill")
                                    .foregroundStyle(.secondary)
                            }
                            .buttonStyle(.plain)
                        }
                        Spacer(minLength: 8)
                        Text("\(macros.count) mục")
                            .font(.system(size: 12, weight: .medium))
                            .foregroundStyle(.secondary)
                        Button("Nạp file…") { importMacros() }
                            .buttonStyle(.glass)
                            .controlSize(.small)
                        Button("Xuất file…") { exportMacros() }
                            .buttonStyle(.glass)
                            .controlSize(.small)
                            .disabled(macros.isEmpty)
                    }
                    .padding(.horizontal, 16)
                    .padding(.vertical, 10)
                }
            }
            .padding(.horizontal, 24)
            .padding(.top, 20)

            ScrollView {
                VStack(spacing: 8) {
                    if filtered.isEmpty {
                        emptyStateView
                    } else {
                        ForEach(filtered) { item in
                            macroRow(item)
                        }
                    }
                }
                .padding(.horizontal, 24)
                .padding(.vertical, 4)
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)

            composerBar
                .padding(.horizontal, 24)
                .padding(.bottom, 20)
        }
        .onAppear(perform: reload)
    }

    private var filtered: [MacroItem] {
        let q = search.trimmingCharacters(in: .whitespaces)
        guard !q.isEmpty else { return macros }
        return macros.filter {
            $0.key.localizedCaseInsensitiveContains(q) || $0.value.localizedCaseInsensitiveContains(q)
        }
    }

    private func macroRow(_ item: MacroItem) -> some View {
        let isEditing = (editingKey == item.key)
        return HStack(spacing: 12) {
            Text(item.key)
                .font(.system(size: 13, weight: .bold, design: .monospaced))
                .foregroundStyle(Color.accentColor)
                .padding(.horizontal, 10)
                .padding(.vertical, 5)
                .background(
                    Capsule(style: .continuous)
                        .fill(Color.accentColor.opacity(0.14))
                )

            Image(systemName: "arrow.right")
                .font(.system(size: 11, weight: .semibold))
                .foregroundStyle(.tertiary)

            Text(item.value)
                .font(.system(size: 13.5))
                .foregroundStyle(.primary)
                .frame(maxWidth: .infinity, alignment: .leading)
                .lineLimit(2)

            HStack(spacing: 6) {
                Button {
                    beginEditing(item)
                } label: {
                    Label("Sửa", systemImage: "pencil")
                        .font(.system(size: 12))
                }
                .buttonStyle(.glass)
                .controlSize(.small)

                Button(role: .destructive) {
                    removeKey(item.key)
                } label: {
                    Image(systemName: "trash")
                        .font(.system(size: 12))
                        .foregroundStyle(.red)
                }
                .buttonStyle(.glass)
                .controlSize(.small)
                .help("Xóa “\(item.key)”")
            }
        }
        .padding(.horizontal, 14)
        .padding(.vertical, 10)
        .background(
            RoundedRectangle(cornerRadius: 12, style: .continuous)
                .fill(.ultraThinMaterial)
        )
        .overlay(
            RoundedRectangle(cornerRadius: 12, style: .continuous)
                .strokeBorder(isEditing ? Color.accentColor : Color.primary.opacity(0.08),
                              lineWidth: isEditing ? 1.5 : 1)
        )
    }

    private var emptyStateView: some View {
        VStack(spacing: 8) {
            Image(systemName: search.isEmpty ? "text.badge.plus" : "magnifyingglass")
                .font(.system(size: 28))
                .foregroundStyle(.secondary)
            Text(search.isEmpty ? "Chưa có từ gõ tắt nào" : "Không tìm thấy kết quả phù hợp")
                .font(.system(size: 14, weight: .semibold))
            Text(search.isEmpty
                 ? "Nhập từ khóa (ví dụ: “vn”) và cụm từ đầy đủ (“Việt Nam”) ở khung bên dưới rồi bấm Thêm."
                 : "Thử tìm với từ khóa khác.")
                .font(.system(size: 12))
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 36)
        .glassEffect(.regular, in: .rect(cornerRadius: 16))
    }

    private var composerBar: some View {
        VStack(alignment: .leading, spacing: 8) {
            if let problem {
                Label(problem, systemImage: "exclamationmark.triangle.fill")
                    .font(.system(size: 12, weight: .medium))
                    .foregroundStyle(.orange)
            }

            HStack(spacing: 10) {
                TextField("Từ khóa (vd: vn)", text: $draftKey)
                    .textFieldStyle(.plain)
                    .font(.system(size: 13, weight: .semibold, design: .monospaced))
                    .padding(.horizontal, 10)
                    .padding(.vertical, 7)
                    .frame(width: 140)
                    .background(
                        RoundedRectangle(cornerRadius: 8, style: .continuous)
                            .fill(Color(nsColor: .textBackgroundColor).opacity(0.9))
                    )
                    .overlay(
                        RoundedRectangle(cornerRadius: 8, style: .continuous)
                            .strokeBorder(Color.primary.opacity(0.12))
                    )
                    .focused($keyFocused)
                    .onSubmit(save)

                Image(systemName: "arrow.right")
                    .font(.system(size: 11, weight: .bold))
                    .foregroundStyle(.secondary)

                TextField("Cụm từ đầy đủ (vd: Việt Nam)", text: $draftValue)
                    .textFieldStyle(.plain)
                    .font(.system(size: 13))
                    .padding(.horizontal, 10)
                    .padding(.vertical, 7)
                    .background(
                        RoundedRectangle(cornerRadius: 8, style: .continuous)
                            .fill(Color(nsColor: .textBackgroundColor).opacity(0.9))
                    )
                    .overlay(
                        RoundedRectangle(cornerRadius: 8, style: .continuous)
                            .strokeBorder(Color.primary.opacity(0.12))
                    )
                    .onSubmit(save)

                if editingKey != nil {
                    Button("Hủy", action: cancelEditing)
                        .buttonStyle(.glass)
                }

                Button(editingKey == nil ? "Thêm mới" : "Lưu thay đổi", action: save)
                    .buttonStyle(.glassProminent)
                    .disabled(draftKey.trimmingCharacters(in: .whitespaces).isEmpty || draftValue.isEmpty)
            }
        }
        .padding(14)
        .glassEffect(.regular, in: .rect(cornerRadius: 16))
        .onChange(of: draftKey) { problem = nil }
    }

    private func reload() {
        var items: [MacroItem] = []
        let count = tk_macro_count()
        for i in 0..<count {
            var key = [CChar](repeating: 0, count: 256)
            var value = [CChar](repeating: 0, count: 4096)
            tk_macro_get(i, &key, 256, &value, 4096)
            let k = String(cString: key)
            let v = String(cString: value)
            if !k.isEmpty {
                items.append(MacroItem(key: k, value: v))
            }
        }
        macros = items.sorted { $0.key.localizedStandardCompare($1.key) == .orderedAscending }
        MacroPersistence.save(macros)
    }

    private func beginEditing(_ item: MacroItem) {
        editingKey = item.key
        draftKey = item.key
        draftValue = item.value
        problem = nil
        keyFocused = true
    }

    private func cancelEditing() {
        editingKey = nil
        draftKey = ""
        draftValue = ""
        problem = nil
    }

    private func save() {
        let key = draftKey.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !key.isEmpty, !draftValue.isEmpty else { return }
        if key.contains(where: { $0.isWhitespace }) {
            problem = "Từ khóa không được chứa khoảng trắng."
            return
        }
        let clash = macros.contains {
            $0.key.caseInsensitiveCompare(key) == .orderedSame && $0.key != editingKey
        }
        if clash {
            problem = "Từ khóa “\(key)” đã tồn tại. Hãy bấm Sửa ở dòng tương ứng."
            return
        }
        if let old = editingKey {
            _ = tk_macro_remove(old)
        }
        _ = tk_macro_add(key, draftValue)
        cancelEditing()
        reload()
    }

    private func removeKey(_ key: String) {
        _ = tk_macro_remove(key)
        if editingKey == key { cancelEditing() }
        reload()
    }

    private func importMacros() {
        let panel = NSOpenPanel()
        panel.title = "Nạp danh sách gõ tắt"
        panel.allowedContentTypes = [.plainText, .data]
        panel.allowsMultipleSelection = false
        guard panel.runModal() == .OK, let url = panel.url else { return }
        _ = tk_load_macro(url.path)
        reload()
    }

    private func exportMacros() {
        let panel = NSSavePanel()
        panel.title = "Xuất danh sách gõ tắt"
        panel.nameFieldStringValue = "TKey-Macros.txt"
        panel.allowedContentTypes = [.plainText]
        guard panel.runModal() == .OK, let url = panel.url else { return }
        _ = tk_save_macro(url.path)
    }
}

enum MacroPersistence {
    private static let key = "macros"
    private static let seededKey = "macrosSeeded"

    static func save(_ items: [MacroItem]) {
        UserDefaults.standard.set(items.map { [$0.key, $0.value] }, forKey: key)
    }

    static func pushToEngine() {
        let d = UserDefaults.standard
        if let stored = d.array(forKey: key) as? [[String]], !stored.isEmpty {
            for pair in stored where pair.count == 2 {
                _ = tk_macro_add(pair[0], pair[1])
            }
            return
        }
        if !d.bool(forKey: seededKey) {
            d.set(true, forKey: seededKey)
            let defaults = [
                ["vn", "Việt Nam"],
                ["qt", "quyết tâm"],
                ["tphcm", "Thành phố Hồ Chí Minh"],
            ]
            d.set(defaults, forKey: key)
            for pair in defaults {
                _ = tk_macro_add(pair[0], pair[1])
            }
        }
    }
}
