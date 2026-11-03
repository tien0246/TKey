#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void tk_setup(void);
void tk_cleanup(void);

unsigned int tk_process_char(unsigned int key_code);
int tk_put_char(int key_code);
int tk_put_backspace(void);
int tk_reset_buff(int code);

void tk_set_input_method(int method);
void tk_set_output_charset(int charset);
void tk_enable_vietnamese(int enable);
void tk_set_option(const char *key, int value);

int tk_get_buff_len(void);
int tk_get_backspace_len(void);
int tk_get_out_type(void);
void tk_get_buff(unsigned char *buf, int *len);
bool tk_is_vietnamese_enabled(void);

int tk_load_macro(const char *path);
int tk_save_macro(const char *path);
int tk_macro_add(const char *key, const char *value);
int tk_macro_remove(const char *key);
int tk_macro_count(void);
void tk_macro_get(int index, char *key, int key_size, char *value, int value_size);

int tk_convert(int from_charset, int to_charset,
               const unsigned char *in, int in_size,
               unsigned char *out, int *out_size);

#ifdef __cplusplus
}
#endif
