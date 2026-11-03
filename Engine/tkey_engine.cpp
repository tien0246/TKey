#include <cstdio>

#include "tkey_engine.h"
#include "tkey.h"
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <set>

#define VSeqList VSeqListTable
#define CSeqList CSeqListTable
#define VCPairList VCPairListWritable

static int SortedVSeqList[70 * 4];
static int SortedCSeqList[31 * 4];
static int VCPairListWritable[153 * 2];
int IsVnVowel[186];

static int tripleVowelCompare(const void *a, const void *b)
{
    const int *x = (const int *)a, *y = (const int *)b;
    for (int i = 0; i < 3; i++) {
        if (x[i] < y[i]) return -1;
        if (x[i] > y[i]) return 1;
    }
    return 0;
}
static int tripleConCompare(const void *a, const void *b) { return tripleVowelCompare(a, b); }
static int VCPairCompare(const void *a, const void *b)
{
    const int *x = (const int *)a, *y = (const int *)b;
    if (x[0] != y[0]) return x[0] < y[0] ? -1 : 1;
    if (x[1] != y[1]) return x[1] < y[1] ? -1 : 1;
    return 0;
}

int vnToLower(int c)
{
    if (c == -1) return -1;
    if (c & 1) return c;
    return c + 1;
}

int changeCase(int c)
{
    if (c == -1) return -1;
    if (c & 1) return c - 1;
    return c + 1;
}

int lookupVSeq(int a, int b, int c)
{
    int key[7];
    key[0] = a; key[1] = b; key[2] = c;
    key[4] = c; key[5] = b; key[6] = a;
    unsigned int *found = (unsigned int *)bsearch(key, SortedVSeqList, 70, 16, tripleVowelCompare);
    if (found) return found[3];
    return -1;
}

int lookupCSeq(int a, int b, int c)
{
    int key[7];
    key[0] = a; key[1] = b; key[2] = c;
    key[4] = c; key[5] = b; key[6] = a;
    unsigned int *found = (unsigned int *)bsearch(key, SortedCSeqList, 31, 16, tripleConCompare);
    if (found) return found[3];
    return -1;
}

bool isValidCV(int con, int vseq)
{
    if (con == -1 || vseq == -1)
        return true;
    unsigned int *vs = (unsigned int *)&VSeqList[13 * vseq];
    if (con == 8 && vs[3] == 75) return false;
    if (con == 23 && vs[3] == 143) return false;
    if (con == 11) {
        for (int i = 0; CVKVseq[i] != -1; i++)
            if (CVKVseq[i] == vseq) return true;
        return false;
    }
    return true;
}

bool isValidVC(int vseq, int con)
{
    if (vseq == -1 || con == -1)
        return true;
    unsigned int *vs = (unsigned int *)&VSeqList[13 * vseq];
    if (vs[2] == 0)
        return false;
    if ((CSeqList[5 * con + 4] & 1) == 0)
        return false;
    int key[2] = { vseq, con };
    return bsearch(key, VCPairListWritable, 153, 8, VCPairCompare) != 0;
}

bool isValidCVC(int c1, int v, int c2)
{
    if (v == -1)
        return c1 == -1 ? true : (c2 != -1);
    if (c1 == -1)
        return isValidVC(v, c2);
    if (c2 == -1)
        return isValidCV(c1, v);
    bool valid = isValidCV(c1, v);
    bool valid2 = isValidVC(v, c2);
    if (!valid || !valid2) {
        if (!valid2) {
            if (c1 == 23 && v == 11 && (c2 == 16 || c2 == 19))
                return true;
            if (c1 == 8 && (v == 3 || v == 4) && (c2 == 16 || c2 == 17))
                return true;
        }
        return false;
    }
    return true;
}

unsigned int TKeyEngine::macroText[2048];
int TKeyEngine::usedAsMapChar = 0;
char TKeyEngine::m_classInit = 0;

void TKeyEngine::engineClassInit()
{
    for (int i = 0; i < 70; i++) {
        for (int j = 0; j < 3; j++)
            SortedVSeqList[4 * i + j] = VSeqList[13 * i + 3 + j];
        SortedVSeqList[4 * i + 3] = i;
    }
    for (int k = 0; k < 31; k++) {
        for (int m = 0; m < 3; m++)
            SortedCSeqList[4 * k + m] = CSeqList[5 * k + 1 + m];
        SortedCSeqList[4 * k + 3] = k;
    }
    qsort(SortedVSeqList, 70, 16, tripleVowelCompare);
    qsort(SortedCSeqList, 31, 16, tripleConCompare);
    memcpy(VCPairListWritable, VCPairListTable, sizeof(VCPairListWritable));
    qsort(VCPairListWritable, 153, 8, VCPairCompare);
    for (int n = 0; n < 186; n++)
        IsVnVowel[n] = 1;

    for (unsigned char c = 'a'; c <= 'z'; c++) {
        if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' && c != 'y') {
            IsVnVowel[AZLexiLowerTable[c - 'a']] = 0;
            IsVnVowel[AZLexiUpperTable[c - 'a']] = 0;
        }
    }
}

TKeyEngine::TKeyEngine()
{
    if ((m_classInit & 1) == 0) {
        engineClassInit();
        m_classInit = 1;
    }
    ctrl = 0;
    checkKbCaseFunc = 0;
    wordCapacity = 128;
    keyCapacity = 128;
    wordIndex = -1;
    keyEventPos = -1;
    singleMode = 0;
    wordStart = 0;
    backspaces = {};
    keyAppended = 0;
    escPending = 0;
    restoredPending = 0;
    upcaseNext = 0;
    lastKeyCode = 0;
}

void TKeyEngine::setCtrlInfo(TKeySharedState *info)
{
    ctrl = info;
}

void TKeyEngine::setCheckKbCaseFunc(void (*fn)(int *, int *))
{
    checkKbCaseFunc = fn;
}

void TKeyEngine::reset(int code)
{
    wordIndex = -1;
    keyEventPos = -1;
    singleMode = 0;
    escPending = 0;
    usedAsMapChar = 0;
    if (code != 13) {
        upcaseNext = 0;
        lastKeyCode = 0;
    }
}

void TKeyEngine::resetKeyBuf() { keyEventPos = -1; }
void TKeyEngine::setSingleMode() { singleMode = 1; }

bool TKeyEngine::atWordBeginning()
{
    if (wordIndex >= 0)
        return words[wordIndex].wordClass == 1;
    return true;
}

bool TKeyEngine::isAbleMarkTone()
{

    return wordIndex >= 0 && words[wordIndex].wordClass >= 3;
}

void TKeyEngine::pass(int keyCode)
{
    prepareBuffer();
    TKeyEvent e = {};
    ctrl->inputProc.keyCodeToEvent(keyCode, e);
    processAppend(e);
}

void TKeyEngine::prepareBuffer()
{
    if (wordIndex >= 0 && wordIndex + 10 >= wordCapacity) {
        int i;
        for (i = wordIndex / 2; ; i++) {
            bool stop = words[i].wordClass == 1 || i >= wordIndex;
            if (stop) break;
        }
        if (i == wordIndex) {
            wordIndex = -1;
        } else {
            int first = i + 1;
            memmove(words, &words[first], sizeof(WordInfo) * (wordIndex - first + 1));
            wordIndex -= first;
        }
    }
    if (keyEventPos > 0) {
        if (keyEventPos + 1 >= keyCapacity) {
            int half = keyEventPos / 2;
            memmove(keyEvents, &keyEvents[half], sizeof(KeyEventRec) * (keyEventPos - half + 1));
            keyEventPos -= half;
        }
    }
}

