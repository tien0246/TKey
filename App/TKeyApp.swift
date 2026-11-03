import SwiftUI

@main
struct TKeyApp: App {
    @NSApplicationDelegateAdaptor(AppDelegate.self) var appDelegate
    @StateObject private var settings = SettingsStore.shared
    @StateObject private var state = AppState.shared

    var body: some Scene {
        MenuBarExtra {
            MenuView()
                .environmentObject(settings)
                .environmentObject(state)
        } label: {
            Image(nsImage: MenuBarIcon.image(
                vietnamese: settings.vietnameseEnabled && !state.excludedNow,
                ready: state.tapActive))
        }
        .menuBarExtraStyle(.menu)
    }
}
