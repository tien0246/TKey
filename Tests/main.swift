import Foundation
import CoreGraphics

var screen: [UInt16] = []
func apply(_ charset: Int) {
    let bs = Int(tk_get_backspace_len())
    var buf = [UInt8](repeating: 0, count: 4104); var len: Int32 = 0
    if tk_get_buff_len() > 0 { tk_get_buff(&buf, &len) }
    if bs > 0 || len > 0 {
        for _ in 0..<bs where !screen.isEmpty { screen.removeLast() }
        for chunk in TKeyOutput.chunks(TKeyOutput.decode(buf, length: Int(len), outType: tk_get_out_type(), charset: charset)) {
            screen += chunk
        }
    }
}
func type(_ keys: String, charset: Int = 0) -> String {
    tkResetBuff(0); screen = []
    for u in keys.utf16 {
        if u == 0x3C { tkPutBackspace(); apply(charset)
            if tk_get_backspace_len() == 0 && tk_get_buff_len() == 0 && !screen.isEmpty { screen.removeLast() }
            continue }
        if u == 0x0A { tkProcessChar(13); apply(charset); screen.append(0x0A); continue }
        tkProcessChar(UInt32(u))
        let bs = Int(tk_get_backspace_len()); let bl = Int(tk_get_buff_len())
        if bs > 0 || bl > 0 { apply(charset) } else { screen.append(u) }
    }
    return String(decoding: screen, as: UTF16.self)
}
var fails = 0
func check(_ keys: String, _ want: String, charset: Int = 0) {
    let got = type(keys, charset: charset)
    let ok = got == want
    print(ok ? "✓" : "✗", keys.padding(toLength: 22, withPad: " ", startingAt: 0), "→", got.debugDescription, ok ? "" : "want \(want.debugDescription)")
    if !ok { fails += 1 }
}

tk_setup()
tk_enable_vietnamese(1); tk_set_input_method(0); tk_set_output_charset(0)
tk_set_option("spell_check", 1); tk_set_option("auto_restore", 1); tk_set_option("auto_upper", 0)
tk_set_option("free_mark", 1); tk_set_option("enable_macro", 0); tkResetBuff(0)

print("── Telex, Unicode")
check("kieeur", "kiểu"); check("Tieengs Vieejt", "Tiếng Việt"); check("dduowcj", "được")
check("nguwowif", "người"); check("VIEETJ", "VIỆT"); check("tieengs<", "tiến")
check("text ", "text ")
print("── VNI")
tk_set_input_method(1)
check("tie61ng vie65t", "tiếng việt"); check("d9uo75c", "được")
check("a8n", "ăn"); check("d9i", "đi")
tk_set_input_method(0)
print("── VIQR")
tk_set_input_method(2)
check("a(n", "ăn"); check("ddi", "đi"); check("u*", "ư")
tk_set_input_method(0)
print("── Telex đơn giản: dd → đ")
tk_set_input_method(4); check("ddi", "đi"); tk_set_input_method(5); check("ddi", "đi"); tk_set_input_method(0)
print("── Chuyển mã (CharsetCodec + engine decoders)")
func conv(_ s: String, _ a: Int, _ b: Int, _ want: String) {
    let got = CharsetCodec.convert(s, from: a, to: b) ?? "<nil>"
    print(got == want ? "ok  " : "FAIL", "\(a)→\(b)", s.debugDescription, "->", got.debugDescription)
    if got != want { fails += 1 }
}
let vnText = "Tiếng Việt: người được ưu tiên, khoẻ, hoà, thuỷ"
conv("tiÕng ViÖt", 20, 0, "tiếng Việt")
conv("Tie^'ng Vie^.t, DDa` Na(~ng", 10, 0, "Tiếng Việt, Đà Nẵng")
for id in [40, 10, 5, 4] { conv(CharsetCodec.convert(vnText, from: 0, to: id) ?? "", id, 0, vnText) }
print("── Bảng phím: FlowArrange không chồng chéo")
do {
    let sizes = [CGSize(width: 84, height: 30), CGSize(width: 135, height: 30),
                 CGSize(width: 84, height: 24), CGSize(width: 300, height: 30)]
    for w: CGFloat in [100, 260, 600] {
        let r = FlowArrange.arrange(sizes: sizes, maxWidth: w, spacing: 14, lineSpacing: 10)
        var overlap = false
        for i in 0..<sizes.count { for j in (i + 1)..<sizes.count {
            if CGRect(origin: r.origins[i], size: sizes[i]).intersects(CGRect(origin: r.origins[j], size: sizes[j])) { overlap = true }
        } }
        print(overlap ? "FAIL" : "ok  ", "width \(Int(w)) -> \(Int(r.size.height))pt, no overlap")
        if overlap { fails += 1 }
    }
}
print(fails == 0 ? "\nALL PASSED" : "\n\(fails) FAILED")