int TKeyEngine::writeOutput(unsigned char *buf, int &len)
{
    int ok = 1;
    StringBOStream out(buf, len);
    VnCharset *cs = VnCharsetLibObj->getVnCharset(ctrl->outputCharset);
    cs->startOutput();
    for (int i = wordStart; i <= wordIndex; i++) {
        unsigned int c;
        if (words[i].vnChar == -1) {
            c = IsoToStdVnChar(words[i].isoChar);
        } else {
            c = (unsigned int)words[i].vnChar + 0x10000;
            if (words[i].upperFlag)
                c--;
            if (words[i].tone)
                c += 2 * words[i].tone;
        }
        if (c != (unsigned int)-1) {
            int unusedLen;
            ok = cs->putChar(out, c, unusedLen, 0, false);
        }
    }
    len = out.getOutBytes();
    return ok ? 0 : 5;
}

int TKeyEngine::getSeqSteps(int from, int to, TKeyBackspaces &bbs)
{
    bbs.count = 0;
    if (to < from)
        return 0;
    if (to == from) {

    }
    if (ctrl->outputCharset != 12 && ctrl->outputCharset != 0) {
        StringBOStream out(0, 0);
        VnCharset *cs = VnCharsetLibObj->getVnCharset(ctrl->outputCharset);
        cs->startOutput();
        for (int i = from; i <= to; i++) {
            unsigned int c;
            if (words[i].vnChar == -1) {
                c = words[i].isoChar;
            } else {
                c = (unsigned int)words[i].vnChar + 0x10000;
                if (words[i].upperFlag) c--;
                if (words[i].tone) c += 2 * words[i].tone;
            }
            if (c != (unsigned int)-1) {
                int unusedLen;
                int written = 0;
                cs->putChar(out, c, written, 0, false);
                (void)written;
            }
        }

        int outBytes = out.getOutBytes();
        if (ctrl->outputCharset == 4)
            outBytes /= 2;
        return outBytes;
    }
    return to - from + 1;
}

void TKeyEngine::markChange(int idx)
{
    if (idx < wordStart) {
        TKeyBackspaces steps{};
        int n = getSeqSteps(idx, wordStart - 1, steps);
        backspaces.count += (unsigned long long)n;
        wordStart = idx;
    }
}

unsigned int TKeyEngine::process(unsigned int keyCode, TKeyBackspaces &backs,
                                unsigned char *buf, int &len, tkeyOutputType &outType)
{
    prepareBuffer();
    backspaces = {};
    wordStart = wordIndex + 1;
    outBuf = buf;
    outLen = &len;
    outputWritten = 0;
    keyAppended = 0;
    restoredPending = 0;
    restoringWord = 0;
    outTypeInt = 0;

    int origKey = keyCode;
    bool upperPending = false;
    if (ctrl->opt(TK_OPT_AUTO_UPPER) != 0 && ctrl->vietnameseEnabled != 0) {
        upperPending = checkUpcaseFirstWord(keyCode) & 1;
        if (upperPending)
            keyCode = toupper(keyCode);
    }

    TKeyEvent e = {};
    ctrl->inputProc.keyCodeToEvent(keyCode, e);

    int appended;
    if (escPending & 1) {
        escPending = 0;
        if (wordIndex < 0 || e.keyType == 20 || e.keyType == 19) {
            appended = processAppend(e);
        } else {
            wordIndex--;
            processAppend(e);
            markChange(wordIndex);
            appended = 1;
        }
    } else {

        int t = e.keyType;
        if (t <= 3)
            appended = processRoof(e);
        else if (t <= 8)
            appended = processHook(e);
        else if (t == 9)
            appended = processDd(e);
        else if (t <= 15)
            appended = processTone(e);
        else if (t == 16)
            appended = processTelexW(e);
        else if (t == 17)
            appended = processSimpleTelexW(e);
        else if (t == 18)
            appended = processMapChar(e);
        else if (t == 19)
            appended = processEscChar(e);
        else
            appended = processAppend(e);
    }

    clearSingleModeAfterDd();

    if (ctrl->vietnameseEnabled != 0 && wordIndex >= 0 &&
        words[wordIndex].wordClass == 0 && e.charType == 0 &&
        (ctrl->opt(TK_OPT_SPELL_CHECK) == 0 || singleMode != 0)) {
        appended = processNoSpellCheck(e);
    }

    if (wordIndex >= 0) {
        e.charType = ctrl->inputProc.getCharType(e.asciiChar);
        KeyEventRec &rec = keyEvents[++keyEventPos];
        rec.keyCode = e.keyType;
        rec.charType = e.charType;
        rec.vnChar = e.vnChar;
        rec.asciiChar = e.asciiChar;
        rec.tone = e.tone;
        rec.processed = appended ? (~restoredPending & 1) : 0;
    }

    if (ctrl->opt(TK_OPT_AUTO_UPPER) != 0 && upperPending && origKey != (int)keyCode &&
        backspaces.count == 0)
        appended = 1;

    if (appended) {
        backs = backspaces;
        if (!(outputWritten & 1))
            writeOutput(buf, len);
        outType = (tkeyOutputType)outTypeInt;
        return appended;
    }
    backs = {};
    len = 0;
    outType = (tkeyOutputType)outTypeInt;
    return 0;
}

void TKeyEngine::clearSingleModeAfterDd()
{
    if (ctrl->vietnameseEnabled != 0 && ctrl->opt(TK_OPT_SPELL_CHECK) != 0) {
        for (unsigned int i = 0; (int)i < wordIndex; i++) {
            if ((words[i].vnChar == 43 || words[i].vnChar == 42) &&
                (words[wordIndex].wordClass == 3 || words[wordIndex].wordClass == 4 ||
                 words[wordIndex].wordClass == 5 || words[wordIndex].wordClass == 6)) {
                singleMode = 0;
                return;
            }
        }
    }
}

char TKeyEngine::checkUpcaseFirstWord(int keyCode)
{
    static std::set<unsigned int> specChars;
    static bool inited = false;
    if (!inited) {
        specChars.insert(46); specChars.insert(33); specChars.insert(63); specChars.insert(13);
        inited = true;
    }
    char prev = upcaseNext & 1;
    if (keyCode == 13) {
        upcaseNext = 1;
    } else if (keyCode == 32) {
        if (specChars.count((unsigned int)lastKeyCode))
            upcaseNext = 1;
    } else {
        lastKeyCode = keyCode;
        upcaseNext = 0;
    }
    return prev;
}

