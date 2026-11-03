#include "Engine/tkey.h"
#include "Engine/tkey_bridge.h"
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
typedef std::u16string U;
static U u(const char *s){ U r; const unsigned char*p=(const unsigned char*)s; while(*p){ unsigned c=*p++; if(c>=0xE0){c=((c&15)<<12)|((p[0]&63)<<6)|(p[1]&63);p+=2;} else if(c>=0xC0){c=((c&31)<<6)|(p[0]&63);p++;} r+=(char16_t)c;} return r; }
static std::string s8(const U&s){ std::string o; for(char16_t c:s){ if(c<0x80)o+=c; else if(c<0x800){o+=0xC0|(c>>6);o+=0x80|(c&63);} else {o+=0xE0|(c>>12);o+=0x80|((c>>6)&63);o+=0x80|(c&63);} } return o; }

struct Field {
    U text; int anchor=0, focus=0; bool omnibox=true; bool focusLeft=true;
    std::vector<U> suggestions{u("amazon.com"),u("apple.com"),u("vietnamnet.vn"),u("tôi tên là"),u("chào buổi sáng"),u("tiếng anh")};
    int lo() const { return std::min(anchor,focus);} int hi() const { return std::max(anchor,focus);}
    void autocomplete(){ if(!omnibox) return; for(auto&s:suggestions) if(s.size()>text.size() && s.compare(0,text.size(),text)==0){
        int caret=(int)text.size(); text=s; if(focusLeft){anchor=(int)s.size(); focus=caret;} else {anchor=caret; focus=(int)s.size();} return; } }
    void insert(const U& t){ text.replace(lo(),hi()-lo(),t); anchor=focus=lo()+(int)t.size(); autocomplete(); }
    void backspace(){ if(lo()!=hi()){ text.erase(lo(),hi()-lo()); anchor=focus=lo(); } else if(focus>0){ text.erase(focus-1,1); anchor=--focus; } }
    void shiftLeft(){ if(focus>0) focus--; }
    U visible() const { return omnibox && lo()!=hi() ? text.substr(0,lo()) : text; }
};

enum Strategy { NONE, SHIFT_LEFT, EMPTY_CHAR };
static U type(const char *keys, Field f, Strategy st){
    tk_reset_buff(0);
    for(const char*p=keys;*p;p++){
        tk_process_char((unsigned char)*p);
        int bs=tk_get_backspace_len(); unsigned char b[4104]; int n=0; if(tk_get_buff_len()>0) tk_get_buff(b,&n);
        U out; for(int i=0;i+1<n;i+=2) out+=(char16_t)(b[i]|(b[i+1]<<8));
        if(!bs && !n){ f.insert(U(1,(char16_t)*p)); continue; }
        int events=bs;
        if(st==SHIFT_LEFT && bs>0 && n>0){ f.shiftLeft(); if(events==1) events=0; }
        if(st==EMPTY_CHAR && bs>0){ f.insert(U(1,(char16_t)0x202F)); events++; }
        for(int i=0;i<events;i++) f.backspace();
        if(n) f.insert(out);
    }
    return f.visible();
}
int main(){
    tk_setup(); tk_set_input_method(0); tk_set_option("spell_check",1); tk_set_option("free_mark",1);
    struct Case{const char*keys,*want;} cases[]={{"aa","â"},{"aa tooi chaof vieetj","â tôi chào việt"},{"tieengs anh","tiếng anh"},{"vieetj nam","việt nam"},{"chaof buooir","chào buổi"},{"tooi teen","tôi tên"}};
    const char *sname[]={"no guard (old)","Shift+Left (yours)","empty char (new)"};
    struct Br{const char*name; bool omnibox, focusLeft;} br[]={{"page textarea",false,true},{"Chromium omnibox",true,true},{"Safari/Firefox bar",true,false}};
    for(int s=0;s<3;s++){ printf("== %s\n", sname[s]); int ok=0,tot=0;
        for(auto&b:br){ for(auto&c:cases){ Field f; f.omnibox=b.omnibox; f.focusLeft=b.focusLeft;
            U got=type(c.keys,f,(Strategy)s); bool good = got==u(c.want); ok+=good; tot++;
            if(!good) printf("   %-18s %-22s -> '%s'  (want '%s')\n", b.name, c.keys, s8(got).c_str(), c.want); } }
        printf("   %d/%d correct\n", ok, tot);
        if (s == EMPTY_CHAR && ok != tot) { puts("FAILED"); return 1; }
        if (s == NONE && ok == tot) { puts("FAILED: model no longer reproduces the bug"); return 1; } }
    puts("ALL PASSED");
    return 0;
}
