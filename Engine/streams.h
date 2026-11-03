#pragma once

#include <cstdio>

class ByteInStream {
public:
    virtual ~ByteInStream() {}
    virtual int eos() = 0;
    virtual int getNext(unsigned char &ch) = 0;
    virtual int peekNext(unsigned char &ch) = 0;
    virtual int unget(unsigned char ch) = 0;
    virtual int getNextW(unsigned short &w) = 0;
    virtual int getNextDW(unsigned int &dw) = 0;
    virtual int peekNextW(unsigned short &w) = 0;
    virtual int peekNextDW(unsigned int &dw) = 0;
    virtual int bookmark() = 0;
    virtual int gotoBookmark() = 0;
    virtual int close() = 0;
};

class StringBIStream : public ByteInStream {
public:
    StringBIStream(unsigned char *buf, int len = -1, int wideChar = 1);
    int eos() override;
    int getNext(unsigned char &ch) override;
    int unget(unsigned char ch) override;
    int getNextW(unsigned short &w) override;
    int getNextDW(unsigned int &dw) override;
    int peekNext(unsigned char &ch) override;
    int peekNextW(unsigned short &w) override;
    int peekNextDW(unsigned int &dw) override;
    int reopen();
    int bookmark() override;
    int gotoBookmark() override;
    int close() override;
    int left();

private:
    unsigned char *base;
    unsigned char *cur;
    int eosFlag;
    int bufLen;
    int count;

    int bmEos;
    unsigned char *bmBase;
    unsigned char *bmCur;
    int bmBufLen;
    int bmCount;
    int hasBookmark;
};

class FileBIStream : public ByteInStream {
public:
    FileBIStream(int bufSize = 0x8000, char *buf = 0);
    ~FileBIStream() override;
    int open(const char *fileName);
    void attach(FILE *f);
    int close() override;
    int eos() override;
    int getNext(unsigned char &ch) override;
    int peekNext(unsigned char &ch) override;
    int unget(unsigned char ch) override;
    int getNextW(unsigned short &w) override;
    int getNextDW(unsigned int &dw) override;
    int peekNextW(unsigned short &w) override;
    int peekNextDW(unsigned int &dw) override;
    int bookmark() override;
    int gotoBookmark() override;

private:
    FILE *f;
    char *buf;
    int bufSize;
    int notOpened;
    int unusedLen;
    unsigned char ungetChar;
    int hasUngetChar;
    int lastGetWasDirect;

    long bmPos;
    int hasBookmarkF;
};

class ByteOutStream {
public:
    virtual ~ByteOutStream() {}
    virtual int putB(unsigned char ch) = 0;
    virtual int putW(unsigned short w) = 0;
    virtual int putDW(unsigned int dw) = 0;
    virtual int puts(const char *s, int len) = 0;
    virtual int isOK() = 0;
    virtual int close() = 0;
};

class StringBOStream : public ByteOutStream {
public:
    StringBOStream(unsigned char *buf, int size);
    int putB(unsigned char ch) override;
    int putW(unsigned short w) override;
    int putDW(unsigned int dw) override;
    int puts(const char *s, int len) override;
    int reopen();
    int isOK() override;
    int close() override;
    int getOutBytes();

private:
    unsigned char *base;
    unsigned char *cur;
    int count;
    int bufSize;
    int overflow;
};

class FileBOStream : public ByteOutStream {
public:
    FileBOStream(int bufSize = 0x8000, char *buf = 0);
    ~FileBOStream() override;
    int open(const char *fileName);
    void attach(FILE *f);
    int close() override;
    int putB(unsigned char ch) override;
    int putW(unsigned short w) override;
    int putDW(unsigned int dw) override;
    int puts(const char *s, int len) override;
    int isOK() override;

private:
    FILE *f;
    char *buf;
    int bufSize;
    int notOpened;
    int error;
};
