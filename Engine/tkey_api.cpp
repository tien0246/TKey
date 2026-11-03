#include "tkey.h"
#include "tkey_engine.h"
#include "vnlib.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cctype>

struct TKeyKeyMappingEntry { unsigned char keyCode; int value; };

static const TKeyKeyMappingEntry TelexMethodMapping[] = {
    {'z',10},{'s',11},{'f',12},{'r',13},{'x',14},{'j',15},{'w',16},{'a',1},
    {'e',2},{'o',3},{'d',9},
    {'[',21+121},{']',21+155},{'{',21+120},{'}',21+154},
    {0,0}
};
static const TKeyKeyMappingEntry VniMethodMapping[] = {
    {'0',10},{'1',11},{'2',12},{'3',13},{'4',14},{'5',15},{'6',0},{'7',5},
    {'8',8},{'9',9},{0,0}
};
static const TKeyKeyMappingEntry VIQRMethodMapping[] = {
    {'0',10},{'\'',11},{'`',12},{'?',13},{'~',14},{'.',15},{'^',0},{'+',5},
    {'(',8},{')',7},{'*',5},{'d',9},{0,0}
};
static const TKeyKeyMappingEntry MsViMethodMapping[] = {
    {'3',12},{'%',12},{'6',13},{'^',13},{'7',14},{'&',14},{'8',11},{'*',11},
    {'5',10},{'9',15},{'0',16},{'4',0},{'1',5},{'2',6},{'@',7},{'!',8},
    {'$',4},{'#',3},{'"',2},{0,0}
};

static const TKeyKeyMappingEntry SimpleTelexMethodMapping[] = {
    {'z',10},{'s',11},{'f',12},{'r',13},{'x',14},{'j',15},{'w',4},{'a',1},
    {'e',2},{'o',3},{'d',9},{0,0}
};
static const TKeyKeyMappingEntry SimpleTelex2MethodMapping[] = {
    {'z',10},{'s',11},{'f',12},{'r',13},{'x',14},{'j',15},{'w',17},{'a',1},
    {'e',2},{'o',3},{'d',9},{0,0}
};
static const TKeyKeyMappingEntry CompositeVniTelexMethodMapping[] = {
    {'z',10},{'s',11},{'f',12},{'r',13},{'x',14},{'j',15},{'w',16},{'a',1},
    {'e',2},{'o',3},{'d',9},
    {'1',11},{'2',12},{'3',13},{'4',14},{'5',15},{'6',0},{'7',5},{'8',8},{'9',9},
    {'[',21+121},{']',21+155},{'{',21+120},{'}',21+154},
    {0,0}
};

static void TkResetKeyMap(int *keyMap)
{
    for (int i = 0; i < 256; i++)
        keyMap[i] = 20;
}

int TKeyInputProcessor::getIM() { return inputMethod; }

void TKeyInputProcessor::init()
{
    if ((ClassifierTableInitialized & 1) == 0) {
        SetupInputClassifierTable();
        ClassifierTableInitialized = 1;
    }
    set_input_method(0);
}

void TKeyInputProcessor::set_input_method(int method)
{
    inputMethod = method;
    switch (method) {
    case 0: useBuiltIn((TKeyKeyMapping *)TelexMethodMapping); break;
    case 1: useBuiltIn((TKeyKeyMapping *)VniMethodMapping); break;
    case 2: useBuiltIn((TKeyKeyMapping *)VIQRMethodMapping); break;
    case 3: useBuiltIn((TKeyKeyMapping *)MsViMethodMapping); break;
    case 4: useBuiltIn((TKeyKeyMapping *)SimpleTelexMethodMapping); break;
    case 5: useBuiltIn((TKeyKeyMapping *)SimpleTelex2MethodMapping); break;
    case 6: useBuiltIn((TKeyKeyMapping *)CompositeVniTelexMethodMapping); break;
    default:
        inputMethod = 0;
        useBuiltIn((TKeyKeyMapping *)TelexMethodMapping);
        break;
    }
}

void TKeyInputProcessor::useBuiltIn(TKeyKeyMapping *m)
{
    TkResetKeyMap(keyMap);
    TKeyKeyMappingEntry *entries = (TKeyKeyMappingEntry *)m;
    for (int i = 0; entries[i].keyCode != 0; i++) {
        unsigned char key = entries[i].keyCode;
        keyMap[key] = entries[i].value;
        if (entries[i].value < 21) {
            if (islower(key))
                keyMap[toupper(key)] = entries[i].value;
            else if (isupper(key))
                keyMap[tolower(key)] = entries[i].value;
        }
    }
}