int TKeyEngine::processAppend(TKeyEvent &e)
{
    switch (e.charType) {
    case 0: {
        int lo = vnToLower(e.vnChar);
        if (lo < 0 || lo >= 186)
            lo = e.asciiChar & 0x7F;
        if ((lo == 69 || lo == 185 || lo == 87 || lo == 169) && wordIndex >= 0 &&
            (words[wordIndex].wordClass == 2 && lo != 185 ||
             words[wordIndex].wordClass == 3 ||
             words[wordIndex].wordClass == 4)) {
            e.charType = 2;
            return processAppend(e);
        }
        int root = StdVnNoToneTable[lo];

        bool isVowel = e.vnChar >= 0 && e.vnChar < 186 && (IsVnVowel[e.vnChar] & 1);
        bool isConsonant = !isVowel;
        if (isVowel && wordIndex >= 0) {
            const WordInfo &p = words[wordIndex];
            if (p.wordClass == 2 &&
                ((p.seqIndex == 22 && root == 143) || (p.seqIndex == 6 && root == 75)))
                isConsonant = true;
            else if ((p.wordClass == 3 || p.wordClass == 4) && p.seqIndex == 43 &&
                     (root == 75 || root == 143))
                isConsonant = true;
        }
        if (isConsonant)
            return appendConsonant(e);
        return appendVowel(e);
    }
    case 1:
        singleMode = 0;
        return processWordEnd(e);
    case 2: {
        if (ctrl->vietnameseEnabled != 0 && ctrl->outputCharset == 10 && checkEscapeVIQR(e))
            return 1;
        WordInfo &w = words[++wordIndex];
        w.wordClass = (e.charType == 1);
        w.conStart = -1;
        w.conEnd = -1;
        w.vowelStart = -1;
        w.seqIndex = -1;
        w.isoChar = e.asciiChar;
        w.vnChar = vnToLower(e.vnChar);
        w.tone = 0;
        w.upperFlag = w.vnChar != e.vnChar;
        if (ctrl->vietnameseEnabled != 0 && ctrl->outputCharset == 6) {
            markChange(wordIndex);
            return 1;
        }
        return 0;
    }
    case 3: {
        if (e.asciiChar == 13 && ctrl->opt(TK_OPT_MACRO) != 0 && macroMatch(e))
            return 1;
        int pos = 0;
        if (ctrl->opt(TK_OPT_AUTO_RESTORE) != 0 && lastWordIsNonVn()) {
            pos = *outLen;
            if (restoreKeyStrokes(backspaces, outBuf, pos, outTypeInt)) {
                restoredPending = 1;
                outputWritten = 1;
            }
        }
        if ((restoredPending & 1) && pos < *outLen) {
            if (e.asciiChar != 13)
                outBuf[pos++] = e.asciiChar;
            *outLen = pos;
            return 1;
        }
        reset(e.asciiChar);
        return 0;
    }
    default:
        return 0;
    }
}

int TKeyEngine::processWordEnd(TKeyEvent &e)
{
    if (ctrl->opt(TK_OPT_MACRO) != 0 && macroMatch(e))
        return 1;
    if (ctrl->opt(TK_OPT_SPELL_CHECK) == 0 || singleMode != 0 || wordIndex < 0 || (restoringWord & 1)) {
        WordInfo &w = words[++wordIndex];
        w.wordClass = 1;
        w.conStart = -1; w.conEnd = -1; w.vowelStart = -1;
        w.seqIndex = -1; w.tone = 0;
        w.isoChar = e.asciiChar;
        w.vnChar = vnToLower(e.vnChar);
        w.upperFlag = w.vnChar != e.vnChar;
        return 0;
    }
    int pos = 0;
    if (ctrl->opt(TK_OPT_AUTO_RESTORE) != 0 && lastWordIsNonVn()) {
        pos = *outLen;
        if (restoreKeyStrokes(backspaces, outBuf, pos, outTypeInt)) {
            restoredPending = 1;
            outputWritten = 1;
        }
    }
    WordInfo &w = words[++wordIndex];
    w.wordClass = 1;
    w.conStart = -1; w.conEnd = -1; w.vowelStart = -1;
    w.seqIndex = -1; w.tone = 0;
    w.isoChar = e.asciiChar;
    w.vnChar = vnToLower(e.vnChar);
    w.upperFlag = w.vnChar != e.vnChar;
    if ((restoredPending & 1) && pos < *outLen) {
        outBuf[pos++] = e.asciiChar;
        *outLen = pos;
        return 1;
    }
    return 0;
}

int TKeyEngine::appendVowel(TKeyEvent &e)
{
    WordInfo &w = words[++wordIndex];
    int lo = vnToLower(e.vnChar);
    int root = StdVnNoToneTable[lo];
    w.vnChar = root;
    w.upperFlag = lo != e.vnChar;
    w.tone = (lo - root) / 2;
    w.isoChar = e.asciiChar;

    if (wordIndex != 0 && ctrl->vietnameseEnabled != 0) {
        WordInfo &prev = words[wordIndex - 1];
        switch (prev.wordClass) {
        case 0: case 5: case 6:
            w.wordClass = 0; w.vowelStart = -1; w.conEnd = -1; w.conStart = -1;
            break;
        case 1:
            w.wordClass = 3; w.conEnd = -1; w.conStart = -1; w.vowelStart = 0;
            w.seqIndex = lookupVSeq(root, -1, -1);
            break;
        case 2: {
            int vs = lookupVSeq(root, -1, -1);
            if (prev.seqIndex < 0 || prev.seqIndex >= 31) {
                w.wordClass = 0; w.vowelStart = -1; w.conEnd = -1; w.conStart = -1;
                break;
            }
            int conType = prev.seqIndex;
            if (!isValidCV(conType, vs)) {
                w.wordClass = 0; w.vowelStart = -1; w.conEnd = -1; w.conStart = -1;
                break;
            }
            w.wordClass = 4; w.conStart = 1; w.conEnd = -1; w.vowelStart = 0; w.seqIndex = vs;
            if (conType == 8 && prev.tone != 0) {
                if (w.tone == 0)
                    w.tone = prev.tone;
                markChange(wordIndex - 1);
                prev.tone = 0;
                return 1;
            }
            break;
        }
        case 3: case 4: {
            int prevSeq = prev.seqIndex;
            if (prevSeq < 0 || prevSeq >= 70) {
                w.wordClass = 0; w.vowelStart = -1; w.conEnd = -1; w.conStart = -1;
                break;
            }
            int seqStart = wordIndex - VSeqList[13 * prevSeq];
            int oldTonePos = getTonePosition(prevSeq, 1) + seqStart;
            int oldTone = words[oldTonePos].tone;
            int newSeq;
            if (lo == root || oldTone == 0) {

                if (VSeqList[13 * prevSeq] == 3)
                    newSeq = -1;
                else if (VSeqList[13 * prevSeq] == 2)
                    newSeq = lookupVSeq(VSeqList[13 * prevSeq + 3], VSeqList[13 * prevSeq + 4], root);
                else
                    newSeq = lookupVSeq(VSeqList[13 * prevSeq + 3], root, -1);
            } else {
                newSeq = -1;
            }
            if (newSeq != -1 && prev.wordClass == 4) {
                int c = words[wordIndex - 1 - prev.conStart].seqIndex;
                if (!isValidCV(c, newSeq))
                    newSeq = -1;
            }
            if (newSeq == -1) {
                w.wordClass = 0; w.vowelStart = -1; w.conEnd = -1; w.conStart = -1;
            } else {
                w.wordClass = prev.wordClass;
                w.conStart = (prev.wordClass == 4) ? prev.conStart + 1 : -1;
                w.conEnd = -1;
                w.vowelStart = 0;
                w.seqIndex = newSeq;
                w.tone = 0;
                int newTone = (lo - root) / 2;
                if (oldTone != 0) {
                    int newPos = wordIndex - VSeqList[13 * prevSeq] + getTonePosition(newSeq, 1);
                    if (newPos != oldTonePos) {
                        markChange(oldTonePos);
                        words[oldTonePos].tone = 0;
                        markChange(newPos);
                        if (newTone != 0)
                            oldTone = newTone;
                        words[newPos].tone = oldTone;
                        return 1;
                    }
                    if (newTone != 0 && newTone != oldTone) {
                        markChange(oldTonePos);
                        words[oldTonePos].tone = newTone;
                        return 1;
                    }
                } else if (newTone != 0) {
                    int pos = wordIndex - VSeqList[13 * prevSeq] + getTonePosition(newSeq, 1);
                    markChange(pos);
                    words[pos].tone = newTone;
                    return 1;
                }
            }
            break;
        }
        default:
            break;
        }
        if (ctrl->outputCharset != 6 && isalpha(w.isoChar))
            return 0;
        markChange(wordIndex);
        return 1;
    }
    w.wordClass = 3; w.conEnd = -1; w.conStart = -1; w.vowelStart = 0;
    w.seqIndex = lookupVSeq(root, -1, -1);
    if (ctrl->vietnameseEnabled != 0 &&
        (ctrl->outputCharset == 6 || !isalpha(w.isoChar))) {
        markChange(wordIndex);
        return 1;
    }
    return 0;
}

