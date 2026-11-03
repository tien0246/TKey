#include "vnlib.h"
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <new>

const char *VIQREscapes[8] = { "://", "/", "@", "mailto:", "email:", "ews:", "www", "ftp" };

const unsigned char SpecialWesternChars[28] = {
    0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8e,
    0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9e, 0x9f, 0
};

static const unsigned char WordBreakSyms[33] = {
    ',', ';', ':', '.', '"', '\'', '!', '?', ' ', '<', '>', '=', '+', '-', '*',
    '/', '\\', '_', '@', '#', '$', '%', '&', '(', ')', '{', '}', '[', ']', '|', '~', '`', '^'
};

static const int AscVnLexiList[] = {
    0xC0, 0x4, 0xC1, 0x2, 0xC2, 0xC, 0xC2, 0x8, 0xC8, 0x30, 0xC9, 0x2E, 0xCA, 0x38, 0xCC, 0x4E,
    0xCD, 0x4C, 0xD2, 0x64, 0xD3, 0x62, 0xD4, 0x6C, 0xD5, 0x68, 0xD9, 0x92, 0xDA, 0x90,
    0xDD, 0xAE, 0xE0, 0x5, 0xE1, 0x3, 0xE2, 0xD, 0xE3, 0x9, 0xE8, 0x31, 0xE9, 0x2F,
    0xEA, 0x39, 0xEC, 0x4F, 0xED, 0x4D, 0xF2, 0x65, 0xF3, 0x63, 0xF4, 0x6D, 0xF5, 0x69,
    0xF9, 0x93, 0xFA, 0x91, 0xFD, 0xAF, 0, 0
};

static const int AZLexiLower[26] = {
    0x1, 0x25, 0x27, 0x29, 0x2D, 0x45, 0x47, 0x49, 0x4B, 0x57, 0x59, 0x5B, 0x5D,
    0x5F, 0x61, 0x85, 0x87, 0x89, 0x8B, 0x8D, 0x8F, 0xA7, 0xA9, 0xAB, 0xAD, 0xB9
};
static const int AZLexiUpper[26] = {
    0x0, 0x24, 0x26, 0x28, 0x2C, 0x44, 0x46, 0x48, 0x4A, 0x56, 0x58, 0x5A, 0x5C,
    0x5E, 0x60, 0x84, 0x86, 0x88, 0x8A, 0x8C, 0x8E, 0xA6, 0xA8, 0xAA, 0xAC, 0xB8
};

unsigned char TkMapCharType[256];
int IsoVnLexiMap[256];
int IsoStdVnCharMap[256];
int LoVowel[26];
int HiVowel[26];
unsigned char ClassifierTableInitialized = 0;

extern CVnCharsetLib *VnCharsetLibObj;

void SetupInputClassifierTable()
{
    int i;
    for (i = 0; i <= 0x20; i++)
        TkMapCharType[i] = 3;
    TkMapCharType[0x14] = 1;
    for (i = 33; i < 256; i++)
        TkMapCharType[i] = 2;
    for (i = 'a'; i <= 'z'; i++)
        TkMapCharType[i] = 0;
    for (i = 'A'; i <= 'Z'; i++)
        TkMapCharType[i] = 0;
    for (int k = 0; AscVnLexiList[2 * k] != 0; k++)
        TkMapCharType[AscVnLexiList[2 * k]] = 0;

    TkMapCharType[0xD2] = 2;
    TkMapCharType[0x9E] = 2;
    TkMapCharType[0xCC] = 2;
    TkMapCharType[0xA6] = 2;
    TkMapCharType[0xDC] = 2;
    TkMapCharType[0xB8] = 2;
    TkMapCharType[0xEA] = 2;
    TkMapCharType[0xC4] = 2;
    for (i = 0; i < 33; i++)
        TkMapCharType[WordBreakSyms[i]] = 1;

    for (i = 0; i < 256; i++)
        IsoVnLexiMap[i] = -1;
    for (int k = 0; AscVnLexiList[2 * k] != 0; k++)
        IsoVnLexiMap[AscVnLexiList[2 * k]] = AscVnLexiList[2 * k + 1];
    for (i = 'a'; i <= 'z'; i++)
        IsoVnLexiMap[i] = AZLexiLower[i - 'a'];
    for (i = 'A'; i <= 'Z'; i++)
        IsoVnLexiMap[i] = AZLexiUpper[i - 'A'];
}

