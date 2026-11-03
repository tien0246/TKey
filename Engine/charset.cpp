#include "charset.h"
#include "vnlib.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

int wideCharCompare(const void *a, const void *b)
{
    unsigned short x = *(unsigned short *)a;
    unsigned short y = *(unsigned short *)b;
    if (x == y) return 0;
    return (x > y) ? 1 : -1;
}

int uniCompInfoCompare(const void *a, const void *b)
{
    unsigned int x = *(const unsigned int *)a;
    unsigned int y = *(const unsigned int *)b;
    if (x == y) return 0;
    return (x > y) ? 1 : -1;
}

unsigned int ConvertUTF32ToUTF16(unsigned int *input, unsigned int count,
                                 unsigned short *output, unsigned int &outCount)
{
    unsigned int n = 0;
    if (output) {
        for (unsigned int i = 0; i < count; i++) {
            unsigned int c = input[i];
            if (c > 0xFFFF) {
                if (c <= 0x10FFFF) {
                    unsigned int v = c - 0x10000;
                    output[n++] = (unsigned short)((v >> 10) + 0xD800);
                    output[n++] = (unsigned short)((v & 0x3FF) + 0xDC00);
                } else {
                    output[n++] = 0xFFFD;
                }
            } else if (c < 0xD800 || c > 0xDFFF) {
                output[n++] = (unsigned short)c;
            } else {
                output[n++] = 0xFFFD;
            }
        }
        output[n] = 0;
    }
    outCount = n;
    return 0;
}

int hexDigitValue(unsigned char ch)
{
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return 0;
}

unsigned int StdVnToUpper(unsigned int c)
{
    if (c >= 0x10000 && c < 0x100BA && (c & 1) != 0)
        return c - 1;
    return c;
}

unsigned int StdVnToLower(unsigned int c)
{
    if (c >= 0x10000 && c < 0x100BA && (c & 1) == 0)
        return c + 1;
    return c;
}

unsigned int StdVnGetRoot(unsigned int c)
{
    if (c >= 0x10000 && c < 0x100D5)
        return (unsigned int)StdVnRootCharTable[c - 0x10000] + 0x10000;
    return c;
}

void VnConvResetOptions(VnConvOptions *opt)
{
    opt->FixedFirst = 0;
    opt->ViqrEscape = 1;
    opt->ToUpper = 0;
    opt->ToLower = 0;
    opt->UpperFirstCharOfSentence = 0;
    opt->UpperFirstCharOfWord = 0;
    opt->RemoveTone = 0;
    opt->OldStyleVIQR = 0;
}

int VnCharset::elementSize() { return 1; }

void *VnCharset::startInput() { return 0; }
void *VnCharset::startOutput() { return 0; }

int VnCharset::putCharToUTF16(ByteOutStream &out, unsigned int c)
{
    unsigned int input[1] = { c };
    unsigned short buf[4];
    unsigned int n = 0;
    int written = 0;
    ConvertUTF32ToUTF16(input, 1, buf, n);
    for (unsigned int i = 0; i < n; i++) {
        if (out.putW(buf[i]))
            written += 2;
    }
    return written;
}

int VnInternalCharset::nextInput(ByteInStream &in, unsigned int &c, int &bytesRead)
{
    if (in.getNextDW(c)) {
        bytesRead = 4;
        return 1;
    }
    bytesRead = 0;
    return 0;
}

int VnInternalCharset::putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode)
{
    (void)bbs;
    if (toUnicode)
        return putCharToUTF16(out, c) != 0;
    bytesWritten = 4;
    int ok1 = out.putW((unsigned short)(c & 0xFFFF));
    int ok2 = out.putW((unsigned short)((c >> 16) & 0xFFFF));
    return ok1 && ok2;
}

int VnInternalCharset::elementSize() { return 4; }

SingleByteCharset::SingleByteCharset(unsigned char *table)
{
    tbl = table;
    memset(revMap, 0, sizeof(revMap));
    for (int i = 0; i < 213; i++) {
        if (tbl[i] != 0 && (i == 212 || tbl[i] != tbl[i + 1]))
            revMap[tbl[i]] = (unsigned short)(i + 1);
    }
}