void TKeyInputProcessor::set_input_map(TKeyKeyEvName *map)
{
    inputMethod = 7;
    for (int i = 0; i < 256; i++)
        keyMap[i] = ((int *)map)[i];
}

void TKeyInputProcessor::getKeyMap(TKeyKeyEvName *map)
{
    for (int i = 0; i < 256; i++)
        ((int *)map)[i] = keyMap[i];
}

int TKeyInputProcessor::keyCodeToEvent(unsigned int keyCode, TKeyEvent &e)
{
    e.asciiChar = keyCode;
    e.tone = 0;
    if (keyCode <= 0xFF) {
        e.charType = TkMapCharType[keyCode];
        e.keyType = keyMap[keyCode];
        if (e.keyType >= 10 && e.keyType <= 15)
            e.tone = e.keyType - 10;
        if (e.keyType < 21) {
            e.vnChar = IsoToVnLexi(keyCode);
        } else {
            e.charType = 0;
            e.vnChar = e.keyType - 21;
            e.keyType = 18;
        }
    } else {
        e.keyType = 20;
        e.vnChar = IsoToVnLexi(keyCode);
        e.charType = (e.vnChar == -1) ? 2 : 0;
    }
    return 0;
}

int TKeyInputProcessor::keyCodeToSymbol(unsigned int keyCode, TKeyEvent &e)
{
    e.asciiChar = keyCode;
    e.keyType = 20;
    e.tone = 0;
    e.vnChar = IsoToVnLexi(keyCode);
    if (keyCode <= 0xFF)
        e.charType = TkMapCharType[keyCode];
    else
        e.charType = (e.vnChar == -1) ? 2 : 0;
    return 0;
}

int TKeyInputProcessor::getCharType(unsigned int code)
{
    if (code <= 0xFF)
        return TkMapCharType[code];
    return IsoToVnLexi(code) == -1 ? 2 : 0;
}

void TKeyInputProcessor::set_special_consonants(bool enable)
{

    (void)enable;
}

struct CiLessImpl {
    static char32_t lower(char32_t c)
    {
        if (c >= 'A' && c <= 'Z') return c + 32;
        if (c >= 0x10000) {

            if ((c - 0x10000) < 0xBA && (c & 1) == 0) return c + 1;
        }
        return c;
    }
};
bool CiLess::operator()(const std::u32string &a, const std::u32string &b) const
{
    size_t n = a.size() < b.size() ? a.size() : b.size();
    for (size_t i = 0; i < n; i++) {
        char32_t x = CiLessImpl::lower(a[i]), y = CiLessImpl::lower(b[i]);
        if (x != y) return x < y;
    }
    return a.size() < b.size();
}

void CMacroTable::init() {}

CMacroTable &CMacroTable::operator=(const CMacroTable &o)
{
    std::map<std::u32string, std::u32string, CiLess>::operator=(o);
    return *this;
}

bool CMacroTable::operator==(const CMacroTable &o) const
{
    if (size() != o.size()) return false;
    auto it2 = o.begin();
    for (auto it1 = begin(); it1 != end(); ++it1, ++it2)
        if (it1->first != it2->first || it1->second != it2->second)
            return false;
    return true;
}

const char32_t *CMacroTable::data() const { return 0; }

const char32_t *CMacroTable::lookup(unsigned int *key)
{
    std::u32string k((const char32_t *)key);
    auto it = find(k);
    if (it != end())
        return it->second.c_str();
    return 0;
}

int CMacroTable::readHeader(FILE *f, int &ver)
{
    char line[2065];
    if (fgets(line, 2065, f)) {
        size_t len = strlen(line);
        const char *p = line;
        int bomVer = -1;
        if (len >= 3 && (unsigned char)line[0] == 239 && (unsigned char)line[1] == 187 && (unsigned char)line[2] == 191) {
            p = line + 3;
            if (len > 3) {
                unsigned char b = (unsigned char)line[3];
                if (b >= 3 && b <= 30)
                    bomVer = b;
            }
        }
        bool ok = false;
        const char *mark = strstr(p, "<<<");
        if (mark) {
            const char *q = mark + 3;
            while (*q == ' ') q++;
            if (sscanf(q, "version=%d", &ver) == 1)
                ok = true;
        }
        if (!ok) {
            if (bomVer == -1) {
                ver = 0;
            } else {
                ver = bomVer;
                ok = true;
            }
        }
        if (!ok)
            fseek(f, 0, SEEK_SET);
        return 1;
    }
    if (feof(f)) {
        fseek(f, 0, SEEK_SET);
        ver = 0;
        return 1;
    }
    return 0;
}

