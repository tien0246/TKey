#pragma once

#include "vnlib.h"

struct TKeyBackspaces { unsigned long long count; };
enum tkeyOutputType { TK_OUT_NONE = 0, TK_OUT_RESTORE = 1, TK_OUT_APPEND = 2 };

struct TKeyEvent {
    int keyType;
    int charType;
    int vnChar;
    int asciiChar;
    int tone;
};

struct TKeySharedState;

class TKeyEngine {
public:
    struct WordInfo {
        int wordClass;
        int conStart;
        int vowelStart;
        int conEnd;
        int seqIndex;
        int upperFlag;
        int tone;
        int vnChar;
        int isoChar;
    };

    TKeyEngine();

    void setCtrlInfo(TKeySharedState *info);
    void setCheckKbCaseFunc(void (*fn)(int *, int *));

    unsigned int process(unsigned int keyCode, TKeyBackspaces &backspaces,
                         unsigned char *outBuf, int &outLen, tkeyOutputType &outType);
    void pass(int keyCode);
    int processBackspace(TKeyBackspaces &backspaces, unsigned char *outBuf, int &outLen, int &outType);
    int restoreKeyStrokes(TKeyBackspaces &backspaces, unsigned char *outBuf, int &outLen, int &outType);
    void prepareBuffer();
    void reset(int code);
    void resetKeyBuf();
    void setSingleMode();
    bool atWordBeginning();
    bool isAbleMarkTone();
    bool lastWordIsNonVn();
    bool lastWordHasVnMark();
    void synchKeyStrokeBuffer();

    int processRoof(TKeyEvent &e);
    int processHook(TKeyEvent &e);
    int processHookWithUO(TKeyEvent &e);
    int processDd(TKeyEvent &e);
    int processTone(TKeyEvent &e);
    int processTelexW(TKeyEvent &e);
    int processSimpleTelexW(TKeyEvent &e);
    int processMapChar(TKeyEvent &e);
    int processEscChar(TKeyEvent &e);
    int processAppend(TKeyEvent &e);
    int processWordEnd(TKeyEvent &e);
    int processNoSpellCheck(TKeyEvent &e);
    int appendVowel(TKeyEvent &e);
    int appendConsonant(TKeyEvent &e);
    int macroMatch(TKeyEvent &e);
    int checkEscapeVIQR(TKeyEvent &e);
    bool isBreakWord(WordInfo &w);

    int getTonePosition(int vseq, bool atLastChar);
    void markChange(int wordIndex);
    int getSeqSteps(int from, int to, TKeyBackspaces &bbs);
    int writeOutput(unsigned char *outBuf, int &outLen);
    char checkUpcaseFirstWord(int keyCode);
    void clearSingleModeAfterDd();

    void (*checkKbCaseFunc)(int *, int *);
    TKeySharedState *ctrl;
    int wordStart;
    TKeyBackspaces backspaces;
    int wordCapacity;
    int keyCapacity;
    int wordIndex;
    int singleMode;
    struct KeyEventRec {
        int keyCode;
        int charType;
        int vnChar;
        int asciiChar;
        int tone;
        unsigned char processed;
    } keyEvents[128];
    int keyEventPos;
    unsigned char escPending;
    unsigned char *outBuf;
    int *outLen;
    unsigned char outputWritten;
    unsigned char keyAppended;
    unsigned char restoredPending;
    unsigned char restoringWord;
    int outTypeInt;
    unsigned char upcaseNext;
    int lastKeyCode;
    WordInfo words[128];

    static void engineClassInit();

private:
    static unsigned int macroText[2048];
    static int usedAsMapChar;
    static char m_classInit;
};

int lookupVSeq(int a, int b, int c);
int lookupCSeq(int a, int b, int c);
bool isValidCV(int c, int v);
bool isValidVC(int v, int c);
bool isValidCVC(int c1, int v, int c2);
int vnToLower(int c);
int changeCase(int c);
int VnConvert(int inCharset, int outCharset, unsigned char *inBuf, unsigned char *outBuf,
              int *inSize, int *outSize, bool opt, bool toUnicode);
