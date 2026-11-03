#include "streams.h"
#include <cstring>

StringBIStream::StringBIStream(unsigned char *buf, int len, int wideChar)
{
    base = buf;
    cur = buf;
    bufLen = len;
    count = len;
    if (len == -1) {

        if (wideChar == 2)
            eosFlag = (*(unsigned short *)buf == 0);
        else if (wideChar == 4)
            eosFlag = (*(unsigned int *)buf == 0);
        else
            eosFlag = (*buf == 0);
    } else {
        eosFlag = (len <= 0);
    }
    hasBookmark = 0;
}

int StringBIStream::eos() { return eosFlag; }

int StringBIStream::getNext(unsigned char &ch)
{
    if (eosFlag)
        return 0;
    ch = *cur++;
    if (bufLen == -1)
        eosFlag = (ch == 0);
    else
        eosFlag = (--count <= 0);
    return 1;
}

int StringBIStream::unget(unsigned char ch)
{
    if (cur != base) {
        *(--cur) = ch;
        eosFlag = 0;
        if (bufLen != -1)
            ++count;
    }
    return 1;
}

int StringBIStream::getNextW(unsigned short &w)
{
    if (eosFlag)
        return 0;
    w = *(unsigned short *)cur;
    cur += 2;
    if (bufLen == -1)
        eosFlag = (w == 0);
    else {
        count -= 2;
        eosFlag = (count <= 0);
    }
    return 1;
}

int StringBIStream::getNextDW(unsigned int &dw)
{
    if (eosFlag)
        return 0;
    dw = *(unsigned int *)cur;
    cur += 4;
    if (bufLen == -1)
        eosFlag = (dw == 0);
    else {
        count -= 4;
        eosFlag = (count <= 0);
    }
    return 1;
}

int StringBIStream::peekNext(unsigned char &ch)
{
    if (eosFlag)
        return 0;
    ch = *cur;
    return 1;
}

int StringBIStream::peekNextW(unsigned short &w)
{
    if (eosFlag)
        return 0;
    w = *(unsigned short *)cur;
    return 1;
}

int StringBIStream::peekNextDW(unsigned int &dw)
{
    if (eosFlag)
        return 0;
    dw = *(unsigned int *)cur;
    return 1;
}

int StringBIStream::reopen()
{
    cur = base;
    count = bufLen;
    if (bufLen == -1)
        eosFlag = (base == 0);
    else
        eosFlag = (bufLen <= 0);
    hasBookmark = 0;
    return 1;
}

int StringBIStream::bookmark()
{
    hasBookmark = 1;
    bmCur = cur;
    bmBase = base;
    bmEos = eosFlag;
    bmCount = count;
    bmBufLen = bufLen;
    return 1;
}

int StringBIStream::gotoBookmark()
{
    if (!hasBookmark)
        return 0;
    cur = bmCur;
    base = bmBase;
    eosFlag = bmEos;
    count = bmCount;
    bufLen = bmBufLen;
    return 1;
}

int StringBIStream::close() { return 1; }

int StringBIStream::left() { return count; }

FileBIStream::FileBIStream(int size, char *buffer)
{
    f = 0;
    buf = buffer;
    bufSize = size;
    notOpened = 1;
    hasUngetChar = 0;
    lastGetWasDirect = 0;
    hasBookmarkF = 0;
    bmPos = 0;
}

FileBIStream::~FileBIStream()
{
    if (f)
        close();
}

int FileBIStream::open(const char *fileName)
{
    f = fopen(fileName, "rb");
    if (f) {
        setvbuf(f, buf, 0, bufSize);
        notOpened = 0;
        hasUngetChar = 0;
        lastGetWasDirect = 0;
        return 1;
    }
    return 0;
}

void FileBIStream::attach(FILE *file)
{
    f = file;
    notOpened = 0;
    hasUngetChar = 0;
    lastGetWasDirect = 0;
}

int FileBIStream::close()
{
    if (f) {
        fclose(f);
        f = 0;
    }
    return 1;
}

int FileBIStream::eos()
{
    if (hasUngetChar)
        return 0;
    return f ? feof(f) : 1;
}

int FileBIStream::getNext(unsigned char &ch)
{
    if (hasUngetChar) {
        hasUngetChar = 0;
        ch = ungetChar;
        lastGetWasDirect = 1;
        return 1;
    }
    lastGetWasDirect = 0;
    ch = (unsigned char)fgetc(f);
    return !feof(f);
}

int FileBIStream::peekNext(unsigned char &ch)
{
    if (hasUngetChar) {
        ch = ungetChar;
        return 1;
    }
    ch = (unsigned char)fgetc(f);
    if (feof(f))
        return 0;
    ungetc(ch, f);
    return 1;
}

int FileBIStream::unget(unsigned char ch)
{
    if (lastGetWasDirect) {
        lastGetWasDirect = 0;
        hasUngetChar = 1;
        ungetChar = ch;
    } else {
        ungetc(ch, f);
    }
    return 1;
}