int CMacroTable::writeHeader(FILE *f)
{
    return fprintf(f, "Khong duoc xoa dong nay<<<version=%d>>>\n", 3);
}

static std::string markNewLineUtf8() { return "<###TKEY-LINE-BREAK###>"; }

static void utReplaceAll(std::string &s, const std::string &from, const std::string &to)
{
    if (from.empty()) return;
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
}

void CMacroTable::resetContent() { clear(); }

void CMacroTable::addItem(const char *line, int wideChar, int version)
{
    const char *sep = (version < 2) ? strstr(line, ":") : strstr(line, "||");
    if (!sep)
        return;
    int n = (int)(sep - line);
    if (n > 15) n = 15;
    char key[24];
    strncpy(key, line, n);
    key[n] = 0;
    const char *val = (version < 2) ? sep + 1 : sep + 2;
    addItem(key, val, wideChar);
}

void CMacroTable::addItem(const void *key, const void *val, int charsetId)
{

    unsigned char kbuf[64] = {0}, kout[128] = {0};
    int inSize = -1, outSize = 120;
    strncpy((char *)kbuf, (const char *)key, 60);
    VnConvert(charsetId, 7, kbuf, kout, &inSize, &outSize, false, false);
    std::u32string k((const char32_t *)kout);
    unsigned char vbufIn[6160] = {0}, vbufOut[6160] = {0};
    strncpy((char *)vbufIn, (const char *)val, 6143);
    int inSize2 = -1, outSize2 = 6140;
    VnConvert(charsetId, 7, vbufIn, vbufOut, &inSize2, &outSize2, false, false);
    std::u32string v((const char32_t *)vbufOut);
    (*this)[k] = v;
}

int CMacroTable::loadFromFile(const char *path)
{
    FILE *f = fopen(path, "rt");
    if (!f)
        return 0;
    resetContent();
    int ver = 0;
    if (!readHeader(f, ver))
        ver = 0;
    char line[2065];
    while (fgets(line, 2065, f)) {
        size_t len = strlen(line);
        if (len && line[len - 1] == '\n') line[--len] = 0;
        if (len > 1 && line[len - 1] == '\r') line[--len] = 0;
        if (ver < 1) {
            addItem(line, 10, ver);
        } else {
            std::string s(line);
            utReplaceAll(s, "\r", markNewLineUtf8());
            addItem(s.c_str(), 1, ver);
        }
    }
    fclose(f);
    if (ver != 3)
        writeToFile(path, true);
    return 1;
}

int CMacroTable::writeToFile(const char *path, bool useTemp)
{
    std::string p(path);
    if (!useTemp)
        p += ".tmp";
    FILE *f = fopen(p.c_str(), "wt");
    if (!f)
        return 0;
    writeHeader(f);
    for (auto it = begin(); it != end(); ++it) {
        unsigned char kbuf[64] = {0}, vbuf[6160] = {0};
        int inSize = -1, outSize = 48;
        VnConvert(7, 1, (unsigned char *)it->first.c_str(), kbuf, &inSize, &outSize, false, false);
        inSize = -1; outSize = 6144;
        int r = VnConvert(7, 1, (unsigned char *)it->second.c_str(), vbuf, &inSize, &outSize, false, false);
        std::string s((char *)vbuf);
        utReplaceAll(s, "\r", markNewLineUtf8());
        if (r == 0)
            fprintf(f, "%s||%s\n", kbuf, s.c_str());
    }
    fclose(f);
    return 1;
}

int CMacroTable::removeKey(const char *key, int tone)
{
    unsigned char kbuf[64] = {0}, vbuf[128] = {0};
    int inSize = -1, outSize = 60;
    strncpy((char *)kbuf, key, 60);
    VnConvert(1, 7, kbuf, vbuf, &inSize, &outSize, false, false);
    erase(std::u32string((const char32_t *)vbuf));
    return 1;
}

int CMacroTable::isExistKey(const char *key, int tone)
{
    unsigned char kbuf[64] = {0}, vbuf[128] = {0};
    int inSize = -1, outSize = 60;
    strncpy((char *)kbuf, key, 60);
    VnConvert(1, 7, kbuf, vbuf, &inSize, &outSize, false, false);
    return find(std::u32string((const char32_t *)vbuf)) != end();
}