int TKeyEngine::appendConsonant(TKeyEvent &e)
{
    bool changed = false;
    int result = 0;
    WordInfo &w = words[++wordIndex];
    int lo = vnToLower(e.vnChar);
    w.vnChar = lo;
    w.upperFlag = lo != e.vnChar;
    w.isoChar = e.asciiChar;
    w.tone = 0;

    if (wordIndex != 0 && ctrl->vietnameseEnabled != 0) {
        WordInfo &prev = words[wordIndex - 1];
        switch (prev.wordClass) {
        case 0:
            w.wordClass = 0; w.vowelStart = -1; w.conEnd = -1; w.conStart = -1;
            break;
        case 1:
            w.wordClass = 2; w.conStart = 0; w.conEnd = -1; w.vowelStart = -1;
            w.seqIndex = lookupCSeq(lo, -1, -1);
            break;
        case 2: case 5: case 6: {
            int prevSeq = prev.seqIndex;
            int newSeq;
            if (prevSeq < 0 || prevSeq >= 31) {

                w.wordClass = 0; w.vowelStart = -1; w.conEnd = -1; w.conStart = -1;
                break;
            }
            if (CSeqList[5 * prevSeq] == 3)
                newSeq = -1;
            else if (CSeqList[5 * prevSeq] == 2)
                newSeq = lookupCSeq(CSeqList[5 * prevSeq + 1], CSeqList[5 * prevSeq + 2], lo);
            else
                newSeq = lookupCSeq(CSeqList[5 * prevSeq + 1], lo, -1);
            if (newSeq != -1 && (prev.wordClass == 5 || prev.wordClass == 6)) {
                int c1 = -1;
                if (prev.conStart != -1)
                    c1 = words[wordIndex - 1 - prev.conStart].seqIndex;
                if (!isValidCVC(c1, words[wordIndex - 1 - prev.vowelStart].seqIndex, newSeq))
                    newSeq = -1;
            }
            if (newSeq == -1) {
                w.wordClass = 0; w.vowelStart = -1; w.conEnd = -1; w.conStart = -1;
            } else {
                if (prev.wordClass == 2) {
                    w.wordClass = 2; w.conStart = 0; w.conEnd = -1; w.vowelStart = -1;
                } else {
                    if (prev.wordClass == 5) {
                        w.wordClass = 5; w.conStart = -1;
                    } else {
                        w.wordClass = 6; w.conStart = prev.conStart + 1;
                    }
                    w.conEnd = 0;
                    w.vowelStart = prev.vowelStart + 1;
                }
                w.seqIndex = newSeq;
            }
            break;
        }
        case 3: case 4: {
            int prevSeq = prev.seqIndex;
            if (prevSeq < 0 || prevSeq >= 70) {
                w.wordClass = 0; w.vowelStart = -1; w.conEnd = -1; w.conStart = -1;
                break;
            }
            int effSeq = prevSeq;
            if (prevSeq == 38 || prevSeq == 43)
                effSeq = 44;
            int c1 = -1;
            if (prev.conStart != -1)
                c1 = words[wordIndex - 1 - prev.conStart].seqIndex;
            int newSeq = lookupCSeq(lo, -1, -1);
            if (isValidCVC(c1, effSeq, newSeq)) {
                if (prevSeq == 43) {
                    markChange(wordIndex - 1);
                    prev.vnChar = 121;
                    prev.seqIndex = 44;
                    changed = true;
                } else if (prevSeq == 38) {
                    markChange(wordIndex - 2);
                    words[wordIndex - 2].vnChar = 155;
                    words[wordIndex - 2].seqIndex = 10;
                    prev.seqIndex = 44;
                    changed = true;
                }
                if (prev.wordClass == 3) {
                    w.wordClass = 5; w.conStart = -1;
                } else {
                    w.wordClass = 6; w.conStart = prev.conStart + 1;
                }
                w.conEnd = 0;
                w.vowelStart = 1;
                w.seqIndex = newSeq;
                int oldStart = wordIndex - VSeqList[13 * prevSeq];
                int oldTonePos = getTonePosition(prevSeq, 1) + oldStart;
                if (words[oldTonePos].tone != 0) {
                    int newStart = wordIndex - VSeqList[13 * effSeq];
                    int newTonePos = getTonePosition(effSeq, 0) + newStart;
                    if (newTonePos != oldTonePos) {
                        markChange(newTonePos);
                        words[newTonePos].tone = words[oldTonePos].tone;
                        markChange(oldTonePos);
                        words[oldTonePos].tone = 0;
                        return 1;
                    }
                }
            } else {
                w.wordClass = 0; w.vowelStart = -1; w.conEnd = -1; w.conStart = -1;
            }
            result = changed ? 1 : 0;
            break;
        }
        default:
            break;
        }
        if (result == 0 && ctrl->outputCharset == 6) {
            markChange(wordIndex);
            return 1;
        }
        return result;
    }
    w.wordClass = 2; w.conStart = 0; w.conEnd = -1; w.vowelStart = -1;
    w.seqIndex = lookupCSeq(lo, -1, -1);
    if (ctrl->vietnameseEnabled != 0 && ctrl->outputCharset == 6) {
        markChange(wordIndex);
        return 1;
    }
    return 0;
}

int TKeyEngine::getTonePosition(int vseq, bool atLastChar)
{
    if (vseq < 0 || vseq >= 70)
        return 0;
    unsigned int *vs = (unsigned int *)&VSeqList[13 * vseq];
    if (vs[0] == 1)
        return 0;
    if (vs[9] == (unsigned int)-1) {
        if (vs[11] == (unsigned int)-1) {
            if (vs[0] == 3)
                return 1;
            return (ctrl->opt(TK_OPT_TONE_POS) != 0 && (vseq == 25 || vseq == 27 || vseq == 40)) || !atLastChar;
        }
        if (vseq == 44 || vseq == 66 || vseq == 67)
            return 1;
        return vs[11];
    }
    return vs[9];
}

bool TKeyEngine::isBreakWord(WordInfo &w)
{
    return w.isoChar != 46 && w.isoChar != 63 && w.isoChar != 33 &&
           w.isoChar != 44 && w.isoChar != 58 && w.wordClass == 1;
}

