import Foundation

enum FlowArrange {

    static func arrange(sizes: [CGSize], maxWidth: CGFloat,
                        spacing: CGFloat, lineSpacing: CGFloat) -> (origins: [CGPoint], size: CGSize) {
        var origins: [CGPoint] = []
        origins.reserveCapacity(sizes.count)
        var x: CGFloat = 0, y: CGFloat = 0, lineHeight: CGFloat = 0, widest: CGFloat = 0
        for s in sizes {
            if x > 0 && x + s.width > maxWidth {
                y += lineHeight + lineSpacing
                x = 0
                lineHeight = 0
            }
            origins.append(CGPoint(x: x, y: y))
            widest = max(widest, x + s.width)
            x += s.width + spacing
            lineHeight = max(lineHeight, s.height)
        }
        return (origins, CGSize(width: widest, height: sizes.isEmpty ? 0 : y + lineHeight))
    }
}
