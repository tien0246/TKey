import SwiftUI
import AppKit

struct TKeyAppIconView: View {
    var body: some View {
        ZStack {

            RoundedRectangle(cornerRadius: 185, style: .continuous)
                .fill(
                    LinearGradient(
                        colors: [
                            Color(red: 0.05, green: 0.09, blue: 0.24),
                            Color(red: 0.09, green: 0.22, blue: 0.56),
                            Color(red: 0.12, green: 0.46, blue: 0.88),
                            Color(red: 0.18, green: 0.72, blue: 0.96)
                        ],
                        startPoint: .bottomTrailing,
                        endPoint: .topLeading
                    )
                )
                .frame(width: 824, height: 824)
                .shadow(color: .black.opacity(0.38), radius: 28, x: 0, y: 18)

            ZStack {
                Circle()
                    .fill(
                        RadialGradient(
                            colors: [Color.cyan.opacity(0.55), Color.clear],
                            center: .center,
                            startRadius: 10,
                            endRadius: 340
                        )
                    )
                    .frame(width: 680, height: 680)
                    .offset(x: -180, y: -200)

                Circle()
                    .fill(
                        RadialGradient(
                            colors: [Color.indigo.opacity(0.60), Color.clear],
                            center: .center,
                            startRadius: 20,
                            endRadius: 360
                        )
                    )
                    .frame(width: 700, height: 700)
                    .offset(x: 200, y: 220)

                LinearGradient(
                    colors: [
                        Color.white.opacity(0.22),
                        Color.white.opacity(0.03),
                        Color.clear
                    ],
                    startPoint: .topLeading,
                    endPoint: .center
                )
            }
            .frame(width: 824, height: 824)
            .clipShape(RoundedRectangle(cornerRadius: 185, style: .continuous))

            RoundedRectangle(cornerRadius: 185, style: .continuous)
                .strokeBorder(
                    LinearGradient(
                        colors: [
                            Color.white.opacity(0.65),
                            Color.white.opacity(0.18),
                            Color.white.opacity(0.06),
                            Color.cyan.opacity(0.35)
                        ],
                        startPoint: .topLeading,
                        endPoint: .bottomTrailing
                    ),
                    lineWidth: 4
                )
                .frame(width: 824, height: 824)

            ZStack {

                RoundedRectangle(cornerRadius: 132, style: .continuous)
                    .fill(Color.black.opacity(0.42))
                    .frame(width: 560, height: 560)
                    .offset(y: 34)
                    .blur(radius: 22)

                RoundedRectangle(cornerRadius: 128, style: .continuous)
                    .fill(
                        LinearGradient(
                            colors: [
                                Color(red: 0.72, green: 0.85, blue: 0.98).opacity(0.42),
                                Color(red: 0.10, green: 0.20, blue: 0.44).opacity(0.78)
                            ],
                            startPoint: .top,
                            endPoint: .bottom
                        )
                    )
                    .frame(width: 556, height: 556)
                    .offset(y: 22)
                    .overlay(
                        RoundedRectangle(cornerRadius: 128, style: .continuous)
                            .strokeBorder(Color.white.opacity(0.28), lineWidth: 2)
                            .offset(y: 22)
                    )

                RoundedRectangle(cornerRadius: 116, style: .continuous)
                    .fill(
                        LinearGradient(
                            colors: [
                                Color.white.opacity(0.34),
                                Color.white.opacity(0.16),
                                Color(red: 0.55, green: 0.82, blue: 1.0).opacity(0.22)
                            ],
                            startPoint: .topLeading,
                            endPoint: .bottomTrailing
                        )
                    )
                    .frame(width: 520, height: 510)
                    .offset(y: -8)
                    .overlay(
                        RoundedRectangle(cornerRadius: 108, style: .continuous)
                            .fill(
                                RadialGradient(
                                    colors: [
                                        Color.white.opacity(0.20),
                                        Color.clear,
                                        Color.black.opacity(0.10)
                                    ],
                                    center: .topLeading,
                                    startRadius: 30,
                                    endRadius: 420
                                )
                            )
                            .padding(10)
                            .offset(y: -8)
                    )
                    .overlay(
                        RoundedRectangle(cornerRadius: 116, style: .continuous)
                            .strokeBorder(
                                LinearGradient(
                                    colors: [
                                        Color.white.opacity(0.90),
                                        Color.white.opacity(0.32),
                                        Color.white.opacity(0.12),
                                        Color.cyan.opacity(0.48)
                                    ],
                                    startPoint: .topLeading,
                                    endPoint: .bottomTrailing
                                ),
                                lineWidth: 4.5
                            )
                            .offset(y: -8)
                    )

                VStack(spacing: 0) {
                    HStack(spacing: 10) {
                        Circle()
                            .fill(Color(red: 0.20, green: 0.95, blue: 0.50))
                            .frame(width: 18, height: 18)
                            .shadow(color: Color.green.opacity(0.9), radius: 8)

                        Text("TKEY")
                            .font(.system(size: 24, weight: .heavy, design: .rounded))
                            .tracking(4)
                            .foregroundStyle(Color.white.opacity(0.88))

                        Spacer()

                        Text("VN")
                            .font(.system(size: 20, weight: .black, design: .rounded))
                            .foregroundStyle(Color(red: 0.08, green: 0.22, blue: 0.52))
                            .padding(.horizontal, 14)
                            .padding(.vertical, 5)
                            .background(
                                Capsule(style: .continuous)
                                    .fill(Color.white.opacity(0.90))
                            )
                    }
                    .padding(.horizontal, 48)
                    .padding(.top, 38)

                    Spacer(minLength: 0)

                    Text("V")
                        .font(.system(size: 265, weight: .black, design: .rounded))
                        .foregroundStyle(
                            LinearGradient(
                                colors: [
                                    Color.white,
                                    Color(red: 0.82, green: 0.94, blue: 1.0)
                                ],
                                startPoint: .top,
                                endPoint: .bottom
                            )
                        )
                        .shadow(color: Color.cyan.opacity(0.35), radius: 18, x: 0, y: 0)
                        .shadow(color: Color.black.opacity(0.28), radius: 12, x: 0, y: 8)
                        .offset(y: -6)

                    Spacer(minLength: 0)

                    HStack(spacing: 14) {
                        Text("TELEX")
                        Text("·")
                        Text("VNI")
                        Text("·")
                        Text("VIQR")
                    }
                    .font(.system(size: 19, weight: .bold, design: .monospaced))
                    .foregroundStyle(Color.white.opacity(0.72))
                    .padding(.bottom, 42)
                }
                .frame(width: 520, height: 510)
                .offset(y: -8)
            }
        }
        .frame(width: 1024, height: 1024)
    }
}

@MainActor
func renderIcon(to path: String) {
    let renderer = ImageRenderer(content: TKeyAppIconView())
    renderer.scale = 1.0
    guard let cgImage = renderer.cgImage else {
        fatalError("Failed to render icon")
    }
    let rep = NSBitmapImageRep(cgImage: cgImage)
    rep.size = NSSize(width: 1024, height: 1024)
    guard let png = rep.representation(using: .png, properties: [:]) else {
        fatalError("Failed to encode PNG")
    }
    try! png.write(to: URL(fileURLWithPath: path))
    print("✓ Rendered 1024x1024 icon to \(path)")
}

MainActor.assumeIsolated {
    renderIcon(to: "build/Icon1024_v2.png")
}