int TKeyEngine::processRoof(TKeyEvent &e)
{
    if (ctrl->vietnameseEnabled == 0 || wordIndex < 0 || words[wordIndex].vowelStart < 0)
        return processAppend(e);
    int roofChar;
    if (e.keyType == 1) roofChar = 13;
    else if (e.keyType == 2) roofChar = 57;
    else if (e.keyType == 3) roofChar = 109;
    else roofChar = -1;

    bool fromUO = false;
    int seqStart = wordIndex - words[wordIndex].vowelStart;
    int vseq = words[seqStart].seqIndex;
    if (vseq < 0 || vseq >= 70)
        return processAppend(e);
    int seqFirst = seqStart - (VSeqList[13 * vseq] - 1);
    int tonePos = getTonePosition(vseq, seqStart == wordIndex) + seqFirst;
    int oldTone = words[tonePos].tone;

    unsigned int newSeq;
    if (vseq == 43 || vseq == 44 || vseq == 64 || vseq == 66) {
        newSeq = lookupVSeq(143, 109, VSeqList[13 * vseq + 5]);
        fromUO = true;
    } else {
        newSeq = VSeqList[13 * vseq + 10];
    }

    unsigned int *nv;
    bool needAppend = false;
    if (newSeq == (unsigned int)-1) {
        if (VSeqList[13 * vseq + 9] == -1)
            return processAppend(e);
        int ch = words[VSeqList[13 * vseq + 9] + seqFirst].vnChar;
        if (roofChar != -1 && ch != roofChar)
            return processAppend(e);
        int replace = (ch == 13) ? 1 : (ch == 57) ? 45 : 97;
        int idx = VSeqList[13 * vseq + 9] + seqFirst;
        if (ctrl->opt(TK_OPT_FREE_MARK) == 0 && idx != wordIndex)
            return processAppend(e);
        markChange(idx);
        words[idx].vnChar = replace;
        if (VSeqList[13 * vseq] == 3)
            newSeq = lookupVSeq(words[seqFirst].vnChar, words[seqFirst + 1].vnChar, words[seqFirst + 2].vnChar);
        else if (VSeqList[13 * vseq] == 2)
            newSeq = lookupVSeq(words[seqFirst].vnChar, words[seqFirst + 1].vnChar, -1);
        else
            newSeq = lookupVSeq(words[seqFirst].vnChar, -1, -1);
        if (newSeq < 0 || newSeq >= 70)
            return processAppend(e);
        nv = (unsigned int *)&VSeqList[13 * newSeq];
        needAppend = true;
    } else {
        if (newSeq < 0 || newSeq >= 70)
            return processAppend(e);
        nv = (unsigned int *)&VSeqList[13 * newSeq];
        if (roofChar != -1 && (int)nv[nv[9] + 3] != roofChar)
            return processAppend(e);
        unsigned int c1 = 0xFFFFFFFF, c2 = 0xFFFFFFFF;
        if (words[wordIndex].conStart != -1)
            c1 = words[wordIndex - words[wordIndex].conStart].seqIndex;
        if (words[wordIndex].conEnd != -1)
            c2 = words[wordIndex - words[wordIndex].conEnd].seqIndex;
        if (!isValidCVC((int)c1, (int)newSeq, (int)c2))
            return processAppend(e);
        int idx = fromUO ? seqFirst : nv[9] + seqFirst;
        if (ctrl->opt(TK_OPT_FREE_MARK) == 0 && idx != wordIndex)
            return processAppend(e);
        markChange(idx);
        if (fromUO) {
            words[seqFirst].vnChar = 143;
            words[seqFirst + 1].vnChar = 109;
        } else {
            words[idx].vnChar = nv[nv[9] + 3];
        }
    }
    for (unsigned int i = 0; i < nv[0]; i++)
        words[i + seqFirst].seqIndex = nv[i + 6];
    int newTonePos = getTonePosition(newSeq, seqStart == wordIndex) + seqFirst;
    if (tonePos != newTonePos && oldTone != 0) {
        markChange(newTonePos);
        words[newTonePos].tone = oldTone;
        markChange(tonePos);
        words[tonePos].tone = 0;
    }
    if (needAppend) {
        singleMode = 0;
        processAppend(e);
        keyAppended = 1;
    }
    return 1;
}

int TKeyEngine::processHook(TKeyEvent &e)
{
    if (ctrl->vietnameseEnabled == 0 || wordIndex < 0 || words[wordIndex].vowelStart < 0)
        return processAppend(e);
    bool needAppend = false;
    int seqStart = wordIndex - words[wordIndex].vowelStart;
    int vseq = words[seqStart].seqIndex;
    if (vseq < 0 || vseq >= 70)
        return processAppend(e);
    unsigned int *cv = (unsigned int *)&VSeqList[13 * vseq];
    if ((int)cv[0] > 1 && e.keyType != 8 && (cv[3] == 143 || cv[3] == 155) &&
        (VSeqList[13 * vseq + 4] == 97 || VSeqList[13 * vseq + 4] == 121 || VSeqList[13 * vseq + 4] == 109))
        return processHookWithUO(e);

    int seqFirst = seqStart - (VSeqList[13 * vseq] - 1);
    int tonePos = getTonePosition(vseq, seqStart == wordIndex) + seqFirst;
    int oldTone = words[tonePos].tone;

    unsigned int newSeq = VSeqList[13 * vseq + 12];
    unsigned int *nv;
    if (newSeq != (unsigned int)-1) {
        if (newSeq >= 70)
            return processAppend(e);
        nv = (unsigned int *)&VSeqList[13 * newSeq];
        int hookChar = (int)nv[nv[11] + 3];
        if (e.keyType == 6) {
            if (hookChar != 155) return processAppend(e);
        } else if (e.keyType == 7) {
            if (hookChar != 121) return processAppend(e);
        } else if (e.keyType == 8) {
            if (hookChar != 25) return processAppend(e);
        } else if (e.keyType == 5 && hookChar == 25) {
            return processAppend(e);
        }
        unsigned int c1 = 0xFFFFFFFF, c2 = 0xFFFFFFFF;
        if (words[wordIndex].conStart != -1)
            c1 = words[wordIndex - words[wordIndex].conStart].seqIndex;
        if (words[wordIndex].conEnd != -1)
            c2 = words[wordIndex - words[wordIndex].conEnd].seqIndex;
        if (!isValidCVC((int)c1, (int)newSeq, (int)c2))
            return processAppend(e);
        int idx = nv[11] + seqFirst;
        if (ctrl->opt(TK_OPT_FREE_MARK) == 0 && idx != wordIndex)
            return processAppend(e);
        markChange(idx);
        words[idx].vnChar = nv[nv[11] + 3];
    } else {
        if (VSeqList[13 * vseq + 11] == -1)
            return processAppend(e);
        int ch = words[VSeqList[13 * vseq + 11] + seqFirst].vnChar;
        int replace = (ch == 25) ? 1 : 97;
        if (ch == 155) replace = 143;
        if (ch == 25) replace = 1;
        int idx = VSeqList[13 * vseq + 11] + seqFirst;
        if (ctrl->opt(TK_OPT_FREE_MARK) == 0 && idx != wordIndex)
            return processAppend(e);
        if (e.keyType == 6) {
            if (ch != 155) return processAppend(e);
        } else if (e.keyType == 7) {
            if (ch != 121) return processAppend(e);
        } else if (e.keyType == 8) {
            if (ch != 25) return processAppend(e);
        } else if (e.keyType == 5 && ch == 25) {
            return processAppend(e);
        }
        markChange(idx);
        words[idx].vnChar = replace;
        if (VSeqList[13 * vseq] == 3)
            newSeq = lookupVSeq(words[seqFirst].vnChar, words[seqFirst + 1].vnChar, words[seqFirst + 2].vnChar);
        else if (VSeqList[13 * vseq] == 2)
            newSeq = lookupVSeq(words[seqFirst].vnChar, words[seqFirst + 1].vnChar, -1);
        else
            newSeq = lookupVSeq(words[seqFirst].vnChar, -1, -1);
        if (newSeq == (unsigned int)-1)
            return processAppend(e);
        nv = (unsigned int *)&VSeqList[13 * newSeq];
        needAppend = true;
    }
    for (unsigned int i = 0; i < nv[0]; i++)
        words[i + seqFirst].seqIndex = nv[i + 6];
    int newTonePos = getTonePosition(newSeq, seqStart == wordIndex) + seqFirst;
    if (tonePos != newTonePos && oldTone != 0) {
        markChange(newTonePos);
        words[newTonePos].tone = oldTone;
        markChange(tonePos);
        words[tonePos].tone = 0;
    }
    if (needAppend) {
        singleMode = 0;
        processAppend(e);
        keyAppended = 1;
    }
    return 1;
}

