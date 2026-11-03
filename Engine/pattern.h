#pragma once

class PatternState {
public:
    void init(char *pattern);
    int foundAtNextChar(char ch);
    void reset();

private:
    char *pattern;
    int prefix[43];
    int pos;
    int foundCount;
};

class PatternList {
public:
    PatternList();
    ~PatternList();
    void init(char **patterns, int count);
    int foundAtNextChar(char ch);
    void reset();

private:
    PatternState *states;
    int count;
};
