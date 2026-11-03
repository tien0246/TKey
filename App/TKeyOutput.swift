import Foundation

enum TKeyOutput {
    static let maxUnitsPerEvent = 20

    static func decode(_ buf: [UInt8], length: Int, outType: Int32, charset: Int) -> [UInt16] {
        let n = max(0, min(length, buf.count))
        let wide = outType != 1 && (charset == 0 || charset == 3 || charset == 4)
        if !wide { return buf[0..<n].map { UInt16($0) } }
        var units = [UInt16]()
        units.reserveCapacity(n / 2)
        var i = 0
        while i + 1 < n {
            units.append(UInt16(buf[i]) | (UInt16(buf[i + 1]) << 8))
            i += 2
        }
        return units
    }

    static func chunks(_ units: [UInt16]) -> [[UInt16]] {
        var out = [[UInt16]]()
        var start = 0
        while start < units.count {
            var end = min(start + maxUnitsPerEvent, units.count)
            if end < units.count, end - start > 1, (0xD800...0xDBFF).contains(units[end - 1]) {
                end -= 1
            }
            out.append(Array(units[start..<end]))
            start = end
        }
        return out
    }
}
