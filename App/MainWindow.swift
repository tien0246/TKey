import SwiftUI
import ServiceManagement
import ApplicationServices

struct MainWindow: View {
    @EnvironmentObject private var settings: SettingsStore
    @EnvironmentObject private var state: AppState
    @Namespace private var sidebarNamespace

    var body: some View {
        ZStack {
            LiquidGlassBackdrop()

            HStack(spacing: 0) {
                sidebar
                    .frame(width: 246)

                Divider()
                    .opacity(0.35)

                detailPane
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
        }
        .frame(minWidth: 780, idealWidth: 900, maxWidth: .infinity,
               minHeight: 540, idealHeight: 620, maxHeight: .infinity)
    }

    private var sidebar: some View {
        GlassEffectContainer(spacing: 10) {
            VStack(alignment: .leading, spacing: 14) {

                Color.clear.frame(height: 18)

                statusHeroCard

                Text("ĐIỀU HƯỚNG")
                    .font(.system(size: 10.5, weight: .bold))
                    .foregroundStyle(.secondary)
                    .padding(.horizontal, 12)
                    .padding(.top, 2)

                VStack(spacing: 4) {
                    ForEach(SettingsPage.allCases) { item in
                        sidebarRow(item)
                    }
                }

                Spacer(minLength: 0)

                hotkeyFooterCard
            }
            .padding(.horizontal, 14)
            .padding(.bottom, 16)
        }
        .background(.ultraThinMaterial.opacity(0.45))
    }

    private var statusHeroCard: some View {
        let effectiveOn = settings.vietnameseEnabled && !state.excludedNow && state.tapActive
        let badgeColor: Color = !state.tapActive ? .orange : (effectiveOn ? .green : .secondary)
        let statusTitle: String = {
            if !state.tapActive { return "Chưa cấp quyền" }
            if state.excludedNow { return "Tạm tắt (Loại trừ)" }
            return settings.vietnameseEnabled ? "Đang gõ Tiếng Việt" : "Đang gõ Tiếng Anh"
        }()
        let statusSub: String = {
            if !state.tapActive { return "Cần quyền Trợ năng" }
            if state.excludedNow {
                return state.activeAppName.isEmpty ? "Đã loại trừ" : state.activeAppName
            }
            return "Kiểu gõ: \(settings.inputMethodName)"
        }()

        return HStack(spacing: 10) {
            ZStack {
                RoundedRectangle(cornerRadius: 11, style: .continuous)
                    .fill(badgeColor.gradient)
                    .frame(width: 36, height: 36)
                Text(!state.tapActive ? "!" : (effectiveOn ? "V" : "E"))
                    .font(.system(size: 17, weight: .heavy, design: .rounded))
                    .foregroundStyle(.white)
            }

            VStack(alignment: .leading, spacing: 2) {
                Text(statusTitle)
                    .font(.system(size: 13, weight: .semibold))
                    .foregroundStyle(.primary)
                Text(statusSub)
                    .font(.system(size: 11))
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
            }
            Spacer(minLength: 0)

            Toggle("", isOn: $settings.vietnameseEnabled)
                .labelsHidden()
                .toggleStyle(.switch)
                .controlSize(.small)
                .disabled(state.excludedNow)
        }
        .padding(12)
        .glassEffect(.regular.interactive(), in: .rect(cornerRadius: 16))
    }

    private func sidebarRow(_ item: SettingsPage) -> some View {
        let selected = (state.page == item)
        return Button {
            withAnimation(.snappy(duration: 0.22)) {
                state.page = item
            }
        } label: {
            HStack(spacing: 11) {
                Image(systemName: item.icon)
                    .font(.system(size: 14, weight: .semibold))
                    .foregroundStyle(selected ? Color.white : Color.accentColor)
                    .frame(width: 26, height: 26)
                    .background(
                        RoundedRectangle(cornerRadius: 7, style: .continuous)
                            .fill(selected ? Color.accentColor : Color.accentColor.opacity(0.14))
                    )

                VStack(alignment: .leading, spacing: 1) {
                    Text(item.title)
                        .font(.system(size: 13, weight: selected ? .semibold : .medium))
                        .foregroundStyle(.primary)
                    Text(item.subtitle)
                        .font(.system(size: 10.5))
                        .foregroundStyle(.secondary)
                        .lineLimit(1)
                }
                Spacer(minLength: 0)
            }
            .padding(.horizontal, 10)
            .padding(.vertical, 8)
            .frame(maxWidth: .infinity, alignment: .leading)
            .background(
                RoundedRectangle(cornerRadius: 12, style: .continuous)
                    .fill(selected ? Color.accentColor.opacity(0.16) : Color.clear)
            )
            .overlay(
                RoundedRectangle(cornerRadius: 12, style: .continuous)
                    .strokeBorder(selected ? Color.accentColor.opacity(0.45) : Color.clear, lineWidth: 1)
            )
            .contentShape(Rectangle())
        }
        .buttonStyle(.plain)
    }

    private var hotkeyFooterCard: some View {
        HStack(spacing: 8) {
            Image(systemName: "command.square.fill")
                .font(.system(size: 14))
                .foregroundStyle(.secondary)
            VStack(alignment: .leading, spacing: 1) {
                Text(settings.hotkey.symbols.map { "Phím tắt: \($0)" } ?? "Phím tắt: Đang tắt")
                    .font(.system(size: 11.5, weight: .medium))
                Text("TKey v\(AboutTKey.version) · tien0246")
                    .font(.system(size: 10.5))
                    .foregroundStyle(.secondary)
            }
            Spacer(minLength: 0)
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 9)
        .glassEffect(.regular, in: .rect(cornerRadius: 12))
    }

    @ViewBuilder
    private var detailPane: some View {
        switch state.page {
        case .typing:
            TypingPage()
        case .macros:
            MacroPage()
        case .convert:
            ConvertPage()
        case .exclusion:
            ExclusionPage()
        case .general:
            GeneralPage()
        }
    }
}

struct GeneralPage: View {
    @EnvironmentObject private var settings: SettingsStore
    @EnvironmentObject private var state: AppState
    @State private var trusted: Bool?
    @State private var loginStatus: SMAppService.Status = .notRegistered
    @State private var loginError: String?
    @State private var didReset = false

    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 20) {
                pageHeader(
                    title: "Hệ thống & Quyền truy cập",
                    subtitle: "Quản lý quyền Trợ năng macOS, tự khởi động khi đăng nhập và thông tin bộ gõ."
                )

                GlassSection(title: "Quyền Trợ năng (Accessibility)", icon: "hand.raised.fill") {
                    permissionRow
                }

                GlassSection(title: "Khởi động cùng macOS", icon: "power.circle.fill") {
                    GlassToggleRow(
                        icon: "sunrise.fill",
                        iconColor: .blue,
                        title: "Mở TKey tự động khi đăng nhập",
                        subtitle: loginSubtitle,
                        isOn: launchAtLogin,
                        showDivider: false
                    )
                }

                GlassSection(title: "Thông tin & Khôi phục", icon: "info.circle.fill") {
                    GlassControlRow(
                        icon: "character.textbox",
                        iconColor: .purple,
                        title: "TKey — Bộ gõ Tiếng Việt cho macOS",
                        subtitle: "Phiên bản \(AboutTKey.version) · Tác giả tien0246 · Giao diện Liquid Glass (macOS 26)",
                        showDivider: true
                    ) {
                        Button("Giới thiệu") {
                            AboutTKey.show()
                        }
                        .buttonStyle(.glass)
                    }

                    GlassControlRow(
                        icon: "arrow.counterclockwise.circle.fill",
                        iconColor: .orange,
                        title: "Khôi phục cài đặt mặc định",
                        subtitle: didReset
                            ? "Đã đưa kiểu gõ (Telex), bảng mã (Unicode) và các tùy chọn về mặc định."
                            : "Đưa kiểu gõ, bảng mã, phím tắt và tùy chọn chính tả về trạng thái ban đầu.",
                        showDivider: false
                    ) {
                        Button(didReset ? "Đã khôi phục" : "Đặt lại mặc định") {
                            settings.resetToDefaults()
                            didReset = true
                        }
                        .buttonStyle(.glass)
                    }
                }
            }
            .padding(24)
            .frame(maxWidth: .infinity, alignment: .leading)
        }
        .task { await pollStatus() }
    }

    private func pollStatus() async {
        while !Task.isCancelled {
            let t = await Task.detached(priority: .utility) { AXIsProcessTrusted() }.value
            let st = await Task.detached(priority: .utility) { SMAppService.mainApp.status }.value
            trusted = t
            loginStatus = st
            try? await Task.sleep(for: .seconds(1.5))
        }
    }

    @ViewBuilder
    private var permissionRow: some View {
        let isTrusted = (trusted ?? state.tapActive) && state.tapActive
        HStack(alignment: .center, spacing: 14) {
            ZStack {
                RoundedRectangle(cornerRadius: 10, style: .continuous)
                    .fill((isTrusted ? Color.green : Color.orange).opacity(0.16))
                    .frame(width: 38, height: 38)
                Image(systemName: isTrusted ? "checkmark.shield.fill" : "exclamationmark.triangle.fill")
                    .font(.system(size: 18, weight: .semibold))
                    .foregroundStyle(isTrusted ? Color.green : Color.orange)
            }

            VStack(alignment: .leading, spacing: 3) {
                Text(isTrusted ? "TKey đang hoạt động bình thường" : "Cần cấp quyền Trợ năng cho TKey")
                    .font(.system(size: 14, weight: .semibold))
                Text(isTrusted
                     ? "Đã kết nối bộ chặn phím hệ thống (CGEventTap). Bạn có thể gõ tiếng Việt ở mọi ứng dụng."
                     : "Mở Cài đặt hệ thống › Quyền riêng tư & Bảo mật › Trợ năng, sau đó bật công tắc TKey. Nếu TKey đã bật sẵn sau khi build mới, hãy tắt rồi bật lại.")
                    .font(.system(size: 12))
                    .foregroundStyle(.secondary)
                    .fixedSize(horizontal: false, vertical: true)
            }
            .frame(maxWidth: .infinity, alignment: .leading)

            HStack(spacing: 8) {
                Button("Kiểm tra lại") {
                    if TKeyTapCallback.install() {
                        state.tapActive = true
                        trusted = true
                    } else {
                        trusted = AXIsProcessTrusted()
                    }
                }
                .buttonStyle(.glass)

                if !isTrusted {
                    Button("Mở Cài đặt…") {
                        openAccessibilitySettings()
                    }
                    .buttonStyle(.glassProminent)
                }
            }
        }
        .padding(16)
    }

    private var launchAtLogin: Binding<Bool> {
        Binding(
            get: { loginStatus == .enabled || loginStatus == .requiresApproval },
            set: { on in
                do {
                    try settings.setLaunchAtLogin(on)
                    loginError = nil
                    loginStatus = on ? .enabled : .notRegistered
                } catch {
                    loginError = "Không đổi được: \(error.localizedDescription)"
                }
            }
        )
    }

    private var loginSubtitle: String {
        if let loginError { return loginError }
        if loginStatus == .requiresApproval {
            return "Cần xác nhận cho phép TKey trong Cài đặt hệ thống › Cài đặt chung › Mục đăng nhập."
        }
        return "TKey chạy gọn trên thanh menu phía trên, không chiếm chỗ trên thanh Dock."
    }

    private func openAccessibilitySettings() {
        if let url = URL(string: "x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility") {
            NSWorkspace.shared.open(url)
        }
    }
}

func pageHeader(title: String, subtitle: String) -> some View {
    VStack(alignment: .leading, spacing: 4) {
        Text(title)
            .font(.system(size: 22, weight: .bold, design: .rounded))
            .foregroundStyle(.primary)
        Text(subtitle)
            .font(.system(size: 13))
            .foregroundStyle(.secondary)
            .fixedSize(horizontal: false, vertical: true)
    }
    .padding(.top, 4)
}