int TKeyEngine::processHookWithUO(TKeyEvent &e)
{
    bool needAppend = false;
    if (ctrl->opt(TK_OPT_FREE_MARK) == 0 && words[wordIndex].vowelStart != 0)
        return processAppend(e);
    int seqStart = wordIndex - words[wordIndex].vowelStart;
    int vseq = words[seqStart].seqIndex;
    int seqFirst = seqStart - VSeqList[13 * vseq] + 1;
    unsigned int *cv = (unsigned int *)&VSeqList[13 * vseq + 3];
    int tonePos = getTonePosition(vseq, seqStart == wordIndex) + seqFirst;
    int oldTone = words[tonePos].tone;
    unsigned int newSeq;

    auto SEQ_OK = [](long v) { return v >= 0 && v < 70; };
    if (e.keyType == 6) {
        if (cv[0] == 143)
            newSeq = VSeqList[13 * vseq + 12];
        else
            newSeq = lookupVSeq(143, 97, VSeqList[13 * vseq + 5]);
    } else if (e.keyType == 7) {
        if (VSeqList[13 * vseq + 4] != 97 && VSeqList[13 * vseq + 4] != 109) {
            newSeq = lookupVSeq(143, 97, VSeqList[13 * vseq + 5]);
            needAppend = true;
        } else if (seqStart != wordIndex || VSeqList[13 * vseq] != 2 ||
                   words[wordIndex].wordClass != 4 ||
                   words[wordIndex - 2].seqIndex != 27) {
            newSeq = lookupVSeq(155, 121, VSeqList[13 * vseq + 5]);
        } else {
            newSeq = VSeqList[13 * vseq + 12];
        }
    } else {
        if (cv[0] == 143) {
            if (VSeqList[13 * vseq + 4] == 97 || VSeqList[13 * vseq + 4] == 109) {
                if ((vseq == 36 || vseq == 37) && seqStart == wordIndex &&
                    words[wordIndex].wordClass == 4 &&
                    (words[wordIndex - 2].seqIndex == 27 || words[wordIndex - 2].seqIndex == 10)) {
                    newSeq = 38;
                } else {
                    unsigned int mid = VSeqList[13 * vseq + 12];
                    newSeq = VSeqList[13 * mid + 12];
                }
            } else {
                newSeq = VSeqList[13 * vseq + 12];
            }
        } else if (VSeqList[13 * vseq + 4] == 97) {
            newSeq = VSeqList[13 * vseq + 12];
        } else {
            newSeq = lookupVSeq(143, 97, VSeqList[13 * vseq + 5]);
            needAppend = true;
        }
    }

    if (e.keyType == 6) {
        if (cv[0] == 143) {
            markChange(seqFirst);
            words[seqFirst].vnChar = 155;
        } else {
            markChange(seqFirst);
            words[seqFirst].vnChar = 143;
            words[seqFirst + 1].vnChar = 97;
        }
    } else if (e.keyType == 7) {
        if (VSeqList[13 * vseq + 4] != 97 && VSeqList[13 * vseq + 4] != 109) {
            if (cv[0] == 155) {
                markChange(seqFirst);
                words[seqFirst].vnChar = 143;
            } else {
                markChange(seqFirst + 1);
            }
            words[seqFirst + 1].vnChar = 97;
        } else {
            if (cv[0] == 143) {
                markChange(seqFirst);
                words[seqFirst].vnChar = 155;
            } else {
                markChange(seqFirst + 1);
            }
            words[seqFirst + 1].vnChar = 121;
        }
    } else {
        if (cv[0] == 143) {
            if (VSeqList[13 * vseq + 4] == 97 || VSeqList[13 * vseq + 4] == 109) {
                if ((vseq == 36 || vseq == 37) && seqStart == wordIndex &&
                    words[wordIndex].wordClass == 4 &&
                    (words[wordIndex - 2].seqIndex == 27 || words[wordIndex - 2].seqIndex == 10)) {
                    markChange(seqFirst + 1);
                    words[seqFirst + 1].vnChar = 121;
                } else {
                    markChange(seqFirst);
                    words[seqFirst].vnChar = 155;
                    words[seqFirst + 1].vnChar = 121;
                }
            } else {
                markChange(seqFirst);
                words[seqFirst].vnChar = 155;
            }
        } else if (VSeqList[13 * vseq + 4] == 97) {
            markChange(seqFirst + 1);
            words[seqFirst + 1].vnChar = 121;
        } else {
            markChange(seqFirst);
            words[seqFirst].vnChar = 143;
            words[seqFirst + 1].vnChar = 97;
        }
    }

    if (!SEQ_OK((long)newSeq))
        return processAppend(e);
    unsigned int *nv = (unsigned int *)&VSeqList[13 * newSeq];
    for (unsigned int i = 0; i < nv[0]; i++)
        words[i + seqFirst].seqIndex = nv[i + 6];
    int newTonePos = getTonePosition(newSeq, seqStart == wordIndex) + seqFirst;
    if (tonePos != newTonePos && oldTone != 0) {
        markChange(newTonePos);
        words[newTonePos].tone = oldTone;
        markChange(tonePos);
        words[tonePos].tone = 0;
    }
    if (needAppend) {
        singleMode = 0;
        processAppend(e);
        keyAppended = 1;
    }
    return 1;
}

int TKeyEngine::processDd(TKeyEvent &e)
{
    if (ctrl->vietnameseEnabled == 0 || wordIndex < 0)
        return processAppend(e);
    if (words[wordIndex].wordClass != 0 || words[wordIndex].vnChar != 41 ||
        (words[wordIndex - 1].vnChar != -1 && (IsVnVowel[words[wordIndex - 1].vnChar] & 1))) {
        if (words[wordIndex].conStart >= 0) {
            int idx = wordIndex - words[wordIndex].conStart;
            if (ctrl->opt(TK_OPT_FREE_MARK) != 0 || idx == wordIndex) {
                if (words[idx].seqIndex == 3) {
                    markChange(idx);
                    words[idx].seqIndex = 4;
                    words[idx].vnChar = 43;
                    singleMode = 1;
                    return 1;
                }
                if (words[idx].seqIndex == 4) {
                    markChange(idx);
                    words[idx].seqIndex = 3;
                    words[idx].vnChar = 41;
                    singleMode = 0;
                    processAppend(e);
                    keyAppended = 1;
                    return 1;
                }
            }
        }
        return processAppend(e);
    }
    singleMode = 1;
    int idx = wordIndex;
    markChange(idx);
    words[idx].seqIndex = 4;
    words[idx].vnChar = 43;
    words[idx].wordClass = 2;
    words[idx].conStart = 0;
    words[idx].conEnd = -1;
    words[idx].vowelStart = -1;
    return 1;
}

int TKeyEngine::processTone(TKeyEvent &e)
{
    if (wordIndex < 0 || ctrl->vietnameseEnabled == 0) return processAppend(e);
    if (words[wordIndex].wordClass == 2 &&
        (words[wordIndex].seqIndex == 8 || words[wordIndex].seqIndex == 9)) {
        int pos = (words[wordIndex].seqIndex == 8) ? wordIndex : wordIndex - 1;
        if (words[pos].tone == 0 && e.tone == 0)
            return processAppend(e);
        markChange(pos);
        if (words[pos].tone == e.tone) {
            words[pos].tone = 0;
            singleMode = 0;
            processAppend(e);
            keyAppended = 1;
        } else {
            words[pos].tone = e.tone;
        }
        return 1;
    }
    if (words[wordIndex].vowelStart < 0) return processAppend(e);
    int seqStart = wordIndex - words[wordIndex].vowelStart;
    int vseq = words[seqStart].seqIndex;
    if (vseq < 0 || vseq >= 70) return processAppend(e);
    unsigned int *vs = (unsigned int *)&VSeqList[13 * vseq];
    if (ctrl->opt(TK_OPT_SPELL_CHECK) != 0 && ctrl->opt(TK_OPT_FREE_MARK) == 0 && vs[1] == 0) return processAppend(e);
    if (words[wordIndex].wordClass == 5 || words[wordIndex].wordClass == 6) {
        int endSeq = words[wordIndex].seqIndex;
        if ((endSeq == 1 || endSeq == 2 || endSeq == 20 || endSeq == 26) &&
            (e.tone == 2 || e.tone == 3 || e.tone == 4))
            return processAppend(e);
        int lo = vnToLower(words[wordIndex].vnChar);
        if (lo == 69 || lo == 185 || lo == 87 || lo == 169)
            return processAppend(e);
    }
    int tonePos = getTonePosition(vseq, seqStart == wordIndex) + seqStart - (vs[0] - 1);
    if (words[tonePos].tone == 0 && e.tone == 0) return processAppend(e);
    if (words[tonePos].tone == e.tone) {
        markChange(tonePos);
        words[tonePos].tone = 0;
        singleMode = 0;
        processAppend(e);
        keyAppended = 1;
    } else {
        markChange(tonePos);
        words[tonePos].tone = e.tone;
    }
    return 1;
}