TKeyEngine TEngine;
TKeySharedState *g_tkState = 0;
static TKeySharedState s_tkState;
static CVnCharsetLib s_vnCharsetLib;
CVnCharsetLib *VnCharsetLibObj = &s_vnCharsetLib;
static CMacroTable *s_macroTables[8];
static int s_macroTableCount = 0;

int tkShiftPressed = 0;
int tkCapsLockOn = 0;
unsigned char tkBuff[4104];
int tkBuffLen = 0;
int tkBackspaces = 0;
int tkOutput = 0;

void TKeyCheckKbCase(int *shift, int *caps)
{
    *shift = tkShiftPressed;
    *caps = tkCapsLockOn;
}

int tkSetup()
{
    if (g_tkState == 0 || g_tkState->initialized == 0) {
        setupTkEngine();
        g_tkState = &s_tkState;
        new (&s_tkState) TKeySharedState();
        g_tkState->inputProc.init();
        g_tkState->macroTable.init();
        g_tkState->vietnameseEnabled = 1;
        g_tkState->useCustomInputMap = 0;
        TEngine.setCtrlInfo(g_tkState);
        TEngine.setCheckKbCaseFunc(TKeyCheckKbCase);
        tkSetInputMethod(0);
        tkSetOutputCharset(0);
        tkCreateDefaultOptions(g_tkState->options);
        g_tkState->initialized = 1;
    }
    return 0;
}

void tkCleanup() {}

int tkProcessChar(unsigned int keyCode)
{
    tkBuffLen = 4104;
    TKeyBackspaces backs{};
    tkeyOutputType outType = TK_OUT_NONE;
    unsigned int r = TEngine.process(keyCode, backs, tkBuff, tkBuffLen, outType);
    tkBackspaces = (int)backs.count;
    tkOutput = (int)outType;
    return r;
}

int tkPutChar(int keyCode)
{
    TEngine.pass(keyCode);
    tkBuffLen = 0;
    tkBackspaces = 0;
    return 0;
}

int tkPutBackspace()
{
    tkBuffLen = 4104;
    TKeyBackspaces b{};
    int r = TEngine.processBackspace(b, tkBuff, tkBuffLen, tkOutput);
    tkBackspaces = (int)b.count;
    return r;
}

int tkResetBuff(int code)
{
    TEngine.reset(code);
    tkBackspaces = 0;
    tkBuffLen = 0;
    return 0;
}

void tkSetSingleMode() { TEngine.setSingleMode(); }
int tkIsAbleMarkTone() { return TEngine.isAbleMarkTone(); }
void tkEnableVietnamese(int enable) { g_tkState->vietnameseEnabled = enable; }

void tkSetUserInputMethod(int method)
{
    if (g_tkState->useCustomInputMap != 0 && method != 7)
        g_tkState->inputProc.set_input_map(g_tkState->customInputMap);
    else
        g_tkState->inputProc.set_input_method(method);
    TEngine.reset(0);
}

void tkSetInputMethod(int method)
{
    if (method > 6) {
        if (method == 7 && g_tkState->useCustomInputMap != 0) {
            g_tkState->inputProc.set_input_map(g_tkState->customInputMap);
            TEngine.reset(0);
        }
        return;
    }
    g_tkState->inputProc.set_input_method(method);
    TEngine.reset(0);
}

void tkSetCapShift(int shift, int caps)
{
    tkShiftPressed = shift;
    tkCapsLockOn = caps;
}

void tkSetOutputCharset(int charset) { g_tkState->outputCharset = charset; }
void tkSetSpecConsonants(int enable) { g_tkState->inputProc.set_special_consonants(enable != 0); }

void tkSetOptions(int *opt)
{
    g_tkState->options[0] = opt[0];
    g_tkState->options[2] = opt[2];
    g_tkState->options[3] = opt[3];
    g_tkState->options[4] = opt[4];
    g_tkState->options[5] = opt[5];
    g_tkState->options[9] = opt[9];
    g_tkState->options[10] = opt[10];
}

int *tkGetOptions(void *out)
{
    if (!out)
        return g_tkState->options;
    memcpy(out, g_tkState->options, 0x30);
    return (int *)out;
}

void tkCreateDefaultOptions(int *opt)
{
    opt[0] = 1; opt[2] = 0; opt[3] = 0; opt[4] = 0; opt[5] = 0;
    opt[9] = 1; opt[10] = 0;
}

int tkGetBuff(unsigned char *buf) { memcpy(buf, tkBuff, tkBuffLen); return tkBuffLen; }
int tkGetBuffLen() { return tkBuffLen; }
int tkGetBackspaceLength() { return tkBackspaces; }
int tkGetOutType() { return tkOutput; }