int SingleByteCharset::nextInput(ByteInStream &in, unsigned int &c, int &bytesRead)
{
    unsigned char ch;
    if (in.getNext(ch)) {

        if (revMap[ch] != 0)
            c = 0x10000 + (unsigned int)(revMap[ch] - 1);
        else
            c = ch;
        bytesRead = 1;
        return 1;
    }
    bytesRead = 0;
    return 0;
}

int SingleByteCharset::putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode)
{
    (void)bbs;
    if (c < 0x10000) {
        if (c > 0xFF || revMap[c] != 0) {
            bytesWritten = 1;
            return out.putB('?');
        }
        bytesWritten = 1;
        if (!toUnicode)
            return out.putB((unsigned char)c);
        bytesWritten = putCharToUTF16(out, c);
        return bytesWritten != 0;
    }
    if (c - 0x10000 >= 0xD5) {
        bytesWritten = putCharToUTF16(out, c);
        return bytesWritten != 0;
    }
    bytesWritten = 1;
    unsigned char b = tbl[c - 0x10000];
    if (b == 0) {

        if (c == 65737) b = '"';
        else if (c == 65738) b = '"';
        else if (c == 65726) b = '.';
        else b = '#';
    }
    if (toUnicode) {
        bytesWritten = putCharToUTF16(out, c);
        return bytesWritten != 0;
    }
    return out.putB(b);
}

DoubleByteCharset::DoubleByteCharset(unsigned short *table)
{
    tbl = table;
    memset(revMap, 0, sizeof(revMap));
    for (int i = 0; i < 213; i++) {
        if ((tbl[i] >> 8) != 0)
            revMap[tbl[i] >> 8] = (unsigned short)0xFFFF;
        else if (revMap[tbl[i]] == 0)
            revMap[tbl[i]] = (unsigned short)(i + 1);
        wideChars[i] = tbl[i] + ((unsigned int)i << 16);
    }
    qsort(wideChars, 213, 4, wideCharCompare);
}

int DoubleByteCharset::nextInput(ByteInStream &in, unsigned int &c, int &bytesRead)
{
    unsigned char first, second;
    bytesRead = 0;
    if (!in.getNext(first))
        return 0;
    bytesRead = 1;
    c = revMap[first];
    if (c != 0) {
        if (c == 0xFFFF) {
            c = 0xFFFFFFFF;
        } else {
            c += 0xFFFF;
            if (in.peekNext(second) && second != 0) {

                unsigned int key = ((unsigned int)second << 8) | first;
                unsigned int *found = (unsigned int *)bsearch(&key, wideChars, 213, 4, wideCharCompare);
                if (found) {
                    c = ((*found) >> 16) + 0x10000;
                    bytesRead = 2;
                    in.getNext(second);
                }
            }
        }
    } else {
        c = first;
    }
    return 1;
}

int DoubleByteCharset::putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode)
{
    (void)bbs;
    if (c < 0x10000) {
        if (c > 0xFF || revMap[c] != 0) {
            bytesWritten = 1;
            if (toUnicode)
                return putCharToUTF16(out, '#') != 0;
            return out.putB('?');
        }
        bytesWritten = 1;
        if (toUnicode)
            return putCharToUTF16(out, c) != 0;
        return out.putB((unsigned char)c);
    }
    if (c - 0x10000 >= 0xD5) {
        bytesWritten = putCharToUTF16(out, c);
        return bytesWritten != 0;
    }
    unsigned short v = tbl[c - 0x10000];
    if ((v & 0xFF00) != 0) {
        bytesWritten = 2;
        if (toUnicode) {
            putCharToUTF16(out, (unsigned char)v);
            return putCharToUTF16(out, (unsigned char)(v >> 8)) != 0;
        }
        out.putB((unsigned char)v);
        return out.putB((unsigned char)(v >> 8));
    }
    unsigned char b = (unsigned char)tbl[c - 0x10000];
    if (revMap[b] == 0xFFFF)
        b = '?';
    bytesWritten = 1;
    if (toUnicode)
        return putCharToUTF16(out, b) != 0;
    return out.putB(b);
}

UnicodeCharset::UnicodeCharset(unsigned short *uniTable)
{
    tbl = uniTable;
    for (int i = 0; i < 213; i++)
        info[i] = uniTable[i] + ((unsigned int)i << 16);
    qsort(info, 213, 4, wideCharCompare);
}

