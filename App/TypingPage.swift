import SwiftUI

struct TypingPage: View {
    @EnvironmentObject private var settings: SettingsStore
    @State private var sample = ""
    @FocusState private var sampleFocused: Bool

    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 20) {
                pageHeader(
                    title: "Cấu hình Gõ chữ",
                    subtitle: "Chọn phương pháp gõ, phím tắt chuyển nhanh và các quy tắc bỏ dấu tiếng Việt."
                )

                GlassSection(title: "Chế độ & Kiểu gõ", icon: "keyboard.fill") {
                    GlassToggleRow(
                        icon: "character.cursor.ibeam",
                        iconColor: .green,
                        title: "Bật gõ Tiếng Việt",
                        subtitle: hotkeyHint,
                        isOn: $settings.vietnameseEnabled,
                        showDivider: true
                    )

                    GlassControlRow(
                        icon: "command",
                        iconColor: .blue,
                        title: "Phím tắt chuyển Việt / Anh",
                        subtitle: "Nhấn tổ hợp phím rồi thả ra ngay để đổi nhanh chế độ gõ.",
                        showDivider: true
                    ) {
                        Picker("", selection: $settings.toggleHotkey) {
                            ForEach(ToggleHotkey.allCases) { k in
                                Text(k.title).tag(k.rawValue)
                            }
                        }
                        .labelsHidden()
                        .fixedSize()
                    }

                    GlassControlRow(
                        icon: "textformat.abc",
                        iconColor: .indigo,
                        title: "Kiểu gõ dấu",
                        subtitle: settings.inputMethodSummary,
                        showDivider: false
                    ) {
                        Picker("", selection: $settings.inputMethod) {
                            ForEach(SettingsStore.commonMethods) { m in Text(m.name).tag(m.id) }
                            Divider()
                            ForEach(SettingsStore.otherMethods) { m in Text(m.name).tag(m.id) }
                        }
                        .labelsHidden()
                        .fixedSize()
                    }
                }

                GlassSection(
                    title: "Bảng quy tắc \(settings.inputMethodName) & Gõ thử",
                    subtitle: "Bấm vào từng phím mẫu để chèn thử hoặc gõ trực tiếp bên dưới",
                    icon: "sparkles.rectangle.stack.fill"
                ) {
                    VStack(alignment: .leading, spacing: 14) {
                        InteractiveKeyGuideView(
                            guide: KeyGuide.forMethod(settings.inputMethod),
                            onSelectHint: { hint in
                                if !sample.isEmpty && !sample.hasSuffix(" ") {
                                    sample += " "
                                }
                                sample += hint.result
                                sampleFocused = true
                            }
                        )

                        HStack(spacing: 10) {
                            Image(systemName: "pencil.and.outline")
                                .foregroundStyle(.secondary)
                            TextField(
                                "Gõ thử",
                                text: $sample,
                                prompt: Text("Gõ thử tại đây — ví dụ: \(KeyGuide.forMethod(settings.inputMethod).example.typed) → \(KeyGuide.forMethod(settings.inputMethod).example.result)")
                            )
                            .textFieldStyle(.plain)
                            .font(.system(size: 14))
                            .focused($sampleFocused)

                            if !sample.isEmpty {
                                Button("Xóa") {
                                    sample = ""
                                    sampleFocused = true
                                }
                                .buttonStyle(.glass)
                                .controlSize(.small)
                            }
                        }
                        .padding(.horizontal, 12)
                        .padding(.vertical, 9)
                        .background(
                            RoundedRectangle(cornerRadius: 10, style: .continuous)
                                .fill(Color(nsColor: .textBackgroundColor).opacity(0.85))
                        )
                        .overlay(
                            RoundedRectangle(cornerRadius: 10, style: .continuous)
                                .strokeBorder(Color.primary.opacity(0.12))
                        )
                    }
                    .padding(16)
                }

                GlassSection(title: "Tùy chọn chính tả & bỏ dấu", icon: "wand.and.stars") {
                    GlassToggleRow(
                        icon: "checkmark.seal.fill",
                        iconColor: .teal,
                        title: "Kiểm tra chính tả tiếng Việt",
                        subtitle: "Chỉ ghép dấu khi từ có cấu trúc nguyên âm – phụ âm hợp lệ trong tiếng Việt.",
                        isOn: $settings.spellCheck
                    )

                    GlassToggleRow(
                        icon: "arrow.uturn.backward.circle.fill",
                        iconColor: .cyan,
                        title: "Tự khôi phục từ tiếng Anh",
                        subtitle: "Khi gõ các từ như “windows”, “user”, “class” rồi nhấn dấu cách, TKey tự trả lại nguyên gốc.",
                        isOn: $settings.autoRestore
                    )

                    GlassToggleRow(
                        icon: "hand.point.up.braille.fill",
                        iconColor: .purple,
                        title: "Bỏ dấu tự do (Free mark)",
                        subtitle: "Cho phép gõ dấu thanh ở cuối từ thay vì ngay sau nguyên âm (ví dụ: “vietje” → “việt”).",
                        isOn: $settings.freeMark
                    )

                    GlassToggleRow(
                        icon: "a.magnify",
                        iconColor: .pink,
                        title: "Đặt dấu kiểu mới (oà, uý)",
                        subtitle: "Đặt dấu thanh lên nguyên âm chính theo quy chuẩn mới (oà, uý) thay vì kiểu cũ (òa, úy).",
                        isOn: $settings.newMark
                    )

                    GlassToggleRow(
                        icon: "textformat.size.larger",
                        iconColor: .orange,
                        title: "Tự động viết hoa đầu câu",
                        subtitle: "Tự viết hoa chữ cái đầu tiên sau dấu chấm kết thúc câu.",
                        isOn: $settings.autoUpper,
                        showDivider: false
                    )
                }

                GlassSection(title: "Bảng mã xuất văn bản", icon: "doc.text.magnifyingglass") {
                    GlassControlRow(
                        icon: settings.outputCharset == 0 ? "globe" : "exclamationmark.triangle.fill",
                        iconColor: settings.outputCharset == 0 ? .blue : .orange,
                        title: "Bảng mã khi gõ",
                        subtitle: settings.charsetNote,
                        showDivider: false
                    ) {
                        Picker("", selection: $settings.outputCharset) {
                            ForEach(SettingsStore.charsets) { c in
                                Text(c.name).tag(c.id)
                            }
                        }
                        .labelsHidden()
                        .fixedSize()
                    }
                }
            }
            .padding(24)
            .frame(maxWidth: .infinity, alignment: .leading)
        }
    }

    private var hotkeyHint: String {
        guard let keys = settings.hotkey.symbols else {
            return "Bật hoặc tắt từ biểu tượng TKey trên thanh menu."
        }
        return "Nhấn rồi thả \(keys) để chuyển nhanh giữa Tiếng Việt và Tiếng Anh."
    }
}

