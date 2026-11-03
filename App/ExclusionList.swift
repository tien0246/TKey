import SwiftUI
import AppKit

struct ExcludedApp: Identifiable, Hashable, Codable {
    var id: String
    var name: String
}

final class ExclusionList: ObservableObject {
    static let shared = ExclusionList()
    private static let key = "excludedApps"
    private static let seededDefaultsKey = "excludedAppsSeededV2"

    static let defaultExclusions: [ExcludedApp] = [
        ExcludedApp(id: "com.googlecode.iterm2", name: "iTerm2"),
        ExcludedApp(id: "com.hexrays.ida", name: "IDA Professional 9.5"),
    ]

    @Published private(set) var apps: [ExcludedApp] = []

    private init() {
        let d = UserDefaults.standard
        var loaded: [ExcludedApp] = []
        if let data = d.data(forKey: Self.key),
           let saved = try? JSONDecoder().decode([ExcludedApp].self, from: data) {
            loaded = saved
        }

        if !d.bool(forKey: Self.seededDefaultsKey) || loaded.isEmpty {
            d.set(true, forKey: Self.seededDefaultsKey)
            for item in Self.defaultExclusions where !loaded.contains(where: { $0.id == item.id }) {
                loaded.append(item)
            }
        }
        apps = loaded.sorted { $0.name.localizedStandardCompare($1.name) == .orderedAscending }
        persist()
    }

    func isExcluded(_ bundleID: String) -> Bool {
        !bundleID.isEmpty && apps.contains { $0.id == bundleID }
    }

    func add(bundleID: String, name: String) {
        let cleanID = bundleID.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !cleanID.isEmpty, !isExcluded(cleanID) else { return }
        apps.append(ExcludedApp(id: cleanID, name: name.isEmpty ? cleanID : name))
        apps.sort { $0.name.localizedStandardCompare($1.name) == .orderedAscending }
        persist()
    }

    func remove(bundleID: String) {
        apps.removeAll { $0.id == bundleID }
        persist()
    }

    private func persist() {
        if let data = try? JSONEncoder().encode(apps) {
            UserDefaults.standard.set(data, forKey: Self.key)
        }
    }
}

