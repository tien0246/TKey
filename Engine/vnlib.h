#pragma once

#include "charset.h"
#include "pattern.h"

extern const char *VIQREscapes[8];

extern const unsigned char SpecialWesternChars[28];

extern unsigned char TkMapCharType[256];
extern int IsoVnLexiMap[256];
extern int IsoStdVnCharMap[256];
extern int LoVowel[26];
extern int HiVowel[26];
extern unsigned char ClassifierTableInitialized;

void SetupInputClassifierTable();
void setupTkEngine();
int IsoToVnLexi(int c);
int IsoToStdVnChar(int c);

class CVnCharsetLib {
public:
    CVnCharsetLib();
    ~CVnCharsetLib();
    VnCharset *getVnCharset(int id);

    SingleByteCharset *singleCharsets[6];
    DoubleByteCharset *doubleCharsets[5];
    UnicodeCharset *uniCharset;
    UnicodeCompCharset *uniCompCharset;
    UnicodeUTF8Charset *uniUTF8Charset;
    UnicodeRefCharset *uniRefCharset;
    UnicodeHexCharset *uniHexCharset;
    VIQRCharset *viqrCharset;
    UTF8VIQRCharset *utf8VIQRCharset;
    WinCP1258Charset *cp1258Charset;
    UnicodeCStringCharset *uniCStringCharset;
    VnInternalCharset *internalCharset;

    PatternList viqrEscapeStates;

    PatternList viqrEscapeOutStates;

    VnConvOptions options;
};

extern CVnCharsetLib *VnCharsetLibObj;

int genConvert(VnCharset &in, VnCharset &out, ByteInStream &bIn, ByteOutStream &bOut,
               bool processOptions, bool toUnicode);
