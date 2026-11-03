#include "pattern.h"

void PatternState::init(char *p)
{
    pos = 0;
    foundCount = 0;
    pattern = p;
    int i = 0;
    int j = -1;
    prefix[0] = -1;
    while (pattern[i] != 0) {
        while (j >= 0 && pattern[i] != pattern[j])
            j = prefix[j];
        i++;
        j++;
        prefix[i] = j;
    }
}

int PatternState::foundAtNextChar(char ch)
{
    while (pos >= 0 && ch != pattern[pos])
        pos = prefix[pos];
    if (pattern[++pos] == 0) {
        foundCount++;
        pos = prefix[pos];
        return 1;
    }
    return 0;
}

void PatternState::reset()
{
    pos = 0;
    foundCount = 0;
}

PatternList::PatternList()
{
    states = 0;
    count = 0;
}

PatternList::~PatternList()
{
    if (states)
        delete[] states;
}

void PatternList::init(char **patterns, int n)
{
    count = n;
    if (states)
        delete[] states;
    states = new PatternState[n];
    for (int i = 0; i < n; i++)
        states[i].init(patterns[i]);
}

int PatternList::foundAtNextChar(char ch)
{
    int found = -1;
    for (int i = 0; i < count; i++) {
        if (states[i].foundAtNextChar(ch))
            found = i;
    }
    return found;
}

void PatternList::reset()
{
    for (int i = 0; i < count; i++)
        states[i].reset();
}
