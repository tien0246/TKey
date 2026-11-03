import Foundation

enum CharsetCodec {

    static func engineID(_ uiCharset: Int) -> Int32 {
        uiCharset == 0 ? 1 : Int32(uiCharset)
    }

    static func encode(_ text: String, charset: Int) -> [UInt8] {
        switch charset {
        case 0:
            return Array(text.utf8)
        case 4:
            var out = [UInt8]()
            out.reserveCapacity(text.utf16.count * 2)
            for u in text.utf16 {
                out.append(UInt8(u & 0xFF))
                out.append(UInt8(u >> 8))
            }
            return out
        default:
            return text.unicodeScalars.map(byte(for:))
        }
    }

    static func decode(_ bytes: [UInt8], charset: Int) -> String {
        switch charset {
        case 0:
            return String(decoding: bytes, as: UTF8.self)
        case 4:
            var units = [UInt16]()
            var i = 0
            while i + 1 < bytes.count {
                units.append(UInt16(bytes[i]) | (UInt16(bytes[i + 1]) << 8))
                i += 2
            }
            return String(decoding: units, as: UTF16.self)
        default:
            var scalars = String.UnicodeScalarView()
            for b in bytes { scalars.append(scalar(for: b)) }
            return String(scalars)
        }
    }

    static func convert(_ text: String, from: Int, to: Int) -> String? {
        if text.isEmpty { return "" }
        if from == to { return text }
        let input = encode(text, charset: from)
        var capacity = max(256, input.count * 4)
        while capacity <= 64 * 1024 * 1024 {
            var output = [UInt8](repeating: 0, count: capacity)
            var outSize = Int32(capacity)
            let r = input.withUnsafeBufferPointer { inBuf in
                output.withUnsafeMutableBufferPointer { outBuf in
                    tk_convert(engineID(from), engineID(to),
                               inBuf.baseAddress!, Int32(input.count),
                               outBuf.baseAddress!, &outSize)
                }
            }
            if r == 0 {
                return decode(Array(output[0..<Int(max(0, outSize))]), charset: to)
            }
            capacity *= 4
        }
        return nil
    }

    private static let cp1252High: [UInt32] = [
        0x20AC, 0x81, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
        0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x8D, 0x017D, 0x8F,
        0x90, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
        0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x9D, 0x017E, 0x0178,
    ]

    private static let cp1252Reverse: [UInt32: UInt8] = {
        var map = [UInt32: UInt8]()
        for (i, v) in cp1252High.enumerated() { map[v] = UInt8(0x80 + i) }
        return map
    }()

    private static func byte(for s: Unicode.Scalar) -> UInt8 {
        let v = s.value
        if v < 0x80 || (v >= 0xA0 && v <= 0xFF) { return UInt8(v) }
        if let b = cp1252Reverse[v] { return b }
        return 0x3F
    }

    private static func scalar(for b: UInt8) -> Unicode.Scalar {
        if b >= 0x80 && b <= 0x9F {
            return Unicode.Scalar(cp1252High[Int(b) - 0x80]) ?? "?"
        }
        return Unicode.Scalar(b)
    }
}