int UnicodeCharset::nextInput(ByteInStream &in, unsigned int &c, int &bytesRead)
{
    unsigned short w;
    if (in.getNextW(w)) {
        bytesRead = 2;
        unsigned int *found = (unsigned int *)bsearch(&w, info, 213, 4, wideCharCompare);
        if (found)
            c = ((*found) >> 16) + 0x10000;
        else
            c = w;
        return 1;
    }
    bytesRead = 0;
    return 0;
}

int UnicodeCharset::putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode)
{
    (void)bbs;
    bytesWritten = 2;
    if (c < 0x10000) {
        return out.putW((unsigned short)c);
    } else if (c - 0x10000 >= 0xD5) {
        return putCharToUTF16(out, c) != 0;
    } else if (toUnicode) {
        return putCharToUTF16(out, tbl[c - 0x10000]) != 0;
    }
    return out.putW(tbl[c - 0x10000]);
}

UnicodeCompCharset::UnicodeCompCharset(unsigned short *uniTable, unsigned int *compositeTable)
{
    uniTbl = uniTable;
    compTbl = compositeTable;
    count = 0;
    for (int i = 0; i < 213; i++) {
        info[count].comp = compositeTable[i];
        info[count].index = i;
        count++;
    }
    for (int i = 0; i < 213; i++) {
        if (uniTable[i] != compositeTable[i]) {
            info[count].comp = uniTable[i];
            info[count].index = i;
            count++;
        }
    }
    qsort(info, count, 8, uniCompInfoCompare);
}

int UnicodeCompCharset::nextInput(ByteInStream &in, unsigned int &c, int &bytesRead)
{
    unsigned short w;
    if (!in.getNextW(w)) {
        bytesRead = 0;
        return 0;
    }
    bytesRead = 2;
    unsigned int key = w;
    unsigned int *found = (unsigned int *)bsearch(&key, info, count, 8, uniCompInfoCompare);
    if (found) {
        c = found[1] + 0x10000;
        unsigned short w2;
        if (in.peekNextW(w2) && w2 != 0) {
            unsigned int key2 = key + ((unsigned int)w2 << 16);
            unsigned int *found2 = (unsigned int *)bsearch(&key2, info, count, 8, uniCompInfoCompare);
            if (found2) {
                c = found2[1] + 0x10000;
                bytesRead += 2;
                in.getNextW(w);
            }
        }
    } else {
        c = key;
    }
    return 1;
}

int UnicodeCompCharset::putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode)
{

    (void)bbs;
    if (c >= 0x10000 && c - 0x10000 < 213) {
        unsigned int v = compTbl[c - 0x10000];
        unsigned short lo = (unsigned short)(v & 0xFFFF);
        unsigned short hi = (unsigned short)(v >> 16);
        int ok;
        if (toUnicode) {
            bytesWritten = putCharToUTF16(out, lo);
            ok = bytesWritten != 0;
            if (hi) { int n = putCharToUTF16(out, hi); bytesWritten += n; ok = n != 0; }
            return ok;
        }
        bytesWritten = 2;
        ok = out.putW(lo);
        if (hi) { bytesWritten += 2; ok = out.putW(hi); }
        return ok;
    }
    if (c >= 0x10000) {
        bytesWritten = putCharToUTF16(out, c);
        return bytesWritten != 0;
    }
    bytesWritten = 2;
    if (toUnicode)
        return putCharToUTF16(out, c) != 0;
    return out.putW((unsigned short)c);
}

int UnicodeCompCharset::elementSize() { return 2; }

UnicodeUTF8Charset::UnicodeUTF8Charset(unsigned short *uniTable)
{
    tbl = uniTable;
    for (int i = 0; i < 213; i++)
        info[i] = uniTable[i] + ((unsigned int)i << 16);
    qsort(info, 213, 4, wideCharCompare);
}