struct ExclusionPage: View {
    @EnvironmentObject private var list: ExclusionList
    @EnvironmentObject private var state: AppState
    @State private var runningCandidates: [ExcludedApp] = []

    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 20) {
                pageHeader(
                    title: "Loại trừ Ứng dụng",
                    subtitle: "TKey tự động chuyển sang gõ Tiếng Anh nguyên bản khi bạn làm việc trong các ứng dụng dưới đây (Terminal, IDE, công cụ dịch ngược) và bật lại Tiếng Việt ngay khi chuyển sang ứng dụng khác."
                )

                GlassSection(
                    title: "Đang loại trừ (\(list.apps.count) ứng dụng)",
                    subtitle: "Không xử lý dấu tiếng Việt trong các ứng dụng này",
                    icon: "macwindow.badge.plus"
                ) {
                    if list.apps.isEmpty {
                        HStack(spacing: 12) {
                            Image(systemName: "checkmark.circle")
                                .font(.system(size: 18))
                                .foregroundStyle(.secondary)
                            Text("Chưa có ứng dụng nào trong danh sách loại trừ. Hãy bấm “+ Thêm” ở danh sách ứng dụng đang mở bên dưới.")
                                .font(.system(size: 12.5))
                                .foregroundStyle(.secondary)
                        }
                        .padding(16)
                    } else {
                        ForEach(Array(list.apps.enumerated()), id: \.element.id) { index, app in
                            excludedAppRow(app, isLast: index == list.apps.count - 1)
                        }
                    }
                }

                GlassSection(
                    title: "Thêm nhanh từ ứng dụng đang chạy",
                    subtitle: "Bấm một lần để thêm vào danh sách loại trừ",
                    icon: "square.stack.3d.up.fill"
                ) {
                    VStack(alignment: .leading, spacing: 0) {
                        let available = runningCandidates.filter { !list.isExcluded($0.id) }
                        if available.isEmpty {
                            Text("Tất cả ứng dụng đang mở đều đã nằm trong danh sách loại trừ.")
                                .font(.system(size: 12.5))
                                .foregroundStyle(.secondary)
                                .padding(16)
                        } else {
                            ForEach(Array(available.enumerated()), id: \.element.id) { index, app in
                                candidateAppRow(app, showDivider: true)
                            }
                        }

                        HStack(spacing: 10) {
                            Button {
                                chooseFromApplications()
                            } label: {
                                Label("Chọn ứng dụng từ thư mục /Applications…", systemImage: "folder.badge.plus")
                            }
                            .buttonStyle(.glassProminent)

                            Button {
                                refreshRunningApps()
                            } label: {
                                Label("Làm mới danh sách", systemImage: "arrow.clockwise")
                            }
                            .buttonStyle(.glass)

                            Spacer()
                        }
                        .padding(14)
                    }
                }
            }
            .padding(24)
            .frame(maxWidth: .infinity, alignment: .leading)
        }
        .onAppear(perform: refreshRunningApps)
    }

    private func excludedAppRow(_ app: ExcludedApp, isLast: Bool) -> some View {
        let isActiveNow = (state.activeAppBundleID == app.id)
        return VStack(spacing: 0) {
            HStack(spacing: 14) {
                Image(nsImage: appIcon(for: app.id))
                    .resizable()
                    .frame(width: 32, height: 32)

                VStack(alignment: .leading, spacing: 2) {
                    HStack(spacing: 8) {
                        Text(app.name)
                            .font(.system(size: 13.5, weight: .semibold))
                        if isActiveNow {
                            Text("Đang dùng")
                                .font(.system(size: 10.5, weight: .semibold))
                                .foregroundStyle(.orange)
                                .padding(.horizontal, 7)
                                .padding(.vertical, 2)
                                .background(Capsule().fill(Color.orange.opacity(0.16)))
                        }
                    }
                    Text(app.id)
                        .font(.system(size: 11.5, design: .monospaced))
                        .foregroundStyle(.secondary)
                }
                .frame(maxWidth: .infinity, alignment: .leading)

                Button(role: .destructive) {
                    withAnimation(.snappy(duration: 0.2)) {
                        list.remove(bundleID: app.id)
                    }
                } label: {
                    Label("Gỡ bỏ", systemImage: "trash")
                        .font(.system(size: 12))
                }
                .buttonStyle(.glass)
                .controlSize(.small)
            }
            .padding(.horizontal, 16)
            .padding(.vertical, 11)

            if !isLast {
                Divider()
                    .padding(.leading, 62)
                    .opacity(0.5)
            }
        }
    }

    private func candidateAppRow(_ app: ExcludedApp, showDivider: Bool) -> some View {
        VStack(spacing: 0) {
            HStack(spacing: 14) {
                Image(nsImage: appIcon(for: app.id))
                    .resizable()
                    .frame(width: 28, height: 28)

                VStack(alignment: .leading, spacing: 1) {
                    Text(app.name)
                        .font(.system(size: 13, weight: .medium))
                    Text(app.id)
                        .font(.system(size: 11, design: .monospaced))
                        .foregroundStyle(.secondary)
                }
                .frame(maxWidth: .infinity, alignment: .leading)

                Button {
                    withAnimation(.snappy(duration: 0.2)) {
                        list.add(bundleID: app.id, name: app.name)
                    }
                } label: {
                    Label("Thêm", systemImage: "plus.circle.fill")
                        .font(.system(size: 12, weight: .medium))
                }
                .buttonStyle(.glass)
                .controlSize(.small)
            }
            .padding(.horizontal, 16)
            .padding(.vertical, 9)

            if showDivider {
                Divider()
                    .padding(.leading, 58)
                    .opacity(0.5)
            }
        }
    }

    private func refreshRunningApps() {
        let ourBundleID = Bundle.main.bundleIdentifier ?? "com.tien0246.tkey"
        var seen = Set<String>()
        var result: [ExcludedApp] = []
        for app in NSWorkspace.shared.runningApplications where app.activationPolicy == .regular {
            guard let id = app.bundleIdentifier,
                  id != ourBundleID,
                  seen.insert(id).inserted else { continue }
            result.append(ExcludedApp(id: id, name: app.localizedName ?? id))
        }
        runningCandidates = result.sorted { $0.name.localizedStandardCompare($1.name) == .orderedAscending }
    }

    private func chooseFromApplications() {
        let panel = NSOpenPanel()
        panel.title = "Chọn ứng dụng để loại trừ"
        panel.message = "Chọn một ứng dụng (.app) không muốn gõ tiếng Việt"
        panel.allowedContentTypes = [.application]
        panel.allowsMultipleSelection = true
        panel.canChooseDirectories = false
        panel.directoryURL = URL(fileURLWithPath: "/Applications")
        guard panel.runModal() == .OK else { return }
        for url in panel.urls {
            let bundle = Bundle(url: url)
            let bundleID = bundle?.bundleIdentifier ?? url.deletingPathExtension().lastPathComponent
            let name = (bundle?.object(forInfoDictionaryKey: "CFBundleDisplayName") as? String)
                ?? (bundle?.object(forInfoDictionaryKey: kCFBundleNameKey as String) as? String)
                ?? url.deletingPathExtension().lastPathComponent
            list.add(bundleID: bundleID, name: name)
        }
    }

    private func appIcon(for bundleID: String) -> NSImage {
        if let url = NSWorkspace.shared.urlsForApplications(withBundleIdentifier: bundleID).first {
            return NSWorkspace.shared.icon(forFile: url.path)
        }
        return NSWorkspace.shared.icon(for: .applicationBundle)
    }
}
