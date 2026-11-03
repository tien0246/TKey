#pragma once

#include "streams.h"
#include "tables.h"

enum {
    CONV_CHARSET_UNICODE      = 0,
    CONV_CHARSET_UNICODEUTF8  = 1,
    CONV_CHARSET_UNICODEREF   = 2,
    CONV_CHARSET_UNICODEHEX   = 3,
    CONV_CHARSET_UNICODECOMPO = 4,
    CONV_CHARSET_WINCP1258    = 5,
    CONV_CHARSET_UNICSTRING   = 6,
    CONV_CHARSET_VNINTERNAL   = 7,
    CONV_CHARSET_VIQR         = 10,
    CONV_CHARSET_UTF8VIQR     = 11,
    CONV_CHARSET_UNICODEUTF8_2= 12,

};

struct VnConvOptions {
    int FixedFirst;
    int ViqrEscape;
    int ToUpper;
    int ToLower;
    int UpperFirstCharOfSentence;
    int UpperFirstCharOfWord;
    int RemoveTone;
    int OldStyleVIQR;
};

class PatternList;

class VnCharset {
public:
    virtual ~VnCharset() {}
    virtual void *startInput();
    virtual void *startOutput();
    virtual int nextInput(ByteInStream &in, unsigned int &c, int & bytesRead) = 0;
    virtual int putChar(ByteOutStream &out, unsigned int c, int & bytesWritten, void *bbs, bool toUnicode) = 0;
    virtual int elementSize();

    int putCharToUTF16(ByteOutStream &out, unsigned int c);
};

int wideCharCompare(const void *a, const void *b);
int uniCompInfoCompare(const void *a, const void *b);

unsigned int ConvertUTF32ToUTF16(unsigned int *input, unsigned int count,
                                 unsigned short *output, unsigned int &outCount);
int hexDigitValue(unsigned char ch);

unsigned int StdVnToUpper(unsigned int c);
unsigned int StdVnToLower(unsigned int c);
unsigned int StdVnGetRoot(unsigned int c);
int  IsoToStdVnChar(int c);
bool isSpecCharForBreakWord(const unsigned int &c);
bool isSpecCharForNewSentence(const unsigned int &c);

class VnInternalCharset : public VnCharset {
public:
    VnInternalCharset() {}
    int nextInput(ByteInStream &in, unsigned int &c, int &bytesRead) override;
    int putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode) override;
    int elementSize() override;
};

class SingleByteCharset : public VnCharset {
public:
    SingleByteCharset(unsigned char *tbl);
    int nextInput(ByteInStream &in, unsigned int &c, int &bytesRead) override;
    int putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode) override;
private:
    unsigned short revMap[256];
    unsigned char *tbl;
};

class DoubleByteCharset : public VnCharset {
public:
    DoubleByteCharset(unsigned short *tbl);
    int nextInput(ByteInStream &in, unsigned int &c, int &bytesRead) override;
    int putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode) override;
private:
    unsigned short revMap[256];
    unsigned int wideChars[213];
    unsigned short *tbl;
};

class UnicodeCharset : public VnCharset {
public:
    UnicodeCharset(unsigned short *uniTable);
    int nextInput(ByteInStream &in, unsigned int &c, int &bytesRead) override;
    int putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode) override;
protected:
    unsigned int info[213];
    unsigned short *tbl;
};

class UnicodeCompCharset : public VnCharset {
public:
    UnicodeCompCharset(unsigned short *uniTable, unsigned int *compositeTable);
    int nextInput(ByteInStream &in, unsigned int &c, int &bytesRead) override;
    int putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode) override;
    int elementSize() override;
private:
    struct { unsigned int comp; unsigned int index; } info[430];
    int count;
    unsigned int *compTbl;
    unsigned short *uniTbl;
};

class UnicodeUTF8Charset : public VnCharset {
public:
    UnicodeUTF8Charset(unsigned short *uniTable);
    int nextInput(ByteInStream &in, unsigned int &c, int &bytesRead) override;
    int putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode) override;
protected:
    unsigned int info[213];
    unsigned short *tbl;
};

class UnicodeRefCharset : public VnCharset {
public:
    UnicodeRefCharset(unsigned short *uniTable);
    int nextInput(ByteInStream &in, unsigned int &c, int &bytesRead) override;
    int putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode) override;
protected:
    unsigned int info[213];
    unsigned short *tbl;
};

class UnicodeHexCharset : public VnCharset {
public:
    UnicodeHexCharset(unsigned short *uniTable);
    int nextInput(ByteInStream &in, unsigned int &c, int &bytesRead) override;
    int putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode) override;
protected:
    unsigned int info[213];
    unsigned short *tbl;
};

class UnicodeCStringCharset : public UnicodeHexCharset {
public:
    UnicodeCStringCharset(unsigned short *uniTable);
    void *startInput() override;
    int nextInput(ByteInStream &in, unsigned int &c, int &bytesRead) override;
    int putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode) override;
private:
    int lastOutWasHex;
};

class VIQRCharset : public VnCharset {
public:
    VIQRCharset(unsigned int *viqrTable);
    void *startInput() override;
    void *startOutput() override;
    int nextInput(ByteInStream &in, unsigned int &c, int &bytesRead) override;
    int putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode) override;
private:
    unsigned int *tbl;
    unsigned short revMap[256];

public:

    friend class UTF8VIQRCharset;
    int lastCharWasNonVn;
    int lastCharIsA;
    int lastCharIsAEOrO;
    int lastCharIsOOrU;
    int lastCharIsVowel;
    int doubleDiacritic;
    int escapedChar;
    int escapePendingOut;
    int lastCharIsDouble;
};

class UTF8VIQRCharset : public VnCharset {
public:
    UTF8VIQRCharset(UnicodeUTF8Charset *u, VIQRCharset *v);
    void *startInput() override;
    void *startOutput() override;
    int nextInput(ByteInStream &in, unsigned int &c, int &bytesRead) override;
    int putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode) override;
private:
    VIQRCharset *viqr;
    UnicodeUTF8Charset *utf8;
};

class WinCP1258Charset : public VnCharset {
public:
    WinCP1258Charset(unsigned short *cpTbl, unsigned short *preTbl);
    int nextInput(ByteInStream &in, unsigned int &c, int &bytesRead) override;
    int putChar(ByteOutStream &out, unsigned int c, int &bytesWritten, void *bbs, bool toUnicode) override;
private:
    unsigned short revMap[256];
    unsigned int wideChars[430];
    int wideCount;
    unsigned short *cpTbl;
    unsigned short *preTbl;
};

void VnConvResetOptions(VnConvOptions *opt);