int UnicodeUTF8Charset::nextInput(ByteInStream &in, unsigned int &c, int &bytesRead)
{
    unsigned char b0, b1, b2, b3;
    bytesRead = 0;
    if (!in.getNext(b0))
        return 0;
    bytesRead = 1;
    unsigned int v = b0;
    if (b0 >= 0x80) {
        if ((b0 & 0xE0) == 0xC0) {
            if (!in.peekNext(b1)) return 0;
            if ((b1 & 0xC0) != 0x80) { c = 0xFFFFFFFF; return 1; }
            in.getNext(b1);
            bytesRead = 2;
            v = (b1 & 0x3F) | ((b0 & 0x1F) << 6);
        } else if ((b0 & 0xF0) == 0xE0) {
            if (!in.peekNext(b1)) return 0;
            if ((b1 & 0xC0) != 0x80) { c = 0xFFFFFFFF; return 1; }
            in.getNext(b1);
            bytesRead = 2;
            if (!in.peekNext(b2)) return 0;
            if ((b2 & 0xC0) != 0x80) { c = 0xFFFFFFFF; return 1; }
            in.getNext(b2);
            bytesRead = 3;
            v = (b2 & 0x3F) | ((b1 & 0x3F) << 6) | ((b0 & 0xF) << 12);
        } else {
            if ((b0 & 0xF0) != 0xF0) { c = 0xFFFFFFFF; return 1; }
            if (!in.peekNext(b1)) return 0;
            if ((b1 & 0xC0) != 0x80) { c = 0xFFFFFFFF; return 1; }
            in.getNext(b1);
            bytesRead = 2;
            if (!in.peekNext(b2)) return 0;
            if ((b2 & 0xC0) != 0x80) { c = 0xFFFFFFFF; return 1; }
            in.getNext(b2);
            if (!in.peekNext(b3)) return 0;
            if ((b3 & 0xC0) != 0x80) { c = 0xFFFFFFFF; return 1; }
            in.getNext(b3);
            bytesRead = 4;
            v = (b3 & 0x3F) | ((b2 & 0x3F) << 6) | ((b1 & 0x3F) << 12) | ((b0 & 0xF) << 18);
        }
    }
    unsigned int *found = (unsigned int *)bsearch(&v, info, 213, 4, wideCharCompare);
    if (found)
        c = ((*found) >> 16) + 0x10000;
    else
        c = v;
    return 1;
}

int UnicodeUTF8Charset::putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode)
{
    (void)bbs; (void)toUnicode;
    unsigned int v;
    if (c < 0x10000 || c == (unsigned int)-1)
        v = c;
    else if ((int)(c - 0x10000) < 213)
        v = tbl[c - 0x10000];
    else
        v = c;

    if (v < 0x80) {
        bytesWritten = 1;
        return out.putB((unsigned char)v);
    } else if (v < 0x800) {
        bytesWritten = 2;
        out.putB((unsigned char)((v >> 6) | 0xC0));
        return out.putB((unsigned char)((v & 0x3F) | 0x80));
    } else if (v < 0x10000) {
        bytesWritten = 3;
        out.putB((unsigned char)((v >> 12) | 0xE0));
        out.putB((unsigned char)(((v >> 6) & 0x3F) | 0x80));
        return out.putB((unsigned char)((v & 0x3F) | 0x80));
    }
    bytesWritten = 4;
    out.putB((unsigned char)((v >> 18) | 0xF0));
    out.putB((unsigned char)(((v >> 12) & 0x3F) | 0x80));
    out.putB((unsigned char)(((v >> 6) & 0x3F) | 0x80));
    return out.putB((unsigned char)((v & 0x3F) | 0x80));
}

UnicodeRefCharset::UnicodeRefCharset(unsigned short *uniTable)
{
    tbl = uniTable;
    for (int i = 0; i < 213; i++)
        info[i] = uniTable[i] + ((unsigned int)i << 16);
    qsort(info, 213, 4, wideCharCompare);
}