int TKeyEngine::processTelexW(TKeyEvent &e)
{
    if (ctrl->vietnameseEnabled == 0)
        return processAppend(e);
    if (usedAsMapChar & 1) {
        e.keyType = 18;
        e.vnChar = isupper(e.asciiChar) ? 154 : 155;
        e.charType = 0;
        int r = processMapChar(e);
        if (r) return r;
        if (wordIndex >= 0) wordIndex--;
        usedAsMapChar = 0;
        e.keyType = 4;
        return processHook(e);
    }
    e.keyType = 4;
    usedAsMapChar = 0;
    int r = processHook(e);
    if (r) return r;
    if (wordIndex >= 0) wordIndex--;
    e.keyType = 18;
    e.vnChar = isupper(e.asciiChar) ? 154 : 155;
    e.charType = 0;
    usedAsMapChar = 1;
    return processMapChar(e);
}

int TKeyEngine::processSimpleTelexW(TKeyEvent &e)
{
    if (ctrl->vietnameseEnabled != 0 && wordIndex >= 0 && !isBreakWord(words[wordIndex]))
        return processTelexW(e);
    return processAppend(e);
}

int TKeyEngine::processMapChar(TKeyEvent &e)
{
    int shift = 0, caps = 0;
    if (checkKbCaseFunc)
        checkKbCaseFunc(&shift, &caps);
    if (caps != 0 && !isalpha(e.asciiChar))
        e.vnChar = changeCase(e.vnChar);
    int appended = processAppend(e);
    if (ctrl->vietnameseEnabled != 0) {
        if (wordIndex < 0 || words[wordIndex].wordClass < 2) {
            if (wordIndex >= 0) {
                --wordIndex;
                bool matched = false;
                if (wordIndex >= 0 && words[wordIndex].wordClass >= 2) {
                    WordInfo &prev = words[wordIndex];
                    int ch = prev.vnChar;
                    if (prev.upperFlag) ch--;
                    if (ch == e.vnChar) {
                        if (prev.wordClass == 2) {
                            markChange(wordIndex);
                            wordIndex--;
                        } else {
                            int seqStart = wordIndex - prev.vowelStart;
                            int vseq = words[seqStart].seqIndex;
                            int seqFirst = seqStart - VSeqList[13 * vseq] + 1;
                            int tonePos = getTonePosition(vseq, seqStart == wordIndex) + seqFirst;
                            int oldTone = words[tonePos].tone;
                            markChange(wordIndex);
                            wordIndex--;
                            if (oldTone != 0 && wordIndex >= 0 &&
                                (words[wordIndex].wordClass == 3 || words[wordIndex].wordClass == 4)) {
                                int newPos = getTonePosition(words[wordIndex].seqIndex, 1) + seqFirst;
                                if (newPos != tonePos) {
                                    markChange(newPos);
                                    words[newPos].tone = oldTone;
                                    markChange(tonePos);
                                    words[tonePos].tone = 0;
                                }
                            }
                        }
                        matched = true;
                    }
                }
                e.keyType = 20;
                e.charType = ctrl->inputProc.getCharType(e.asciiChar);
                e.vnChar = IsoToVnLexi(e.asciiChar);
                int r = processAppend(e);
                if (matched) {
                    singleMode = 0;
                    keyAppended = 1;
                    return 1;
                }
                return r;
            }
            return 0;
        }
        return 1;
    }
    return appended;
}

int TKeyEngine::processEscChar(TKeyEvent &e)
{
    if (ctrl->vietnameseEnabled != 0 && wordIndex >= 0 && words[wordIndex].wordClass >= 2)
        escPending = 1;
    return processAppend(e);
}

int TKeyEngine::processNoSpellCheck(TKeyEvent &e)
{
    WordInfo &w = words[wordIndex];
    int vcc = w.vnChar;
    if (vcc < 0 || vcc >= 186)
        return 0;
    if (IsVnVowel[vcc] & 1) {
        w.wordClass = 3;
        w.vowelStart = 0;
        w.seqIndex = lookupVSeq(w.vnChar, -1, -1);
        w.conEnd = -1;
        w.conStart = -1;
    } else {
        w.wordClass = 2;
        w.conStart = 0;
        w.conEnd = -1;
        w.vowelStart = -1;
        w.seqIndex = lookupCSeq(w.vnChar, -1, -1);
    }
    if (e.keyType == 20 && ((w.isoChar >= 97 && w.isoChar <= 122) || (w.isoChar >= 65 && w.isoChar <= 90)))
        return 0;
    markChange(wordIndex);
    return 1;
}

void TKeyEngine::synchKeyStrokeBuffer()
{
    if (keyEventPos >= 0)
        keyEventPos--;
    if (wordIndex >= 0 && words[wordIndex].wordClass == 1) {
        while (keyEventPos >= 0 && keyEvents[keyEventPos].charType != 1)
            keyEventPos--;
    }
}

int TKeyEngine::processBackspace(TKeyBackspaces &backs, unsigned char *buf, int &len, int &outType)
{

    upcaseNext = 0;
    lastKeyCode = 0;
    outType = 0;
    if ((ctrl->vietnameseEnabled == 0 && ctrl->opt(TK_OPT_MACRO) == 0) || wordIndex < 0) {
        backs = {};
        len = 0;
        return 0;
    }
    backspaces = {};
    wordStart = wordIndex + 1;
    markChange(wordIndex);

    const WordInfo &cur = words[wordIndex];
    bool plainDelete = wordIndex == 0 || cur.wordClass <= 2 || cur.vowelStart < 0 ||
                       words[wordIndex - 1].wordClass == 2 ||
                       words[wordIndex - 1].wordClass == 5 ||
                       words[wordIndex - 1].wordClass == 6;
    int vseqStart = 0, vseq = -1, prevSeq = -1;
    if (!plainDelete) {
        vseqStart = wordIndex - cur.vowelStart;
        vseq = words[vseqStart].seqIndex;
        prevSeq = words[wordIndex - 1].seqIndex;
        plainDelete = vseq < 0 || vseq >= 70 || prevSeq < 0 || prevSeq >= 70;
    }
    if (!plainDelete) {
        int seqFirst = vseqStart - VSeqList[13 * vseq] + 1;
        int curTonePos = getTonePosition(vseq, vseqStart == wordIndex) + seqFirst;
        int prevTonePos = getTonePosition(prevSeq, 1) + seqFirst;
        int curTone = words[curTonePos].tone;
        bool moveTone = curTone != 0 && curTonePos != prevTonePos &&
                        !(curTonePos == wordIndex && cur.tone != 0);
        if (moveTone) {
            markChange(prevTonePos);
            words[prevTonePos].tone = curTone;
            markChange(curTonePos);
            words[curTonePos].tone = 0;
            wordIndex--;
            synchKeyStrokeBuffer();
            backs = backspaces;
            writeOutput(buf, len);
            return 1;
        }
    }
    wordIndex--;
    synchKeyStrokeBuffer();
    backs = backspaces;
    len = 0;
    return backspaces.count > 1;
}