int IsoToVnLexi(int c)
{
    if (c < 256)
        return IsoVnLexiMap[c];
    return -1;
}

int IsoToStdVnChar(int c)
{
    if (c >= 256)
        return c;
    return IsoStdVnCharMap[c];
}

void setupTkEngine()
{
    SetupInputClassifierTable();
    for (int i = 0; i < 256; i++)
        IsoStdVnCharMap[i] = i;
    for (int j = 0; SpecialWesternChars[j] != 0; j++)
        IsoStdVnCharMap[SpecialWesternChars[j]] = j + 65722;
    for (int k = 0; k < 256; k++) {
        int v = IsoToVnLexi(k);
        if (v != -1)
            IsoStdVnCharMap[k] = v + 0x10000;
    }
}

VIQRCharset::VIQRCharset(unsigned int *viqrTable)
{
    tbl = viqrTable;
    memset(revMap, 0, sizeof(revMap));
    for (int i = 0; i < 213; i++) {
        unsigned int v = tbl[i];
        if ((v & 0xFFFFFF00) == 0)
            revMap[v] = (unsigned short)(i + 256);
    }

    revMap['\''] = 2;
    revMap['`'] = 4;
    revMap['?'] = 6;
    revMap['~'] = 8;
    revMap['.'] = 10;
    revMap['^'] = 12;
    revMap['('] = 24;
    revMap['*'] = 26;
    revMap['+'] = 26;
    lastCharWasNonVn = 0;
    lastCharIsA = 0;
    lastCharIsAEOrO = 0;
    lastCharIsOOrU = 0;
    lastCharIsVowel = 0;
    doubleDiacritic = 0;
    escapedChar = 0;
    escapePendingOut = 0;
    lastCharIsDouble = 0;
}

void *VIQRCharset::startInput()
{
    lastCharIsDouble = 0;
    lastCharWasNonVn = 1;
    doubleDiacritic = 0;
    escapedChar = 0;
    if (VnCharsetLibObj->options.ViqrEscape)
        VnCharsetLibObj->viqrEscapeStates.reset();
    return 0;
}

void *VIQRCharset::startOutput()
{
    lastCharIsA = 0;
    lastCharIsAEOrO = 0;
    lastCharIsOOrU = 0;
    lastCharIsVowel = 0;
    escapePendingOut = 0;
    VnCharsetLibObj->viqrEscapeOutStates.reset();
    return 0;
}

int VIQRCharset::nextInput(ByteInStream &in, unsigned int &c, int &bytesRead)
{
    unsigned char ch;
    bytesRead = 0;
    if (!in.getNext(ch))
        return 0;
    bytesRead = 1;

    if (ch == '\\') {
        unsigned char lit;
        if (in.getNext(lit)) {
            bytesRead++;
            c = lit;
            lastCharWasNonVn = 1;
            return 1;
        }
        c = ch;
        return 1;
    }

    unsigned int code = revMap[ch];
    if (code < 0x100) {
        c = ch;
        lastCharWasNonVn = 1;
        doubleDiacritic = 0;
        return 1;
    }

    unsigned char up = (unsigned char)toupper(ch);
    unsigned char next;

    if (up == 'D' && in.peekNext(next) && toupper(next) == 'D') {
        in.getNext(next);
        bytesRead++;
        c = code + 2 + 0xFF00;
        lastCharWasNonVn = 0;
        return 1;
    }

    bool isVowel = ((ch >= 'a' && ch <= 'z') && LoVowel[ch - 'a']) ||
                   ((ch >= 'A' && ch <= 'Z') && HiVowel[ch - 'A']);
    if (isVowel && in.peekNext(next)) {
        unsigned int m = revMap[next];
        bool shape = (m == 12 && (up == 'A' || up == 'E' || up == 'O')) ||
                     (m == 24 && up == 'A') ||
                     (m == 26 && (up == 'O' || up == 'U'));
        if (shape) {
            in.getNext(next);
            bytesRead++;
            unsigned int off = (m == 26) ? 24 : m;
            if (off == 24 && up == 'U')
                off = 12;
            code += off;
            if (in.peekNext(next) && revMap[next] >= 2 && revMap[next] <= 10) {
                in.getNext(next);
                bytesRead++;
                code += revMap[next];
            }
        } else if (m >= 2 && m <= 10) {
            in.getNext(next);
            bytesRead++;
            code += m;
        }
    }
    c = code + 0xFF00;
    lastCharWasNonVn = 0;
    return 1;
}