int UnicodeRefCharset::nextInput(ByteInStream &in, unsigned int &c, int &bytesRead)
{
    unsigned char ch;
    bytesRead = 0;
    if (!in.getNext(ch))
        return 0;
    bytesRead = 1;
    unsigned short v = ch;
    if (ch == '&' && in.peekNext(ch) && ch == '#') {
        in.getNext(ch);
        bytesRead++;
        if (!in.eos()) {
            if (in.peekNext(ch) && (ch == 'x' || ch == 'X')) {
                in.getNext(ch);
                bytesRead++;
                unsigned short acc = 0;
                for (int i = 0; in.peekNext(ch) && isxdigit(ch) && i < 4; i++) {
                    in.getNext(ch);
                    bytesRead++;
                    acc = hexDigitValue(ch) + 16 * acc;
                }
                if (in.peekNext(ch) && ch == ';') {
                    in.getNext(ch);
                    bytesRead++;
                    v = acc;
                }
            } else {
                unsigned short acc = 0;
                for (int j = 0; in.peekNext(ch) && isdigit(ch) && j < 5; j++) {
                    in.getNext(ch);
                    bytesRead++;
                    acc = (ch - '0') + 10 * acc;
                }
                if (in.peekNext(ch) && ch == ';') {
                    in.getNext(ch);
                    bytesRead++;
                    v = acc;
                }
            }
        }
    }
    unsigned int key = v;
    unsigned int *found = (unsigned int *)bsearch(&key, info, 213, 4, wideCharCompare);
    if (found)
        c = ((*found) >> 16) + 0x10000;
    else
        c = v;
    return 1;
}

int UnicodeRefCharset::putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode)
{
    (void)bbs; (void)toUnicode;
    unsigned int v;
    if (c >= 0x10000) {
        if (c - 0x10000 >= 0xD5)
            v = c;
        else
            v = tbl[c - 0x10000];
    } else {
        v = (unsigned short)c;
    }
    if (v < 0x80) {
        bytesWritten = 1;
        return out.putB((unsigned char)v);
    }
    bytesWritten = 2;
    out.putB('&');
    out.putB('#');
    int started = 0;
    int divisor = 10000;
    for (int i = 0; i < 5; i++) {
        if (v / divisor != 0 || started) {
            started = 1;
            bytesWritten++;
            out.putB((unsigned char)(v / divisor + '0'));
        }
        v %= (unsigned int)divisor;
        divisor /= 10;
    }
    out.putB(';');
    bytesWritten++;
    return 1;
}

UnicodeHexCharset::UnicodeHexCharset(unsigned short *uniTable)
{
    tbl = uniTable;
    for (int i = 0; i < 213; i++)
        info[i] = uniTable[i] + ((unsigned int)i << 16);
    qsort(info, 213, 4, wideCharCompare);
}

int UnicodeHexCharset::nextInput(ByteInStream &in, unsigned int &c, int &bytesRead)
{

    UnicodeRefCharset ref(tbl);
    return ref.nextInput(in, c, bytesRead);
}

int UnicodeHexCharset::putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode)
{
    (void)bbs; (void)toUnicode;
    unsigned short v;
    if (c >= 0x10000)
        v = tbl[c - 0x10000];
    else
        v = (unsigned short)c;

    if (v < 0x100) {
        bytesWritten = 1;
        return out.putB((unsigned char)v);
    }
    bytesWritten = 3;
    out.putB('&');
    out.putB('#');
    out.putB('x');
    int started = 0;
    int shift = 12;
    for (int i = 0; i < 4; i++) {
        int digit = (v >> shift) & 0xF;
        if (digit != 0 || started) {
            started = 1;
            bytesWritten++;
            unsigned char ch = (digit >= 10) ? (digit + 55) : (digit + 48);
            out.putB(ch);
        }
        shift -= 4;
    }
    out.putB(';');
    bytesWritten++;
    return 1;
}

UnicodeCStringCharset::UnicodeCStringCharset(unsigned short *uniTable)
    : UnicodeHexCharset(uniTable)
{
    lastOutWasHex = 0;
}

void *UnicodeCStringCharset::startInput()
{
    lastOutWasHex = 0;
    return 0;
}

int UnicodeCStringCharset::nextInput(ByteInStream &in, unsigned int &c, int &bytesRead)
{
    unsigned char ch;
    bytesRead = 0;
    if (!in.getNext(ch))
        return 0;
    bytesRead = 1;
    unsigned short v = ch;
    if (ch == '\\' && in.peekNext(ch) && (ch == 'x' || ch == 'X')) {
        in.getNext(ch);
        bytesRead++;
        unsigned short acc = 0;
        for (int i = 0; in.peekNext(ch) && isxdigit(ch) && i < 4; i++) {
            in.getNext(ch);
            bytesRead++;
            acc = hexDigitValue(ch) + 16 * acc;
        }
        v = acc;
    }
    unsigned int key = v;
    unsigned int *found = (unsigned int *)bsearch(&key, info, 213, 4, wideCharCompare);
    if (found)
        c = ((*found) >> 16) + 0x10000;
    else
        c = v;
    return 1;
}

