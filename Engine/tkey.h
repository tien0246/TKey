#pragma once

#include <cstdint>
#include <map>
#include <string>

struct TKeyEvent;
class TKeyEngine;

enum {
    TK_OPT_FREE_MARK = 0,
    TK_OPT_VIQR_ESC = 1,
    TK_OPT_TONE_POS = 2,
    TK_OPT_MACRO = 3,
    TK_OPT_ALWAYS_MACRO = 4,
    TK_OPT_STRICT_SPELL = 5,
    TK_OPT_USE_CLIPBOARD = 6,
    TK_OPT_SMART_SWITCH = 7,
    TK_OPT_EXTENDED = 8,
    TK_OPT_SPELL_CHECK = 9,
    TK_OPT_AUTO_RESTORE = 10,
    TK_OPT_AUTO_UPPER = 11
};

struct TKeyKeyMapping;
struct TKeyKeyEvName;

class TKeyInputProcessor {
public:
    void init();
    void set_input_method(int method);
    int getIM();
    void useBuiltIn(TKeyKeyMapping *m);
    void set_input_map(TKeyKeyEvName *map);
    void getKeyMap(TKeyKeyEvName *map);
    int keyCodeToEvent(unsigned int keyCode, TKeyEvent &e);
    int keyCodeToSymbol(unsigned int keyCode, TKeyEvent &e);
    int getCharType(unsigned int code);
    void set_special_consonants(bool enable);

    int inputMethod;
    int keyMap[256];
};

struct CiLess {
    bool operator()(const std::u32string &a, const std::u32string &b) const;
};

class CMacroTable : public std::map<std::u32string, std::u32string, CiLess> {
public:
    CMacroTable() {}
    void init();
    CMacroTable &operator=(const CMacroTable &o);
    bool operator==(const CMacroTable &o) const;
    const char32_t *data() const;
    int readHeader(FILE *f, int &ver);
    int writeHeader(FILE *f);
    int loadFromFile(const char *path);
    int writeToFile(const char *path, bool text);
    void resetContent();
    void addItem(const char *key, int tone, int method);
    void addItem(const void *key, const void *val, int wideChar);
    int removeKey(const char *key, int tone);
    int isExistKey(const char *key, int tone);
    const char32_t *lookup(unsigned int *key);
};

struct TKeySharedState {
    int initialized;
    int vietnameseEnabled;
    int options[12];
    TKeyInputProcessor inputProc;
    int useCustomInputMap;
    TKeyKeyEvName *customInputMap;
    int outputCharset;
    CMacroTable macroTable;

    int &opt(int i) { return options[i]; }
};

extern TKeyEngine TEngine;
extern TKeySharedState *g_tkState;
extern int tkShiftPressed;
extern int tkCapsLockOn;
extern unsigned char tkBuff[4104];
extern int tkBuffLen;
extern int tkBackspaces;
extern int tkOutput;

int tkSetup();
void tkCleanup();
int tkProcessChar(unsigned int keyCode);
int tkPutChar(int keyCode);
int tkPutBackspace();
int tkResetBuff(int code);
void tkSetSingleMode();
int tkIsAbleMarkTone();
void tkEnableVietnamese(int enable);
void tkSetUserInputMethod(int method);
void tkSetInputMethod(int method);
void tkSetCapShift(int shift, int caps);
void tkSetOutputCharset(int charset);
void tkSetSpecConsonants(int enable);
void tkSetOptions(int *opt);
int *tkGetOptions(void *out);
void tkCreateDefaultOptions(int *opt);
void TKeyCheckKbCase(int *shift, int *caps);
int tkGetBuff(unsigned char *buf);
int tkGetBuffLen();
int tkGetBackspaceLength();
int tkGetOutType();
int tkRestoreKeys();
int tkLoadMacroTable(const char *path);
int tkConvertUnicode1ByteTo2Byte(unsigned char *buf, int len);
void tkSetFreeMarkStyle(int v);
void tkSetNewMarkStyle(int v);
void tkSetAutoUpperCase(int v);
void tkSetEnableGrammarCheck(int v);
void tkSetEnableMacro(int v);
void tkSetAlwaysMacro(int v);
void tkSetAutoRestore(int v);
void *tkCreateNewMacroTable();
void *tkGetCurrentMacroTable();
int tkReleaseMacroTable(void *t);
int tkUpdateMacroTableAndWrite2File(void *t, const char *path);
int tkCompareMacroTableWithCurrent(void *t);
int tkAddKeyValueToMacroTable(void *t, const void *key, const void *value, int wideChar);
int tkRemoveKeyValueFromMacroTable(void *t, const char *key, int tone);
int tkCheckExistKey(const char *key, int tone);
int tkGetMapTableFromData(const void *data, int size);
int tkLoadMacroTableAndReturnValue(const char *path);
int tkSaveMacroToFile(const char *path);