bool TKeyEngine::lastWordIsNonVn()
{
    if (wordIndex < 0)
        return false;
    switch (words[wordIndex].wordClass) {
    case 0: return true;
    case 1: case 2: return false;
    case 3: case 4:
        return VSeqList[13 * words[wordIndex].seqIndex + 1] == 0;
    case 5: case 6: {
        int seqStart = wordIndex - words[wordIndex].vowelStart;
        int vseq = words[seqStart].seqIndex;
        if (VSeqList[13 * vseq + 1] != 0) {
            int endSeq = words[wordIndex].seqIndex;
            int startSeq = -1;
            if (words[wordIndex].conStart != -1)
                startSeq = words[wordIndex - words[wordIndex].conStart].seqIndex;
            if (isValidCVC(startSeq, vseq, endSeq)) {
                int seqFirst = seqStart - VSeqList[13 * vseq] + 1;
                int tone = words[getTonePosition(vseq, 0) + seqFirst].tone;
                if ((endSeq != 1 && endSeq != 2 && endSeq != 20 && endSeq != 26) ||
                    (tone != 2 && tone != 3 && tone != 4))
                    return false;
                return true;
            }
            return true;
        }
        return true;
    }
    default: return false;
    }
}

bool TKeyEngine::lastWordHasVnMark()
{
    for (int i = wordIndex; i >= 0 && words[i].wordClass != 1; i--) {
        int c = words[i].vnChar;
        if (c != -1) {
            if (c >= 0 && c < 186 && (IsVnVowel[c] & 1) && words[i].tone != 0)
                return true;
            if (c != StdVnRootCharTable[c])
                return true;
        }
    }
    return 0;
}

int TKeyEngine::restoreKeyStrokes(TKeyBackspaces &backs, unsigned char *buf, int &len, int &outType)
{
    outType = 1;
    if (!lastWordHasVnMark()) {
        backs = {};
        len = 0;
        return 0;
    }
    backspaces = {};
    wordStart = wordIndex + 1;
    bool anyProcessed = false;
    int i;
    for (i = keyEventPos; i >= 0 && keyEvents[i].charType != 1; i--) {
        if (keyEvents[i].processed & 1)
            anyProcessed = true;
    }
    i++;
    if (!anyProcessed) {
        backs = {};
        len = 0;
        return 0;
    }
    while (wordIndex >= 0 && words[wordIndex].wordClass != 1)
        wordIndex--;
    markChange(wordIndex + 1);
    backs = backspaces;
    restoringWord = 1;
    int pos = len;
    int n = 0;
    while (i <= keyEventPos) {
        if (n < len)
            buf[n++] = keyEvents[i].asciiChar;
        TKeyEvent sym = {};
        ctrl->inputProc.keyCodeToSymbol(keyEvents[i].asciiChar, sym);
        keyEvents[i].processed = 0;
        processAppend(sym);
        i++;
    }
    len = n;
    restoringWord = 0;
    return 1;
}

int TKeyEngine::checkEscapeVIQR(TKeyEvent &e)
{
    if (wordIndex < 0)
        return 0;
    WordInfo &w = words[wordIndex];
    bool escape = false;
    if (w.wordClass == 3 || w.wordClass == 4) {
        switch (e.asciiChar) {
        case '\'': case '.': case '?': case '`': case '~':
            escape = w.tone == 0;
            break;
        case '(':
            escape = w.vnChar == 1;
            break;
        case '+':
            escape = w.vnChar == 97 || w.vnChar == 143;
            break;
        case '^':
            escape = w.vnChar == 1 || w.vnChar == 97 || w.vnChar == 45;
            break;
        }
    } else if (w.wordClass == 0) {
        int up = toupper(w.isoChar);
        switch (e.asciiChar) {
        case '\'': case '.': case '?': case '`': case '~':
            escape = up == 'A' || up == 'E' || up == 'I' || up == 'O' || up == 'U' || up == 'Y';
            break;
        case '(':
            escape = up == 'A';
            break;
        case '+':
            escape = up == 'O' || up == 'U';
            break;
        case '^':
            escape = up == 'A' || up == 'O' || up == 'E';
            break;
        }
    }
    if (escape) {
        WordInfo &w1 = words[++wordIndex];
        w1.wordClass = (e.charType == 1);
        w1.conStart = -1; w1.conEnd = -1; w1.vowelStart = -1;
        w1.isoChar = '?';
        w1.vnChar = -1;
        WordInfo &w2 = words[++wordIndex];
        w2.wordClass = (e.charType == 1);
        w2.conStart = -1; w2.conEnd = -1; w2.vowelStart = -1;
        w2.isoChar = e.asciiChar;
        w2.vnChar = -1;
        outBuf[0] = '\\';
        outBuf[1] = e.asciiChar;
        *outLen = 2;
        outputWritten = 1;
    }
    return escape;
}

int TKeyEngine::macroMatch(TKeyEvent &e)
{

    unsigned int key[18];
    const char32_t *macro = 0;
    {
        int i = wordIndex;
        while (i >= 0 && wordIndex - i + 1 < 16 && !isBreakWord(words[i]))
            i--;
        if (i >= 0 && !isBreakWord(words[i]))
            return 0;
        int start = i + 1;
        if (start > wordIndex)
            return 0;
        for (int j = start; j <= wordIndex; j++) {
            if (words[j].vnChar == -1) {
                key[j - start] = words[j].isoChar;
            } else {
                key[j - start] = (unsigned int)words[j].vnChar + 0x10000;
                if (words[j].upperFlag) key[j - start]--;
                key[j - start] += 2 * words[j].tone;
            }
        }
        key[wordIndex - start + 1] = 0;
        macro = ctrl->macroTable.lookup(key);
        if (!macro)
            return 0;
        markChange(start);
    }

    int mode = 0;
    if (macro[0] >= 0x10000 && macro[0] < 0x100BA && (macro[0] & 1) == 0) {
        mode = 1;
        int count = 1;
        for (int k = 1; macro[k] != 0; k++) {
            count++;
            if (macro[k] >= 0x10000 && macro[k] < 0x100BA && (macro[k] & 1) == 0) {
                mode = 3;
                break;
            }
        }
        if (count == 1)
            mode = 3;
    }
    int len = 0;
    for (; macro[len] != 0; len++)
        ;
    if (len >= 2048)
        len = 2048;
    if (mode == 1) {
        for (int m = 0; m < len; m++)
            macroText[m] = StdVnToUpper(macro[m]);
    } else {
        memcpy(macroText, macro, 4 * len);
        if (mode == 3)
            macroText[0] = StdVnToUpper(macroText[0]);
    }
    int inLenV = 4 * len;
    int outLenV = *outLen;
    VnConvert(7, ctrl->outputCharset, (unsigned char *)macroText, outBuf, &inLenV, &outLenV, false, true);
    int total = outLenV;
    if (e.asciiChar != 13 && total < *outLen) {
        int rem = *outLen - total;
        unsigned int ch[2] = { 0, 0 };
        if (e.vnChar == -1)
            ch[0] = e.asciiChar;
        else
            ch[0] = (unsigned int)e.vnChar + 0x10000;
        int inLen2 = 4, outLen2 = rem;
        VnConvert(7, ctrl->outputCharset, (unsigned char *)ch, outBuf + total, &inLen2, &outLen2, false, true);
        total += outLen2;
    }
    TKeyBackspaces keep = backspaces;
    reset(e.asciiChar);
    outputWritten = 1;
    backspaces = keep;
    *outLen = total;
    outTypeInt = 2;
    return 1;
}