int VIQRCharset::putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode)
{
    (void)bbs;
    unsigned char ch = (unsigned char)c;
    int result = 0;
    if (c < 0x10000) {
        if (c <= 0xFF) {
            bytesWritten = 1;
            unsigned short m = revMap[ch];
            if (!VnCharsetLibObj->options.FixedFirst && !escapePendingOut &&
                (ch == '\\' ||
                 (m != 0 && m <= 0xA && lastCharIsVowel) ||
                 (m == 12 && lastCharIsAEOrO) ||
                 (m == 24 && lastCharIsA) ||
                 (m == 26 && lastCharIsOOrU))) {
                bytesWritten++;
                if (toUnicode)
                    putCharToUTF16(out, '\\');
                else
                    out.putB('\\');
                if (VnCharsetLibObj->viqrEscapeOutStates.foundAtNextChar('\\') != -1)
                    escapePendingOut = 1;
            }
            if (toUnicode)
                result = putCharToUTF16(out, ch);
            else
                result = out.putB(ch);
            if (VnCharsetLibObj->viqrEscapeOutStates.foundAtNextChar((char)ch) != -1)
                escapePendingOut = 1;
            if (escapePendingOut && (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'))
                escapePendingOut = 0;
        } else {
            bytesWritten = 1;
            if (toUnicode)
                result = putCharToUTF16(out, '#');
            else
                result = out.putB('#');
            if (VnCharsetLibObj->viqrEscapeOutStates.foundAtNextChar('#') != -1)
                escapePendingOut = 1;
        }
        lastCharIsA = 0;
        lastCharIsAEOrO = 0;
        lastCharIsOOrU = 0;
        lastCharIsVowel = 0;
        return result;
    }
    if (c - 0x10000 >= 0xD5) {
        bytesWritten = putCharToUTF16(out, c);
        return bytesWritten != 0;
    }
    bytesWritten = 1;
    unsigned int v = tbl[c - 0x10000];
    char upper = (char)toupper((unsigned char)v);
    unsigned char lo = (unsigned char)(v & 0xFF);
    if (toUnicode)
        result = putCharToUTF16(out, lo);
    else
        result = out.putB(lo);
    if (VnCharsetLibObj->viqrEscapeOutStates.foundAtNextChar((char)lo) != -1)
        escapePendingOut = 1;
    if (escapePendingOut && (lo == ' ' || lo == '\t' || lo == '\r' || lo == '\n'))
        escapePendingOut = 0;

    if ((v & 0xFF00) != 0) {
        bytesWritten++;
        unsigned char mid = (unsigned char)((v >> 8) & 0xFF);
        if (toUnicode)
            result = putCharToUTF16(out, mid);
        else
            result = out.putB(mid);
        if ((v & 0xFF0000) != 0) {
            bytesWritten++;
            unsigned char hi = (unsigned char)((v >> 16) & 0xFF);
            if (toUnicode)
                result = putCharToUTF16(out, hi);
            else
                result = out.putB(hi);
            lastCharIsVowel = 0;
        } else {
            unsigned short m = revMap[mid];
            lastCharIsVowel = (m == 12 || m == 24 || m == 26);
        }
        VnCharsetLibObj->viqrEscapeOutStates.reset();
        lastCharIsA = 0;
        lastCharIsOOrU = 0;
        lastCharIsAEOrO = 0;
        return result;
    }

    bool isVowel =
        ((unsigned char)v >= 'a' && (unsigned char)v <= 'z' && LoVowel[(unsigned char)v - 'a']) ||
        ((unsigned char)v >= 'A' && (unsigned char)v <= 'Z' && HiVowel[(unsigned char)v - 'A']);
    lastCharIsVowel = isVowel ? 1 : 0;
    lastCharIsA = (upper == 'A');
    lastCharIsOOrU = (upper == 'U' || upper == 'O');
    lastCharIsAEOrO = (upper == 'A' || upper == 'E' || upper == 'O');
    return result;
}

UTF8VIQRCharset::UTF8VIQRCharset(UnicodeUTF8Charset *u, VIQRCharset *v)
{
    utf8 = u;
    viqr = v;
}

void *UTF8VIQRCharset::startInput()
{
    utf8->startInput();
    return viqr->startInput();
}

void *UTF8VIQRCharset::startOutput()
{
    utf8->startOutput();
    return viqr->startOutput();
}

int UTF8VIQRCharset::nextInput(ByteInStream &in, unsigned int &c, int &bytesRead)
{
    unsigned char ch;
    if (!in.peekNext(ch))
        return 0;
    if (ch <= 0xBF || ch >= 0xFE)
        return viqr->nextInput(in, c, bytesRead);
    viqr->startInput();
    viqr->lastCharIsDouble = 1;
    return utf8->nextInput(in, c, bytesRead);
}

int UTF8VIQRCharset::putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode)
{
    return viqr->putChar(out, c, bytesWritten, bbs, toUnicode);
}

CVnCharsetLib::CVnCharsetLib()
{
    new (&viqrEscapeStates) PatternList();
    new (&viqrEscapeOutStates) PatternList();
    for (int i = 'a'; i <= 'z'; i++)
        LoVowel[i - 'a'] = 0;
    LoVowel[0] = 1;
    LoVowel[4] = 1;
    LoVowel[8] = 1;
    LoVowel[14] = 1;
    LoVowel[20] = 1;
    LoVowel[24] = 1;
    for (int i = 'A'; i <= 'Z'; i++)
        HiVowel[i - 'A'] = 0;
    HiVowel[0] = 1;
    HiVowel[4] = 1;
    HiVowel[8] = 1;
    HiVowel[14] = 1;
    HiVowel[20] = 1;
    HiVowel[24] = 1;
    memset(singleCharsets, 0, sizeof(singleCharsets));
    memset(doubleCharsets, 0, sizeof(doubleCharsets));
    uniCharset = 0;
    uniCompCharset = 0;
    uniUTF8Charset = 0;
    uniRefCharset = 0;
    uniHexCharset = 0;
    viqrCharset = 0;
    utf8VIQRCharset = 0;
    cp1258Charset = 0;
    uniCStringCharset = 0;
    internalCharset = 0;
    VnConvResetOptions(&options);
    char **esc = const_cast<char **>(VIQREscapes);
    viqrEscapeStates.init(esc, 8);
    viqrEscapeOutStates.init(esc, 8);
}

CVnCharsetLib::~CVnCharsetLib()
{
    delete uniCharset;
    delete uniCompCharset;
    delete uniUTF8Charset;
    delete uniRefCharset;
    delete uniHexCharset;
    delete viqrCharset;
    delete utf8VIQRCharset;
    delete cp1258Charset;
    delete uniCStringCharset;
    delete internalCharset;
    for (int i = 0; i < 6; i++)
        delete singleCharsets[i];
    for (int i = 0; i < 5; i++)
        delete doubleCharsets[i];
}

VnCharset *CVnCharsetLib::getVnCharset(int id)
{
    switch (id) {
    case 0:
        if (!uniCharset)
            uniCharset = new UnicodeCharset(const_cast<unsigned short *>(UnicodeTable));
        return uniCharset;
    case 1:
    case 12:
        if (!uniUTF8Charset)
            uniUTF8Charset = new UnicodeUTF8Charset(const_cast<unsigned short *>(UnicodeTable));
        return uniUTF8Charset;
    case 2:
        if (!uniRefCharset)
            uniRefCharset = new UnicodeRefCharset(const_cast<unsigned short *>(UnicodeTable));
        return uniRefCharset;
    case 3:
        if (!uniHexCharset)
            uniHexCharset = new UnicodeHexCharset(const_cast<unsigned short *>(UnicodeTable));
        return uniHexCharset;
    case 4:
        if (!uniCompCharset)
            uniCompCharset = new UnicodeCompCharset(const_cast<unsigned short *>(UnicodeTable), const_cast<unsigned int *>(UnicodeComposite));
        return uniCompCharset;
    case 5:
        if (!cp1258Charset)
            cp1258Charset = new WinCP1258Charset(const_cast<unsigned short *>(WinCP1258), const_cast<unsigned short *>(WinCP1258Pre));
        return cp1258Charset;
    case 6:
        if (!uniCStringCharset)
            uniCStringCharset = new UnicodeCStringCharset(const_cast<unsigned short *>(UnicodeTable));
        return uniCStringCharset;
    case 7:
        if (!internalCharset)
            internalCharset = new VnInternalCharset();
        return internalCharset;
    case 10:
        if (!viqrCharset)
            viqrCharset = new VIQRCharset(const_cast<unsigned int *>(VIQRTable));
        return viqrCharset;
    case 11:
        if (!utf8VIQRCharset) {
            if (!viqrCharset)
                viqrCharset = new VIQRCharset(const_cast<unsigned int *>(VIQRTable));
            if (!uniUTF8Charset)
                uniUTF8Charset = new UnicodeUTF8Charset(const_cast<unsigned short *>(UnicodeTable));
            utf8VIQRCharset = new UTF8VIQRCharset(uniUTF8Charset, viqrCharset);
        }
        return utf8VIQRCharset;
    default:
        if (id >= 20 && id < 26) {
            int i = id - 20;
            if (!singleCharsets[i])
                singleCharsets[i] = new SingleByteCharset(const_cast<unsigned char *>(SingleByteTables[i]));
            return singleCharsets[i];
        }
        if (id >= 40 && id <= 44) {
            int i = id - 40;
            if (!doubleCharsets[i])
                doubleCharsets[i] = new DoubleByteCharset(const_cast<unsigned short *>(DoubleByteTables[i]));
            return doubleCharsets[i];
        }
        return 0;
    }
}

bool isSpecCharForBreakWord(const unsigned int &c)
{
    for (int i = 0; i < 35; i++)
        if (SpecCharsBreakWord[i] == c)
            return true;
    return false;
}

bool isSpecCharForNewSentence(const unsigned int &c)
{
    for (int i = 0; i < 7; i++)
        if (SpecCharsNewSentence[i] == c)
            return true;
    return false;
}

int genConvert(VnCharset &csIn, VnCharset &csOut, ByteInStream &bIn, ByteOutStream &bOut,
               bool applyOptions, bool toUnicode)
{
    csIn.startInput();
    csOut.startOutput();
    int ok = 1;
    int upperPending = 1;
    unsigned int c;
    int unusedLen;

    while (!bIn.eos()) {
        if (!csIn.nextInput(bIn, c, unusedLen))
            break;
        if (c == (unsigned int)-1)
            continue;
        if (applyOptions) {
            if (VnCharsetLibObj->options.ToLower)
                c = StdVnToLower(c);
            else if (VnCharsetLibObj->options.ToUpper)
                c = StdVnToUpper(c);
            else if (VnCharsetLibObj->options.UpperFirstCharOfSentence) {
                bool spec = isSpecCharForNewSentence(c);
                if (spec)
                    upperPending = 1;
                if (upperPending && !spec && c != ' ') {
                    upperPending = 0;
                    c = StdVnToUpper(c);
                }
            } else if (VnCharsetLibObj->options.UpperFirstCharOfWord) {
                bool spec = isSpecCharForBreakWord(c);
                if (spec)
                    upperPending = 1;
                if (upperPending && !spec) {
                    upperPending = 0;
                    c = StdVnToUpper(c);
                }
            }
            if (VnCharsetLibObj->options.RemoveTone)
                c = StdVnGetRoot(c);
        }
        ok = csOut.putChar(bOut, c, unusedLen, 0, toUnicode);
    }
    return ok ? 0 : 5;
}