int FileBIStream::getNextW(unsigned short &w)
{
    unsigned char lo, hi;
    if (getNext(lo) && getNext(hi)) {
        *((unsigned char *)&w) = lo;
        *(((unsigned char *)&w) + 1) = hi;
        return 1;
    }
    return 0;
}

int FileBIStream::getNextDW(unsigned int &dw)
{
    unsigned short lo, hi;
    if (getNextW(lo) && getNextW(hi)) {
        *((unsigned short *)&dw) = lo;
        *(((unsigned short *)&dw) + 1) = hi;
        return 1;
    }
    return 0;
}

int FileBIStream::peekNextW(unsigned short &w)
{
    unsigned char lo, hi;
    if (peekNext(lo) && peekNext(hi)) {
        *((unsigned char *)&w) = lo;
        *(((unsigned char *)&w) + 1) = hi;
        return 1;
    }
    return 0;
}

int FileBIStream::peekNextDW(unsigned int &dw)
{
    unsigned short lo, hi;
    if (peekNextW(lo) && peekNextW(hi)) {
        *((unsigned short *)&dw) = lo;
        *(((unsigned short *)&dw) + 1) = hi;
        return 1;
    }
    return 0;
}

int FileBIStream::bookmark()
{
    hasBookmarkF = 1;
    bmPos = ftell(f);
    return 1;
}

int FileBIStream::gotoBookmark()
{
    if (!hasBookmarkF)
        return 0;
    fseek(f, (long)bmPos, SEEK_SET);
    return 1;
}

StringBOStream::StringBOStream(unsigned char *buf, int size)
{
    base = buf;
    cur = buf;
    bufSize = size;
    count = 0;
    overflow = 0;
}

int StringBOStream::putB(unsigned char ch)
{
    ++count;
    if (overflow)
        return 0;
    if (count > bufSize) {
        overflow = 1;
        return 0;
    }
    *cur++ = ch;
    return 1;
}

int StringBOStream::putW(unsigned short w)
{
    count += 2;
    if (overflow)
        return 0;
    if (count > bufSize) {
        overflow = 1;
        return 0;
    }
    *(unsigned short *)cur = w;
    cur += 2;
    return 1;
}

int StringBOStream::putDW(unsigned int dw)
{
    count += 4;
    if (overflow)
        return 0;
    if (count > bufSize) {
        overflow = 1;
        return 0;
    }
    *(unsigned int *)cur = dw;
    cur += 4;
    return 1;
}

int StringBOStream::puts(const char *s, int len)
{
    if (len == -1) {
        while (*s) {
            if (++count <= bufSize)
                *cur++ = *s;
            ++s;
        }
        if (!overflow && count > bufSize)
            overflow = 1;
        return !overflow;
    }
    if (!overflow && count <= bufSize) {
        int n = bufSize - count;
        if (n > len)
            n = len;
        memcpy(cur, s, n);
        cur += n;
    }
    count += len;
    if (!overflow && count > bufSize)
        overflow = 1;
    return !overflow;
}

int StringBOStream::reopen()
{
    cur = base;
    count = 0;
    overflow = 0;
    return 1;
}

int StringBOStream::isOK() { return !overflow; }
int StringBOStream::close() { return 1; }
int StringBOStream::getOutBytes() { return count; }

FileBOStream::FileBOStream(int size, char *buffer)
{
    f = 0;
    buf = buffer;
    bufSize = size;
    notOpened = 1;
    error = 1;
}

FileBOStream::~FileBOStream()
{
    if (f)
        close();
}

int FileBOStream::open(const char *fileName)
{
    f = fopen(fileName, "wb");
    if (f) {
        error = 0;
        setvbuf(f, buf, 0, bufSize);
        notOpened = 1;
        return 1;
    }
    return 0;
}

void FileBOStream::attach(FILE *file)
{
    f = file;
    notOpened = 0;
    error = 0;
}

int FileBOStream::close()
{
    if (f) {
        fclose(f);
        f = 0;
    }
    return 1;
}

int FileBOStream::putB(unsigned char ch)
{
    if (error)
        return 0;
    error = (fputc(ch, f) == EOF);
    return !error;
}

int FileBOStream::putW(unsigned short w)
{
    if (error)
        return 0;
    error = (fputc((unsigned char)(w & 0xFF), f) == EOF);
    if (error)
        return 0;
    error = (fputc((unsigned char)((w >> 8) & 0xFF), f) == EOF);
    return !error;
}

int FileBOStream::putDW(unsigned int dw)
{
    if (error)
        return 0;
    for (int i = 0; i < 4; i++) {
        error = (fputc((unsigned char)((dw >> (8 * i)) & 0xFF), f) == EOF);
        if (error)
            return 0;
    }
    return 1;
}

int FileBOStream::puts(const char *s, int len)
{
    if (error)
        return 0;
    if (len == -1)
        error = (fputs(s, f) == EOF);
    else
        error = ((int)fwrite(s, 1, len, f) != len);
    return !error;
}

int FileBOStream::isOK() { return !error; }