struct KeyHint: Identifiable {
    let id = UUID()
    let keys: [String]
    let result: String
    let caption: String?
}

struct KeyGuide {
    let tones: [KeyHint]
    let letters: [KeyHint]
    let example: (typed: String, result: String)
    let note: String?

    static func forMethod(_ id: Int) -> KeyGuide {
        switch id {
        case 1: return vni
        case 2: return viqr
        case 3: return msvi
        case 4, 5: return simpleTelex
        case 6: return mixed
        default: return telex
        }
    }

    private static func h(_ keys: String, _ result: String, _ caption: String? = nil) -> KeyHint {
        KeyHint(keys: keys.map { String($0) }, result: result, caption: caption)
    }

    static let telex = KeyGuide(
        tones: [h("as", "á", "sắc"), h("af", "à", "huyền"), h("ar", "ả", "hỏi"),
                h("ax", "ã", "ngã"), h("aj", "ạ", "nặng"), h("asz", "a", "xóa dấu")],
        letters: [h("aa", "â"), h("aw", "ă"), h("ee", "ê"), h("oo", "ô"),
                  h("ow", "ơ"), h("uw", "ư"), h("w", "ư"), h("dd", "đ")],
        example: ("Tieengs Vieejt", "Tiếng Việt"),
        note: nil)

    static let simpleTelex = KeyGuide(
        tones: telex.tones,
        letters: [h("aa", "â"), h("aw", "ă"), h("ee", "ê"), h("oo", "ô"),
                  h("ow", "ơ"), h("uw", "ư"), h("dd", "đ")],
        example: ("Tieengs Vieejt", "Tiếng Việt"),
        note: "Giống Telex, nhưng phím w gõ rời không tự biến thành chữ ư.")

    static let vni = KeyGuide(
        tones: [h("a1", "á", "sắc"), h("a2", "à", "huyền"), h("a3", "ả", "hỏi"),
                h("a4", "ã", "ngã"), h("a5", "ạ", "nặng"), h("a10", "a", "xóa dấu")],
        letters: [h("a6", "â"), h("a8", "ă"), h("e6", "ê"), h("o6", "ô"),
                  h("o7", "ơ"), h("u7", "ư"), h("d9", "đ")],
        example: ("Tie61ng Vie65t", "Tiếng Việt"),
        note: nil)

