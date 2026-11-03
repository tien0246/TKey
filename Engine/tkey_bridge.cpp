#include "tkey_bridge.h"
#include "tkey.h"
#include "tkey_engine.h"
#include "vnlib.h"
#include <string.h>
#include <stdlib.h>

void tk_setup(void) { tkSetup(); }
void tk_cleanup(void) { tkCleanup(); }

unsigned int tk_process_char(unsigned int key_code) { return tkProcessChar(key_code); }
int tk_put_char(int key_code) { return tkPutChar(key_code); }
int tk_put_backspace(void) { return tkPutBackspace(); }
int tk_reset_buff(int code) { return tkResetBuff(code); }

void tk_set_input_method(int method) { tkSetInputMethod(method); }
void tk_set_output_charset(int charset) { tkSetOutputCharset(charset); }
void tk_enable_vietnamese(int enable) { tkEnableVietnamese(enable); }

void tk_set_option(const char *key, int value) {
    if (!key) return;
    if (strcmp(key, "spell_check") == 0) tkSetEnableGrammarCheck(value);
    else if (strcmp(key, "auto_restore") == 0) tkSetAutoRestore(value);
    else if (strcmp(key, "free_mark") == 0) tkSetFreeMarkStyle(value);
    else if (strcmp(key, "new_mark") == 0) tkSetNewMarkStyle(value);
    else if (strcmp(key, "auto_upper") == 0) tkSetAutoUpperCase(value);
    else if (strcmp(key, "enable_macro") == 0) tkSetEnableMacro(value);
}

int tk_get_buff_len(void) { return tkGetBuffLen(); }
int tk_get_backspace_len(void) { return tkGetBackspaceLength(); }
int tk_get_out_type(void) { return tkGetOutType(); }

void tk_get_buff(unsigned char *buf, int *len) {
    if (!buf || !len) return;
    *len = tkGetBuff(buf);
}

bool tk_is_vietnamese_enabled(void) {
    return g_tkState ? g_tkState->vietnameseEnabled != 0 : false;
}

int tk_load_macro(const char *path) { return path ? tkLoadMacroTable(path) : 0; }
int tk_save_macro(const char *path) { return path ? tkSaveMacroToFile(path) : 0; }

int tk_macro_add(const char *key, const char *value) {
    if (!key || !value) return 0;
    return tkAddKeyValueToMacroTable(tkGetCurrentMacroTable(), key, value, 1);
}

int tk_macro_remove(const char *key) {
    if (!key) return 0;
    return tkRemoveKeyValueFromMacroTable(tkGetCurrentMacroTable(), key, 1);
}

int tk_macro_count(void) {
    CMacroTable *t = (CMacroTable *)tkGetCurrentMacroTable();
    return t ? (int)t->size() : 0;
}

void tk_macro_get(int index, char *key, int key_size, char *value, int value_size) {
    if (!key || !value || key_size <= 0 || value_size <= 0) return;
    key[0] = value[0] = 0;
    CMacroTable *t = (CMacroTable *)tkGetCurrentMacroTable();
    if (!t || index < 0 || index >= (int)t->size()) return;
    auto it = t->begin();
    std::advance(it, (size_t)index);

    auto to_utf8 = [](const std::u32string &s, char *dst, int dst_size) {
        int n = 0;
        for (char32_t ch : s) {
            unsigned int c = (unsigned int)ch;
            if (c >= 0x10000 && c - 0x10000 < 213)
                c = UnicodeTable[c - 0x10000];
            if (c < 0x80 && n < dst_size - 1) dst[n++] = (char)c;
            else if (c < 0x800 && n < dst_size - 2) {
                dst[n++] = 0xC0 | (c >> 6); dst[n++] = 0x80 | (c & 0x3F);
            } else if (c < 0x10000 && n < dst_size - 3) {
                dst[n++] = 0xE0 | (c >> 12); dst[n++] = 0x80 | ((c >> 6) & 0x3F); dst[n++] = 0x80 | (c & 0x3F);
            } else if (n < dst_size - 4) {
                dst[n++] = 0xF0 | (c >> 18); dst[n++] = 0x80 | ((c >> 12) & 0x3F);
                dst[n++] = 0x80 | ((c >> 6) & 0x3F); dst[n++] = 0x80 | (c & 0x3F);
            } else break;
        }
        dst[n] = 0;
    };
    to_utf8(it->first, key, key_size);
    to_utf8(it->second, value, value_size);
}

int tk_convert(int from, int to, const unsigned char *in, int in_size,
               unsigned char *out, int *out_size) {
    if (!in || !out || !out_size) return -1;
    return VnConvert(from, to, (unsigned char *)in, out, &in_size, out_size, false, false);
}
