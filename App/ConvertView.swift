import SwiftUI
import AppKit

struct ConvertPage: View {
    @AppStorage("convertFrom") private var from = 20
    @AppStorage("convertTo") private var to = 0
    @State private var input = ""
    @State private var output = ""
    @State private var failed = false
    @State private var statusBanner: String?

    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            pageHeader(
                title: "Công cụ Chuyển mã Tiếng Việt",
                subtitle: "Chuyển đổi văn bản trực tiếp giữa Unicode, TCVN3 (ABC), VNI Windows, CP1258 và VIQR."
            )

            HStack(spacing: 14) {
                editorCard(
                    title: "Văn bản gốc",
                    charset: $from,
                    isSource: true
                ) {
                    TextEditor(text: $input)
                        .font(.system(size: 13.5))
                        .scrollContentBackground(.hidden)
                        .padding(8)
                        .overlay(alignment: .topLeading) {
                            if input.isEmpty {
                                Text("Dán hoặc gõ văn bản cần chuyển mã vào đây…")
                                    .font(.system(size: 13.5))
                                    .foregroundStyle(.tertiary)
                                    .padding(.horizontal, 13)
                                    .padding(.vertical, 10)
                                    .allowsHitTesting(false)
                            }
                        }
                } footer: {
                    Button("Dán từ Clipboard", action: pasteFromClipboard)
                        .buttonStyle(.glass)
                        .controlSize(.small)
                    if !input.isEmpty {
                        Button("Xóa trắng") {
                            input = ""
                            statusBanner = nil
                        }
                        .buttonStyle(.glass)
                        .controlSize(.small)
                    }
                    Spacer()
                    Button(action: swap) {
                        Label("Đảo chiều", systemImage: "arrow.left.arrow.right")
                    }
                    .buttonStyle(.glass)
                    .controlSize(.small)
                }

                editorCard(
                    title: "Kết quả",
                    charset: $to,
                    isSource: false
                ) {
                    ScrollView {
                        Text(output.isEmpty ? "Kết quả chuyển mã sẽ hiển thị tức thì tại đây…" : output)
                            .font(.system(size: 13.5))
                            .foregroundStyle(output.isEmpty ? .tertiary : .primary)
                            .textSelection(.enabled)
                            .frame(maxWidth: .infinity, alignment: .topLeading)
                            .padding(12)
                    }
                } footer: {
                    Button {
                        quickConvertClipboard()
                    } label: {
                        Label("Đổi Clipboard", systemImage: "bolt.fill")
                    }
                    .buttonStyle(.glass)
                    .controlSize(.small)
                    .fixedSize()

                    Spacer()

                    Button("Sao chép kết quả", action: copyResult)
                        .buttonStyle(.glassProminent)
                        .controlSize(.small)
                        .disabled(output.isEmpty || failed)
                }
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)

            if let banner = statusBanner ?? contextualHint {
                HStack(spacing: 8) {
                    Image(systemName: failed ? "exclamationmark.triangle.fill" : "info.circle.fill")
                        .foregroundStyle(failed ? Color.orange : Color.accentColor)
                    Text(banner)
                        .font(.system(size: 12))
                        .foregroundStyle(failed ? Color.orange : Color.secondary)
                    Spacer()
                }
                .padding(.horizontal, 14)
                .padding(.vertical, 9)
                .glassEffect(.regular, in: .rect(cornerRadius: 12))
            }
        }
        .padding(24)
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
        .onChange(of: input) { convert() }
        .onChange(of: from)  { convert() }
        .onChange(of: to)    { convert() }
        .onAppear(perform: convert)
    }

    private func editorCard<Content: View, Footer: View>(
        title: String,
        charset: Binding<Int>,
        isSource: Bool,
        @ViewBuilder content: () -> Content,
        @ViewBuilder footer: () -> Footer
    ) -> some View {
        VStack(alignment: .leading, spacing: 10) {
            HStack {
                Text(title)
                    .font(.system(size: 13, weight: .semibold))
                    .foregroundStyle(.primary)
                Spacer()
                charsetMenu(selection: charset)
            }
            .padding(.horizontal, 2)

            content()
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                .background(
                    RoundedRectangle(cornerRadius: 12, style: .continuous)
                        .fill(Color(nsColor: .textBackgroundColor).opacity(isSource ? 0.92 : 0.72))
                )
                .overlay(
                    RoundedRectangle(cornerRadius: 12, style: .continuous)
                        .strokeBorder(Color.primary.opacity(0.10))
                )

            HStack(spacing: 8) {
                footer()
            }
        }
        .padding(14)
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .glassEffect(.regular, in: .rect(cornerRadius: 16))
    }

    private func charsetName(_ id: Int) -> String {
        SettingsStore.charsets.first(where: { $0.id == id })?.name ?? "Unicode"
    }

    private func charsetMenu(selection: Binding<Int>) -> some View {
        Menu {
            ForEach(SettingsStore.charsets) { c in
                Button {
                    selection.wrappedValue = c.id
                } label: {
                    if selection.wrappedValue == c.id {
                        Label(c.name, systemImage: "checkmark")
                    } else {
                        Text(c.name)
                    }
                }
            }
        } label: {
            Text(charsetName(selection.wrappedValue))
                .font(.system(size: 12.5, weight: .medium))
        }
        .buttonStyle(.glass)
    }

    private var contextualHint: String? {
        if failed {
            return "Không chuyển được mã — hãy kiểm tra lại bảng mã nguồn."
        }
        if from == 20 || to == 20 {
            return "Lưu ý: Bảng mã TCVN3 (ABC) chuẩn không phân biệt hoa/thường cho chữ có dấu (cần font .VnTimeH để hiển thị chữ hoa)."
        }
        return nil
    }

    private func convert() {
        statusBanner = nil
        guard !input.isEmpty else {
            output = ""
            failed = false
            return
        }
        if let res = CharsetCodec.convert(input, from: from, to: to) {
            output = res
            failed = false
        } else {
            output = ""
            failed = true
        }
    }

    private func swap() {
        let tmp = from
        from = to
        to = tmp
        if !output.isEmpty {
            input = output
        }
    }

    private func pasteFromClipboard() {
        input = NSPasteboard.general.string(forType: .string) ?? ""
    }

    private func copyResult() {
        let pb = NSPasteboard.general
        pb.clearContents()
        pb.setString(output, forType: .string)
        statusBanner = "Đã sao chép kết quả (\(charsetName(to))) vào Clipboard."
    }

    private func quickConvertClipboard() {
        let pb = NSPasteboard.general
        guard let text = pb.string(forType: .string), !text.isEmpty else {
            statusBanner = "Clipboard hiện đang trống."
            return
        }
        input = text
        if let converted = CharsetCodec.convert(text, from: from, to: to) {
            output = converted
            pb.clearContents()
            pb.setString(converted, forType: .string)
            failed = false
            statusBanner = "Đã chuyển nhanh Clipboard từ \(charsetName(from)) sang \(charsetName(to))."
        } else {
            failed = true
        }
    }
}