    static let viqr = KeyGuide(
        tones: [h("a'", "á", "sắc"), h("a`", "à", "huyền"), h("a?", "ả", "hỏi"),
                h("a~", "ã", "ngã"), h("a.", "ạ", "nặng")],
        letters: [h("a^", "â"), h("a(", "ă"), h("e^", "ê"), h("o^", "ô"),
                  h("o+", "ơ"), h("u+", "ư"), h("dd", "đ")],
        example: ("Tie^'ng Vie^.t", "Tiếng Việt"),
        note: nil)

    static let mixed = KeyGuide(
        tones: [h("as", "á", "sắc"), h("a1", "á", "sắc"), h("af", "à", "huyền"), h("a2", "à", "huyền")],
        letters: [h("aw", "ă"), h("a8", "ă"), h("dd", "đ"), h("d9", "đ")],
        example: ("Tieengs Vie65t", "Tiếng Việt"),
        note: "Cho phép dùng đồng thời cả phím chữ Telex lẫn phím số VNI.")

    static let msvi = KeyGuide(
        tones: [], letters: [],
        example: ("", ""),
        note: "Bố cục bàn phím tiếng Việt của Microsoft: dùng hàng phím số để gõ nguyên âm có mũ và dấu thanh.")
}

struct InteractiveKeyGuideView: View {
    let guide: KeyGuide
    var onSelectHint: (KeyHint) -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            if !guide.tones.isEmpty {
                FlowLayout(spacing: 10, lineSpacing: 10) {
                    ForEach(guide.tones) { hint in
                        InteractiveHintChip(hint: hint) { onSelectHint(hint) }
                    }
                }
            }
            if !guide.letters.isEmpty {
                FlowLayout(spacing: 10, lineSpacing: 10) {
                    ForEach(guide.letters) { hint in
                        InteractiveHintChip(hint: hint) { onSelectHint(hint) }
                    }
                }
            }
            if let note = guide.note {
                Text(note)
                    .font(.system(size: 12))
                    .foregroundStyle(.secondary)
            }
        }
    }
}

private struct InteractiveHintChip: View {
    let hint: KeyHint
    let action: () -> Void

    var body: some View {
        Button(action: action) {
            HStack(spacing: 6) {
                Text(hint.keys.joined())
                    .font(.system(size: 12.5, weight: .bold, design: .monospaced))
                    .foregroundStyle(.primary)
                    .fixedSize()
                    .padding(.horizontal, 7)
                    .padding(.vertical, 3)
                    .background(
                        RoundedRectangle(cornerRadius: 6, style: .continuous)
                            .fill(Color.primary.opacity(0.09))
                    )

                Image(systemName: "arrow.right")
                    .font(.system(size: 9, weight: .bold))
                    .foregroundStyle(.secondary)

                Text(hint.result)
                    .font(.system(size: 15, weight: .bold))
                    .foregroundStyle(Color.accentColor)
                    .fixedSize()

                if let caption = hint.caption {
                    Text(caption)
                        .font(.system(size: 11.5))
                        .foregroundStyle(.secondary)
                        .fixedSize()
                }
            }
            .fixedSize()
            .padding(.horizontal, 11)
            .padding(.vertical, 6)
            .contentShape(Capsule())
        }
        .buttonStyle(.plain)
        .glassEffect(.regular.interactive(), in: .capsule)
        .help("Bấm để chèn “\(hint.result)” — gõ phím: \(hint.keys.joined())")
    }
}

struct FlowLayout: Layout {
    var spacing: CGFloat = 10
    var lineSpacing: CGFloat = 10

    func sizeThatFits(proposal: ProposedViewSize, subviews: Subviews, cache: inout ()) -> CGSize {
        let sizes = subviews.map {
            let s = $0.sizeThatFits(.unspecified)
            return CGSize(width: ceil(s.width) + 2, height: ceil(s.height))
        }
        let maxWidth = proposal.width ?? .infinity
        let result = FlowArrange.arrange(sizes: sizes, maxWidth: maxWidth,
                                         spacing: spacing, lineSpacing: lineSpacing)
        let width = (proposal.width.map { $0.isFinite } ?? false) ? proposal.width! : result.size.width
        return CGSize(width: width, height: result.size.height)
    }

    func placeSubviews(in bounds: CGRect, proposal: ProposedViewSize, subviews: Subviews, cache: inout ()) {
        let sizes = subviews.map {
            let s = $0.sizeThatFits(.unspecified)
            return CGSize(width: ceil(s.width) + 2, height: ceil(s.height))
        }
        let result = FlowArrange.arrange(sizes: sizes, maxWidth: bounds.width,
                                         spacing: spacing, lineSpacing: lineSpacing)
        for (i, subview) in subviews.enumerated() {
            let o = result.origins[i]
            subview.place(at: CGPoint(x: bounds.minX + o.x, y: bounds.minY + o.y),
                          anchor: .topLeading, proposal: ProposedViewSize(sizes[i]))
        }
    }
}