int UnicodeCStringCharset::putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode)
{
    (void)bbs; (void)toUnicode;
    unsigned short v;
    if (c >= 0x10000)
        v = tbl[c - 0x10000];
    else
        v = (unsigned short)c;

    if (v < 0x80 && !isxdigit(v) && v != 'x' && v != 'X') {
        bytesWritten = 1;
        return out.putB((unsigned char)v);
    }
    bytesWritten = 2;
    out.putB('\\');
    out.putB('x');
    int started = 0;
    int shift = 12;
    for (int i = 0; i < 4; i++) {
        int digit = (v >> shift) & 0xF;
        if (digit != 0 || started) {
            started = 1;
            bytesWritten++;
            unsigned char ch = (digit >= 10) ? (digit + 55) : (digit + 48);
            out.putB(ch);
        }
        shift -= 4;
    }
    out.isOK();
    lastOutWasHex = 1;
    return 1;
}

WinCP1258Charset::WinCP1258Charset(unsigned short *cpTable, unsigned short *preTable)
{
    cpTbl = cpTable;
    preTbl = preTable;
    memset(revMap, 0, sizeof(revMap));
    for (int i = 0; i < 213; i++) {
        if ((cpTbl[i] >> 8) != 0)
            revMap[cpTbl[i] >> 8] = (unsigned short)0xFFFF;
        else if (revMap[cpTbl[i]] == 0)
            revMap[cpTbl[i]] = (unsigned short)(i + 1);
        wideChars[i] = cpTbl[i] + ((unsigned int)i << 16);
    }
    wideCount = 213;
    for (int i = 0; i < 213; i++) {
        if (preTbl[i] != cpTbl[i]) {
            if ((preTbl[i] >> 8) != 0)
                revMap[preTbl[i] >> 8] = (unsigned short)0xFFFF;
            else if (revMap[preTbl[i]] == 0)
                revMap[preTbl[i]] = (unsigned short)(i + 1);
            wideChars[wideCount] = preTbl[i] + ((unsigned int)i << 16);
            wideCount++;
        }
    }
    qsort(wideChars, wideCount, 4, wideCharCompare);
}

int WinCP1258Charset::nextInput(ByteInStream &in, unsigned int &c, int &bytesRead)
{
    unsigned char first, second;
    bytesRead = 0;
    if (!in.getNext(first))
        return 0;
    bytesRead = 1;
    c = revMap[first];
    if (c != 0) {
        if (c == 0xFFFF) {
            c = 0xFFFFFFFF;
        } else {
            c += 0xFFFF;
            if (in.peekNext(second) && second != 0) {
                unsigned int key = ((unsigned int)second << 8) | first;
                unsigned int *found = (unsigned int *)bsearch(&key, wideChars, wideCount, 4, wideCharCompare);
                if (found) {
                    c = ((*found) >> 16) + 0x10000;
                    bytesRead = 2;
                    in.getNext(second);
                }
            }
        }
    } else {
        c = first;
    }
    return 1;
}

int WinCP1258Charset::putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode)
{
    (void)bbs; (void)toUnicode;
    if (c >= 0x10000) {
        if (c - 0x10000 < 213) {
            unsigned short v = preTbl[c - 0x10000];
            if ((v >> 8) != 0) {
                bytesWritten = 2;
                out.putB((unsigned char)(v & 0xFF));
                return out.putB((unsigned char)(v >> 8));
            }
            bytesWritten = 1;
            return out.putB((unsigned char)v);
        }
        bytesWritten = putCharToUTF16(out, c);
        return bytesWritten != 0;
    }
    if (c > 0xFF || revMap[c] != 0) {
        bytesWritten = 1;
        return out.putB('?');
    }
    bytesWritten = 1;
    return out.putB((unsigned char)c);
}