int tkRestoreKeys()
{
    tkBuffLen = 4104;
    TKeyBackspaces b{};
    int r = TEngine.restoreKeyStrokes(b, tkBuff, tkBuffLen, tkOutput);
    tkBackspaces = (int)b.count;
    return r;
}

int tkLoadMacroTable(const char *path)
{
    return g_tkState->macroTable.loadFromFile(path);
}

int tkConvertUnicode1ByteTo2Byte(unsigned char *buf, int len)
{

    int out = 0;
    unsigned char *p = buf;
    unsigned char tmp[8192];
    unsigned short *w = (unsigned short *)tmp;
    unsigned int cp = 0;
    for (int i = 0; i < len; ) {
        unsigned char c = p[i];
        int extra = 0;
        if (c < 0x80) { cp = c; }
        else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; extra = 1; }
        else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; extra = 2; }
        else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; extra = 3; }
        i++;
        for (int j = 0; j < extra && i < len; j++, i++)
            cp = (cp << 6) | (p[i] & 0x3F);
        w[out++] = (unsigned short)cp;
    }
    memcpy(buf, tmp, out * 2);
    return out * 2;
}

void tkSetFreeMarkStyle(int v) { g_tkState->options[0] = v; }
void tkSetNewMarkStyle(int v) { g_tkState->options[2] = v; }
void tkSetAutoUpperCase(int v) { g_tkState->options[11] = v; }
void tkSetEnableGrammarCheck(int v) { g_tkState->options[9] = v; }
void tkSetEnableMacro(int v) { g_tkState->options[3] = v; }
void tkSetAlwaysMacro(int v) { (void)v; }
void tkSetAutoRestore(int v) { g_tkState->options[10] = v; }

void *tkCreateNewMacroTable()
{
    CMacroTable *t = new CMacroTable();
    if (s_macroTableCount < 8) s_macroTables[s_macroTableCount++] = t;
    return t;
}

void *tkGetCurrentMacroTable() { return &g_tkState->macroTable; }

int tkReleaseMacroTable(void *t)
{
    delete (CMacroTable *)t;
    return 1;
}

int tkUpdateMacroTableAndWrite2File(void *t, const char *path)
{
    g_tkState->macroTable = *(CMacroTable *)t;
    return g_tkState->macroTable.writeToFile(path, true);
}

int tkCompareMacroTableWithCurrent(void *t)
{
    return g_tkState->macroTable == *(CMacroTable *)t;
}

int tkAddKeyValueToMacroTable(void *t, const void *key, const void *val, int wideChar)
{
    ((CMacroTable *)t)->addItem(key, val, wideChar);
    return 1;
}

int tkRemoveKeyValueFromMacroTable(void *t, const char *key, int tone)
{
    return ((CMacroTable *)t)->removeKey(key, tone);
}

int tkCheckExistKey(const char *key, int tone)
{
    return g_tkState->macroTable.isExistKey(key, tone);
}

int tkGetMapTableFromData(const void *data, int size)
{
    if (!data || size != 256)
        return 0;
    static int map[256];
    memcpy(map, data, sizeof(map));
    g_tkState->customInputMap = (TKeyKeyEvName *)map;
    g_tkState->useCustomInputMap = 1;
    return 1;
}

int tkLoadMacroTableAndReturnValue(const char *path)
{
    return tkLoadMacroTable(path);
}

int tkSaveMacroToFile(const char *path)
{
    return g_tkState->macroTable.writeToFile(path, true);
}

int VnConvert(int inCharset, int outCharset, unsigned char *inBuf, unsigned char *outBuf,
              int *inSize, int *outSize, bool options, bool toUnicode)
{
    int inLen = *inSize;
    int outLen = *outSize;
    if (inLen < -1)
        return -1;
    VnCharset *csIn = VnCharsetLibObj ? VnCharsetLibObj->getVnCharset(inCharset) : 0;
    VnCharset *csOut = VnCharsetLibObj ? VnCharsetLibObj->getVnCharset(outCharset) : 0;
    if (!csIn || !csOut)
        return 2;
    int elem = csIn->elementSize();
    StringBIStream in(inBuf, inLen, elem);
    StringBOStream out(outBuf, outLen);
    int r = genConvert(*csIn, *csOut, in, out, options, toUnicode);
    *outSize = out.getOutBytes();
    *inSize = in.left();
    return r;
}
